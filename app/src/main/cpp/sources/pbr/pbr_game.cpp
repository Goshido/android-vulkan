#include <pbr/pbr_game.h>
#include <vulkan_utils.h>
#include <pbr/component.h>
#include <pbr/material_manager.h>
#include <pbr/scene_desc.h>
#include <pbr/mesh_manager.h>


namespace pbr {

constexpr static char const* SCENE = "pbr/N7_ADM_Reception/scene.scene";
constexpr static uint32_t const SCENE_DESC_FORMAT_VERSION = 1U;

constexpr static float const FIELD_OF_VIEW = 60.0F;
constexpr static float const Z_NEAR = 0.1F;
constexpr static float const Z_FAR = 1.0e+3F;

constexpr static uint32_t const RESOLUTION_SCALE_WIDTH = 100U;
constexpr static uint32_t const RESOLUTION_SCALE_HEIGHT = 100U;

//----------------------------------------------------------------------------------------------------------------------

PBRGame::PBRGame ():
    _camera {},
    _commandPool ( VK_NULL_HANDLE ),
    _commandBuffers {},
    _renderSession {},
    _components {}
{
    // NOTHING
}

bool PBRGame::IsReady ()
{
    return !_components.empty ();
}

bool PBRGame::OnInit ( android_vulkan::Renderer &renderer )
{
    VkExtent2D resolution = renderer.GetViewportResolution ();
    resolution.width = resolution.width * RESOLUTION_SCALE_WIDTH / 100U;
    resolution.height = resolution.height * RESOLUTION_SCALE_HEIGHT / 100U;

    if ( !_renderSession.Init ( renderer, resolution ) )
    {
        OnDestroy ( renderer );
        return false;
    }

    if ( !UploadGPUContent ( renderer ) )
    {
        OnDestroy ( renderer );
        return false;
    }

    VkExtent2D const& surfaceResolution = renderer.GetViewportResolution ();

    _camera.SetProjection ( GXDegToRad ( FIELD_OF_VIEW ),
        surfaceResolution.width / static_cast<float> ( surfaceResolution.height ),
        Z_NEAR,
        Z_FAR
    );

    _camera.CaptureInput ();
    return true;
}

bool PBRGame::OnFrame ( android_vulkan::Renderer &renderer, double deltaTime )
{
    _camera.Update ( static_cast<float> ( deltaTime ) );
    _renderSession.Begin ( _camera.GetViewMatrix (), _camera.GetProjectionMatrix () );

    for ( auto &component : _components )
        component->Submit ( _renderSession );

    return _renderSession.End ( ePresentTarget::Normal, renderer );
}

bool PBRGame::OnDestroy ( android_vulkan::Renderer &renderer )
{
    const bool result = renderer.CheckVkResult ( vkDeviceWaitIdle ( renderer.GetDevice () ),
        "PBRGame::OnDestroy",
        "Can't wait device idle"
    );

    if ( !result )
        return false;

    Camera::ReleaseInput ();
    DestroyCommandPool ( renderer );

    _components.clear ();
    _renderSession.Destroy( renderer );

    MeshManager::Destroy ( renderer );
    MaterialManager::Destroy ( renderer );

    return true;
}

bool PBRGame::CreateCommandPool ( android_vulkan::Renderer &renderer, size_t commandBufferCount )
{
    VkCommandPoolCreateInfo createInfo;
    createInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    createInfo.pNext = nullptr;
    createInfo.flags = 0U;
    createInfo.queueFamilyIndex = renderer.GetQueueFamilyIndex ();

    VkDevice device = renderer.GetDevice ();

    bool result = renderer.CheckVkResult (
        vkCreateCommandPool ( device, &createInfo, nullptr, &_commandPool ),
        "PBRGame::CreateCommandPool",
        "Can't create command pool"
    );

    if ( !result )
        return false;

    AV_REGISTER_COMMAND_POOL ( "PBRGame::_commandPool" )

    _commandBuffers.resize ( commandBufferCount );

    VkCommandBufferAllocateInfo allocateInfo;
    allocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocateInfo.pNext = nullptr;
    allocateInfo.commandBufferCount = static_cast<uint32_t> ( commandBufferCount );
    allocateInfo.commandPool = _commandPool;
    allocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;

    result = renderer.CheckVkResult (
        vkAllocateCommandBuffers ( device, &allocateInfo, _commandBuffers.data () ),
        "PBRGame::CreateCommandPool",
        "Can't allocate command buffers"
    );

    if ( result )
        return true;

    _commandBuffers.clear ();
    return false;
}

void PBRGame::DestroyCommandPool ( android_vulkan::Renderer &renderer )
{
    if ( _commandPool == VK_NULL_HANDLE )
        return;

    vkDestroyCommandPool ( renderer.GetDevice (), _commandPool, nullptr );
    _commandPool = VK_NULL_HANDLE;
    AV_UNREGISTER_COMMAND_POOL ( "PBRGame::_commandPool" )
}

bool PBRGame::UploadGPUContent ( android_vulkan::Renderer& renderer )
{
    android_vulkan::File file ( SCENE );

    if ( !file.LoadContent () )
        return false;

    std::vector<uint8_t> const& content = file.GetContent ();
    uint8_t const* data = content.data ();
    auto const* sceneDesc = reinterpret_cast<pbr::SceneDesc const*> ( data );

    // Sanity checks.
    static_assert ( sizeof ( GXMat4 ) == sizeof ( sceneDesc->_viewerLocal ) );
    assert ( sceneDesc->_formatVersion == SCENE_DESC_FORMAT_VERSION );

    auto const* viewerLocal = reinterpret_cast<GXMat4 const*> ( &sceneDesc->_viewerLocal );
    GXVec3 location;
    viewerLocal->GetW ( location );
    _camera.SetLocation ( location );

    if ( !CreateCommandPool ( renderer, static_cast<size_t> ( sceneDesc->_textureCount + sceneDesc->_meshCount ) ) )
        return false;

    VkCommandBuffer const* commandBuffers = _commandBuffers.data ();
    uint8_t const* readPointer = data + sizeof ( pbr::SceneDesc );

    auto const limit = static_cast<size_t const> ( sceneDesc->_componentCount );
    size_t consumed = 0U;
    size_t read = 0U;

    for ( size_t i = 0U; i < limit; ++i )
    {
        ComponentRef component = Component::Create ( consumed,
            read,
            *reinterpret_cast<ComponentDesc const*> ( readPointer ),
            data,
            renderer,
            commandBuffers
        );

        if ( component )
            _components.push_back ( component );

        commandBuffers += consumed;
        readPointer += read;
    }

    bool const result = renderer.CheckVkResult ( vkQueueWaitIdle ( renderer.GetQueue () ),
        "PBRGame::UploadGPUContent",
        "Can't run upload commands"
    );

    if ( !result )
        return false;

    for ( auto &component : _components )
        component->FreeTransferResources ( renderer );

    return true;
}

} // namespace pbr
