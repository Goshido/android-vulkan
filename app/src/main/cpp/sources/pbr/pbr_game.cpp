#include <pbr/pbr_game.h>
#include <vulkan_utils.h>
#include <pbr/component.h>
#include <pbr/cube_map_manager.h>
#include <pbr/material_manager.h>
#include <pbr/scene_desc.h>
#include <pbr/mesh_manager.h>

GX_DISABLE_COMMON_WARNINGS

#include <cassert>

GX_RESTORE_WARNING_STATE


namespace pbr {

constexpr static char const* SCENES[] =
{
    "pbr/assets/N7_ADM_Reception.scene",
    "pbr/assets/N7_ENG_Injection.scene"
};

constexpr static size_t ACTIVE_SCENE = 1U;
static_assert ( std::size ( SCENES ) > ACTIVE_SCENE );

[[maybe_unused]] constexpr static uint32_t const SCENE_DESC_FORMAT_VERSION = 2U;

constexpr static float const FIELD_OF_VIEW = 75.0F;
constexpr static float const Z_NEAR = 0.1F;
constexpr static float const Z_FAR = 1.0e+3F;

constexpr static uint32_t const RESOLUTION_SCALE_WIDTH = 100U;
constexpr static uint32_t const RESOLUTION_SCALE_HEIGHT = 100U;

//----------------------------------------------------------------------------------------------------------------------

PBRGame::PBRGame () noexcept:
    _camera {},
    _commandPool ( VK_NULL_HANDLE ),
    _commandBuffers {},
    _renderSession {},
    _components {},
    _pointLight ( nullptr ),
    _lightPhase ( 0.0F ),
    _lightOrigin {}
{
    // NOTHING
}

bool PBRGame::IsReady ()
{
    return !_components.empty ();
}

bool PBRGame::OnFrame ( android_vulkan::Renderer &renderer, double deltaTime )
{
    _lightPhase += static_cast<float> ( deltaTime );

    GXVec3 offset ( std::sin ( _lightPhase ), 0.0F, std::cos ( _lightPhase ) );
    offset.Multiply ( offset, 32.0F );

    GXVec3 target;
    target.Sum ( _lightOrigin, offset );
    _pointLight->SetLocation ( target );

    _camera.Update ( static_cast<float> ( deltaTime ) );
    _renderSession.Begin ( _camera.GetLocalMatrix (), _camera.GetProjectionMatrix () );

    for ( auto& component : _components )
        component->Submit ( _renderSession );

    return _renderSession.End ( renderer, deltaTime );
}

bool PBRGame::OnInitDevice ( android_vulkan::Renderer &renderer )
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
        "PBRGame::OnInit",
        "Can't create command pool"
    );

    if ( !result )
        return false;

    AV_REGISTER_COMMAND_POOL ( "PBRGame::_commandPool" )

   _renderSession.OnInitDevice ();

    if ( !UploadGPUContent ( renderer ) )
    {
        OnDestroyDevice ( device );
        return false;
    }

    return true;
}

void PBRGame::OnDestroyDevice ( VkDevice device )
{
    _components.clear ();
    _renderSession.OnDestroyDevice ();
    DestroyCommandPool ( device );

    MeshManager::Destroy ( device );
    MaterialManager::Destroy ( device );
    CubeMapManager::Destroy ( device );
}

bool PBRGame::OnSwapchainCreated ( android_vulkan::Renderer &renderer )
{
    VkExtent2D resolution = renderer.GetViewportResolution ();
    resolution.width = resolution.width * RESOLUTION_SCALE_WIDTH / 100U;
    resolution.height = resolution.height * RESOLUTION_SCALE_HEIGHT / 100U;

    VkExtent2D const& surfaceResolution = renderer.GetViewportResolution ();

    _camera.SetProjection ( GXDegToRad ( FIELD_OF_VIEW ),
        surfaceResolution.width / static_cast<float> ( surfaceResolution.height ),
        Z_NEAR,
        Z_FAR
    );

    if ( !_renderSession.OnSwapchainCreated ( renderer, resolution, _commandPool ) )
        return false;

    _camera.CaptureInput ();
    return true;
}

void PBRGame::OnSwapchainDestroyed ( VkDevice device )
{
    Camera::ReleaseInput ();
    _renderSession.OnSwapchainDestroyed ( device );
}

void PBRGame::DestroyCommandPool ( VkDevice device )
{
    if ( _commandPool == VK_NULL_HANDLE )
        return;

    vkDestroyCommandPool ( device, _commandPool, nullptr );
    _commandPool = VK_NULL_HANDLE;
    AV_UNREGISTER_COMMAND_POOL ( "PBRGame::_commandPool" )
}

bool PBRGame::UploadGPUContent ( android_vulkan::Renderer& renderer )
{
    android_vulkan::File file ( SCENES[ ACTIVE_SCENE ] );

    if ( !file.LoadContent () )
        return false;

    std::vector<uint8_t> const& content = file.GetContent ();
    uint8_t const* data = content.data ();
    auto const* sceneDesc = reinterpret_cast<pbr::SceneDesc const*> ( data );

    // Sanity checks.
    static_assert ( sizeof ( GXVec3 ) == sizeof ( sceneDesc->_viewerLocation ) );
    assert ( sceneDesc->_formatVersion == SCENE_DESC_FORMAT_VERSION );

    _camera.SetLocation ( *reinterpret_cast<GXVec3 const*> ( &sceneDesc->_viewerLocation ) );
    _camera.SetRotation ( sceneDesc->_viewerPitch, sceneDesc->_viewerYaw );

    auto const comBuffs =
        static_cast<size_t> ( sceneDesc->_textureCount + sceneDesc->_meshCount + sceneDesc->_envMapCount );

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
        "PBRGame::UploadGPUContent",
        "Can't allocate command buffers"
    );

    if ( !result )
        return false;

    VkCommandBuffer const* commandBuffers = _commandBuffers.data ();
    uint8_t const* readPointer = data + sizeof ( pbr::SceneDesc );

    auto const limit = static_cast<size_t const> ( sceneDesc->_componentCount );
    size_t consumed = 0U;
    size_t read = 0U;

    for ( size_t i = 0U; i < limit; ++i )
    {
        ComponentRef component = Component::Create ( renderer,
            consumed,
            read,
            *reinterpret_cast<ComponentDesc const*> ( readPointer ),
            data,
            commandBuffers
        );

        if ( component )
        {
            _components.push_back ( component );

            if ( component->GetClassID () == ClassID::PointLight )
            {
                // Note it's safe to cast like that here. "NOLINT" is a clang-tidy control comment.
                auto const* raw = static_cast<PointLightComponent*> ( component.get () ); // NOLINT
                _pointLight = static_cast<PointLight*> ( raw->GetLight ().get () ); // NOLINT
                _lightOrigin = _pointLight->GetLocation ();
            }
        }

        commandBuffers += consumed;
        readPointer += read;
    }

    result = android_vulkan::Renderer::CheckVkResult ( vkQueueWaitIdle ( renderer.GetQueue () ),
        "PBRGame::UploadGPUContent",
        "Can't run upload commands"
    );

    if ( !result )
        return false;

    for ( auto& component : _components )
        component->FreeTransferResources ( device );

    return true;
}

} // namespace pbr
