#include <precompiled_headers.hpp>
#include <android_vulkan_sdk/pbr/actor_desc.hpp>
#include <android_vulkan_sdk/pbr/scene_desc.hpp>
#include <pbr/pbr_game.hpp>
#include <pbr/cube_map_manager.hpp>
#include <pbr/material_manager.hpp>
#include <pbr/mesh_manager.hpp>
#include <pbr/renderable_component.hpp>
#include <av_assert.hpp>
#include <gamepad.hpp>
#include <trace.hpp>


namespace pbr {

namespace {

constexpr char const* SCENES[] =
{
    "pbr/assets/N7_ADM_Reception.scene",
    "pbr/assets/N7_ENG_Injection.scene"
};

constexpr size_t ACTIVE_SCENE = 0U;
static_assert ( std::size ( SCENES ) > ACTIVE_SCENE );

[[maybe_unused]] constexpr uint32_t SCENE_DESC_FORMAT_VERSION = 3U;

constexpr float FIELD_OF_VIEW = 75.0F;
constexpr float Z_NEAR = 0.1F;
constexpr float Z_FAR = 1.0e+4F;

constexpr uint32_t RESOLUTION_SCALE_WIDTH = 80U;
constexpr uint32_t RESOLUTION_SCALE_HEIGHT = 70U;

} // end of anonymous namespace

//----------------------------------------------------------------------------------------------------------------------

bool PBRGame::IsReady () noexcept
{
    return !_allComponents.empty ();
}

bool PBRGame::OnFrame ( android_vulkan::Renderer &renderer, double deltaTime ) noexcept
{
    AV_TRACE ( "OnFrame" )
    auto const dt = static_cast<float> ( deltaTime );

    _camera.Update ( dt );
    GXMat4 const &cameraLocal = _camera.GetLocalMatrix ();
    _renderSession.Begin ( cameraLocal, _camera.GetProjectionMatrix () );

    for ( auto &component : _renderableComponents )
    {
        // NOLINTNEXTLINE - downcast.
        auto &renderableComponent = static_cast<RenderableComponent &> ( *component.get () );
        renderableComponent.Submit ( _renderSession );
    }

    return _renderSession.End ( renderer, deltaTime );
}

bool PBRGame::OnInitDevice ( android_vulkan::Renderer &renderer ) noexcept
{
    VkCommandPoolCreateInfo const createInfo
    {
        .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0U,
        .queueFamilyIndex = renderer.GetQueueFamilyIndex ()
    };

    VkDevice device = renderer.GetDevice ();

    bool result = android_vulkan::Renderer::CheckVkResult (
        vkCreateCommandPool ( device, &createInfo, nullptr, &_commandPool ),
        "pbr::PBRGame::OnInit",
        "Can't create command pool"
    );

    if ( !result ) [[unlikely]]
        return false;

    AV_SET_VULKAN_OBJECT_NAME ( device, _commandPool, VK_OBJECT_TYPE_COMMAND_POOL, "Asset pool" )

    if ( !_renderSession.OnInitDevice ( renderer ) || !UploadGPUContent ( renderer ) ) [[unlikely]]
       return false;

    _renderSession.FreeTransferResources ( renderer );
    return true;
}

void PBRGame::OnDestroyDevice ( android_vulkan::Renderer &renderer ) noexcept
{
    _allComponents.clear ();
    _renderableComponents.clear ();

    _renderSession.OnDestroyDevice ( renderer );
    DestroyCommandPool ( renderer.GetDevice () );

    MeshManager::Destroy ( renderer );
    MaterialManager::Destroy ( renderer );
    CubeMapManager::Destroy ( renderer );
}

bool PBRGame::OnSwapchainCreated ( android_vulkan::Renderer &renderer ) noexcept
{
    VkExtent2D const &surfaceResolution = renderer.GetViewportResolution ();

    VkExtent2D const resolution
    {
        .width = surfaceResolution.width * RESOLUTION_SCALE_WIDTH / 100U,
        .height = surfaceResolution.height * RESOLUTION_SCALE_HEIGHT / 100U
    };

    _camera.SetProjection ( GXDegToRad ( FIELD_OF_VIEW ),
        static_cast<float> ( surfaceResolution.width ) / static_cast<float> ( surfaceResolution.height ),
        Z_NEAR,
        Z_FAR
    );

    if ( !_renderSession.OnSwapchainCreated ( renderer, resolution ) ) [[unlikely]]
        return false;

    _camera.CaptureInput ();
    return true;
}

void PBRGame::OnSwapchainDestroyed ( android_vulkan::Renderer &renderer ) noexcept
{
    _camera.ReleaseInput ();
    _renderSession.OnSwapchainDestroyed ( renderer.GetDevice () );
}

void PBRGame::DestroyCommandPool ( VkDevice device ) noexcept
{
    if ( _commandPool == VK_NULL_HANDLE ) [[unlikely]]
        return;

    vkDestroyCommandPool ( device, _commandPool, nullptr );
    _commandPool = VK_NULL_HANDLE;
}

bool PBRGame::UploadGPUContent ( android_vulkan::Renderer &renderer ) noexcept
{
    android_vulkan::File file ( SCENES[ ACTIVE_SCENE ] );

    if ( !file.LoadContent () ) [[unlikely]]
        return false;

    std::vector<uint8_t> const &content = file.GetContent ();
    uint8_t const* data = content.data ();
    auto const* sceneDesc = reinterpret_cast<SceneDesc const*> ( data );

    // Sanity checks.
    static_assert ( sizeof ( GXVec3 ) == sizeof ( sceneDesc->_viewerLocation ) );
    AV_ASSERT ( sceneDesc->_formatVersion == SCENE_DESC_FORMAT_VERSION )

    _camera.SetLocation ( *reinterpret_cast<GXVec3 const*> ( &sceneDesc->_viewerLocation ) );
    _camera.SetRotation ( sceneDesc->_viewerPitch, sceneDesc->_viewerYaw );
    _camera.Update ( 0.0F );

    auto const comBuffs = static_cast<size_t> (
        sceneDesc->_textureCount +
        sceneDesc->_meshCount +
        sceneDesc->_envMapCount
    );

    VkCommandBufferAllocateInfo const allocateInfo
    {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
        .pNext = nullptr,
        .commandPool = _commandPool,
        .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
        .commandBufferCount = static_cast<uint32_t> ( comBuffs )
    };

    _commandBuffers.resize ( comBuffs );
    VkDevice device = renderer.GetDevice ();

    bool result = android_vulkan::Renderer::CheckVkResult (
        vkAllocateCommandBuffers ( device, &allocateInfo, _commandBuffers.data () ),
        "pbr::PBRGame::UploadGPUContent",
        "Can't allocate command buffers"
    );

    if ( !result ) [[unlikely]]
        return false;

    VkCommandBuffer const* commandBuffers = _commandBuffers.data ();

#if defined ( AV_ENABLE_VVL ) || defined ( AV_ENABLE_RENDERDOC )

    for ( size_t i = 0U; i < comBuffs; ++i )
    {
        AV_SET_VULKAN_OBJECT_NAME ( device,
            commandBuffers[ i ],
            VK_OBJECT_TYPE_COMMAND_BUFFER,
            "Asset loading #%zu",
            i
        )
    }

#endif // AV_ENABLE_VVL || AV_ENABLE_RENDERDOC


    uint8_t const* readPointer = data + sizeof ( SceneDesc );

    size_t consumed = 0U;
    size_t read = 0U;
    uint64_t const actors = sceneDesc->_actorCount;

    for ( uint64_t actorIdx = 0U; actorIdx < actors; ++actorIdx )
    {
        auto const* actorDesc = reinterpret_cast<ActorDesc const*> ( readPointer );
        readPointer += sizeof ( ActorDesc );

        uint32_t const components = actorDesc->_components;

        for ( uint32_t compIdx = 0U; compIdx < components; ++compIdx ) {
            ComponentRef component = Component::Create ( renderer,
                consumed,
                read,
                *reinterpret_cast<ComponentDesc const*> ( readPointer ),
                data,
                commandBuffers
            );

            if ( component )
            {
                _allComponents.push_back ( component );
                ClassID const classID = component->GetClassID ();

                if ( classID == ClassID::PointLight | classID == ClassID::Reflection | classID == ClassID::StaticMesh )
                {
                    _renderableComponents.emplace_back ( _allComponents.back () );
                }
            }

            commandBuffers += consumed;
            readPointer += read;
        }
    }

    result = android_vulkan::Renderer::CheckVkResult ( vkQueueWaitIdle ( renderer.GetQueue () ),
        "pbr::PBRGame::UploadGPUContent",
        "Can't run upload commands"
    );

    if ( !result ) [[unlikely]]
        return false;

    for ( auto &component : _renderableComponents )
    {
        // NOLINTNEXTLINE - downcast.
        auto &renderableComponent = static_cast<RenderableComponent &> ( *component.get () );
        renderableComponent.FreeTransferResources ( renderer );
    }

    MaterialManager::GetInstance ().FreeTransferResources ( renderer );
    return true;
}

} // namespace pbr
