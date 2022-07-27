#include <pbr/pbr_game.h>
#include <pbr/actor_desc.h>
#include <pbr/cube_map_manager.h>
#include <pbr/material_manager.h>
#include <pbr/mesh_manager.h>
#include <pbr/renderable_component.h>
#include <pbr/scene_desc.h>
#include <gamepad.h>

GX_DISABLE_COMMON_WARNINGS

#include <cassert>

GX_RESTORE_WARNING_STATE


namespace pbr {

constexpr static char const* SCENES[] =
{
    "pbr/assets/N7_ADM_Reception.scene",
    "pbr/assets/N7_ENG_Injection.scene"
};

constexpr static size_t ACTIVE_SCENE = 0U;
static_assert ( std::size ( SCENES ) > ACTIVE_SCENE );

[[maybe_unused]] constexpr static uint32_t SCENE_DESC_FORMAT_VERSION = 3U;

constexpr static float FIELD_OF_VIEW = 75.0F;
constexpr static float Z_NEAR = 0.1F;
constexpr static float Z_FAR = 1.0e+4F;

constexpr static uint32_t RESOLUTION_SCALE_WIDTH = 80U;
constexpr static uint32_t RESOLUTION_SCALE_HEIGHT = 70U;

//----------------------------------------------------------------------------------------------------------------------

bool PBRGame::IsReady () noexcept
{
    return !_allComponents.empty ();
}

bool PBRGame::OnFrame ( android_vulkan::Renderer &renderer, double deltaTime ) noexcept
{
    auto const dt = static_cast<float> ( deltaTime );

    _camera.Update ( dt );
    GXMat4 const& cameraLocal = _camera.GetLocalMatrix ();
    _renderSession.Begin ( cameraLocal, _camera.GetProjectionMatrix () );

    for ( auto& component : _renderableComponents )
    {
        // NOLINTNEXTLINE - downcast.
        auto& renderableComponent = static_cast<RenderableComponent&> ( *component.get () );
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
        .flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
        .queueFamilyIndex = renderer.GetQueueFamilyIndex ()
    };

    VkDevice device = renderer.GetDevice ();

    bool result = android_vulkan::Renderer::CheckVkResult (
        vkCreateCommandPool ( device, &createInfo, nullptr, &_commandPool ),
        "pbr::PBRGame::OnInit",
        "Can't create command pool"
    );

    if ( !result )
        return false;

    AV_REGISTER_COMMAND_POOL ( "pbr::PBRGame::_commandPool" )

    if ( !_renderSession.OnInitDevice ( renderer ) )
    {
       OnDestroyDevice ( device );
       return false;
    }

    if ( !UploadGPUContent ( renderer ) )
    {
        OnDestroyDevice ( device );
        return false;
    }

    _renderSession.FreeTransferResources ( device );
    return true;
}

void PBRGame::OnDestroyDevice ( VkDevice device ) noexcept
{
    _allComponents.clear ();
    _renderableComponents.clear ();

    _renderSession.OnDestroyDevice ( device );
    DestroyCommandPool ( device );

    MeshManager::Destroy ( device );
    MaterialManager::Destroy ( device );
    CubeMapManager::Destroy ( device );
}

bool PBRGame::OnSwapchainCreated ( android_vulkan::Renderer &renderer ) noexcept
{
    VkExtent2D resolution = renderer.GetViewportResolution ();
    resolution.width = ( resolution.width * RESOLUTION_SCALE_WIDTH / 100U );
    resolution.height = ( resolution.height * RESOLUTION_SCALE_HEIGHT / 100U );

    VkExtent2D const& surfaceResolution = renderer.GetViewportResolution ();

    _camera.SetProjection ( GXDegToRad ( FIELD_OF_VIEW ),
        static_cast<float> ( surfaceResolution.width ) / static_cast<float> ( surfaceResolution.height ),
        Z_NEAR,
        Z_FAR
    );

    if ( !_renderSession.OnSwapchainCreated ( renderer, resolution ) )
        return false;

    _camera.CaptureInput ();
    return true;
}

void PBRGame::OnSwapchainDestroyed ( VkDevice device ) noexcept
{
    _camera.ReleaseInput ();
    _renderSession.OnSwapchainDestroyed ( device );
}

void PBRGame::DestroyCommandPool ( VkDevice device ) noexcept
{
    if ( _commandPool == VK_NULL_HANDLE )
        return;

    vkDestroyCommandPool ( device, _commandPool, nullptr );
    _commandPool = VK_NULL_HANDLE;
    AV_UNREGISTER_COMMAND_POOL ( "pbr::PBRGame::_commandPool" )
}

bool PBRGame::UploadGPUContent ( android_vulkan::Renderer& renderer ) noexcept
{
    android_vulkan::File file ( SCENES[ ACTIVE_SCENE ] );

    if ( !file.LoadContent () )
        return false;

    std::vector<uint8_t> const& content = file.GetContent ();
    uint8_t const* data = content.data ();
    auto const* sceneDesc = reinterpret_cast<SceneDesc const*> ( data );

    // Sanity checks.
    static_assert ( sizeof ( GXVec3 ) == sizeof ( sceneDesc->_viewerLocation ) );
    assert ( sceneDesc->_formatVersion == SCENE_DESC_FORMAT_VERSION );

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

    if ( !result )
        return false;

    VkCommandBuffer const* commandBuffers = _commandBuffers.data ();
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
                    _renderableComponents.emplace_back (
                        std::reference_wrapper<ComponentRef> { _allComponents.back () }
                    );
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

    if ( !result )
        return false;

    for ( auto& component : _renderableComponents )
    {
        // NOLINTNEXTLINE - downcast.
        auto& renderableComponent = static_cast<RenderableComponent&> ( *component.get () );
        renderableComponent.FreeTransferResources ( device );
    }

    return true;
}

} // namespace pbr
