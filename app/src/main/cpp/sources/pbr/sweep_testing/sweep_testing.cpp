#include <pbr/sweep_testing/sweep_testing.h>
#include <pbr/material_manager.h>
#include <pbr/mesh_manager.h>


namespace pbr::sweep_testing {

constexpr static float const FIELD_OF_VIEW = 75.0F;
constexpr static float const Z_NEAR = 0.1F;
constexpr static float const Z_FAR = 1.0e+4F;

constexpr static uint32_t const RESOLUTION_SCALE_WIDTH = 80U;
constexpr static uint32_t const RESOLUTION_SCALE_HEIGHT = 70U;

//----------------------------------------------------------------------------------------------------------------------

bool SweepTesting::IsReady () noexcept
{
    return static_cast<bool> ( _overlay );
}

bool SweepTesting::OnFrame ( android_vulkan::Renderer &renderer, double deltaTime ) noexcept
{
    auto const dt = static_cast<float> ( deltaTime );
    _physics.Simulate ( dt );
    _camera.Update ( dt );

    _renderSession.Begin ( _camera.GetLocalMatrix (), _camera.GetProjectionMatrix () );
    _light.Submit ( _renderSession );

    for ( auto& actor : _actors )
        actor.Submit ( _renderSession );

    return _renderSession.End ( renderer, deltaTime );
}

bool SweepTesting::OnInitDevice ( android_vulkan::Renderer &renderer ) noexcept
{
    if ( !CreateCommandPool ( renderer ) )
    {
        OnDestroyDevice ( renderer.GetDevice () );
        return false;
    }

    if ( !_renderSession.OnInitDevice ( renderer ) )
    {
        OnDestroyDevice ( renderer.GetDevice () );
        return false;
    }

    if ( !CreateScene ( renderer ) )
    {
        OnDestroyDevice ( renderer.GetDevice () );
        return false;
    }

    return true;
}

void SweepTesting::OnDestroyDevice ( VkDevice device ) noexcept
{
    _renderSession.OnDestroyDevice ( device );

    for ( auto& actor : _actors )
        actor.Destroy ();

    if ( _overlay )
    {
        _overlay->FreeResources ( device );
        _overlay.reset ();
    }

    DestroyCommandPool ( device );

    MeshManager::Destroy ( device );
    MaterialManager::Destroy ( device );
}

bool SweepTesting::OnSwapchainCreated ( android_vulkan::Renderer &renderer ) noexcept
{
    VkExtent2D const& surfaceResolution = renderer.GetViewportResolution ();

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

    if ( !_renderSession.OnSwapchainCreated ( renderer, resolution, _commandPool ) )
        return false;

    _camera.CaptureInput ();
    _physics.Resume ();
    return true;
}

void SweepTesting::OnSwapchainDestroyed ( VkDevice device ) noexcept
{
    Camera::ReleaseInput ();
    _physics.Pause ();
    _renderSession.OnSwapchainDestroyed ( device );
}

bool SweepTesting::CreateCommandPool ( android_vulkan::Renderer &renderer ) noexcept
{
    VkCommandPoolCreateInfo const info
    {
        .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
        .pNext = nullptr,
        .flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
        .queueFamilyIndex = renderer.GetQueueFamilyIndex ()
    };

    bool const result = android_vulkan::Renderer::CheckVkResult (
        vkCreateCommandPool ( renderer.GetDevice (), &info, nullptr, &_commandPool ),
        "SweepTesting::CreateCommandPool",
        "Can't create command pool"
    );

    if ( !result )
        return false;

    AV_REGISTER_COMMAND_POOL ( "SweepTesting::_commandPool" )
    return true;
}

void SweepTesting::DestroyCommandPool ( VkDevice device ) noexcept
{
    if ( _commandPool == VK_NULL_HANDLE )
        return;

    vkDestroyCommandPool ( device, _commandPool, nullptr );
    _commandPool = VK_NULL_HANDLE;
    AV_UNREGISTER_COMMAND_POOL ( "SweepTesting::_commandPool" )
}

bool SweepTesting::CreateScene ( android_vulkan::Renderer &renderer ) noexcept
{
    _camera.SetLocation ( GXVec3 ( 66.5348F, 53.0677F, -25.9031F ) );
    _camera.SetRotation ( 0.9079F, -1.044F );
    _camera.Update ( 0.0F );

    constexpr GXVec3 lightLocation ( 0.5F, 2.0F, 0.0F );

    GXVec3 lightLocationRender {};
    constexpr float renderScale = 32.0F;
    lightLocationRender.Multiply ( lightLocation, renderScale );
    _light.SetLocation ( lightLocationRender );

    constexpr float lightBounds = 160.0F;
    _light.SetBoundDimensions ( lightBounds, lightBounds, lightBounds );
    _light.SetIntensity ( 16.0F );

    constexpr size_t actorMeshCommandBuffers = 1U;
    constexpr size_t materialCommandBuffers = 1U;
    constexpr size_t overlayTextureCommandBuffers = 1U;
    constexpr size_t comBuffs = actorMeshCommandBuffers + materialCommandBuffers + overlayTextureCommandBuffers;

    VkCommandBufferAllocateInfo const allocateInfo
    {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
        .pNext = nullptr,
        .commandPool = _commandPool,
        .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
        .commandBufferCount = static_cast<uint32_t> ( comBuffs )
    };

    std::array<VkCommandBuffer, comBuffs> commandBuffers {};
    VkCommandBuffer* cb = commandBuffers.data ();
    VkDevice device = renderer.GetDevice ();

    bool result = android_vulkan::Renderer::CheckVkResult (
        vkAllocateCommandBuffers ( device, &allocateInfo, cb ),
        "SweepTesting::CreateScene",
        "Can't allocate command buffers"
    );

    if ( !result )
        return false;

    constexpr GXVec3 blockSize ( 0.5F, 0.2F, 0.4F );
    constexpr float gapSize = 0.2F;

    constexpr GXVec2 blocks ( static_cast<float> ( GRID_X ), static_cast<float> ( GRID_Z ) );
    constexpr GXVec2 gaps ( static_cast<float> ( GRID_X - 1U ), static_cast<float> ( GRID_Z - 1U ) );
    constexpr GXVec2 offset ( blockSize._data[ 0U ] + gapSize, blockSize._data[ 2U ] + gapSize );

    constexpr GXVec2 bounds ( blocks._data[ 0U ] * blockSize._data[ 0U ] + gapSize * gaps._data[ 0U ],
        blocks._data[ 1U ] * blockSize._data[ 2U ] + gapSize * gaps._data[ 1U ]
    );

    constexpr GXVec2 alpha ( blockSize._data[ 0U ] - bounds._data[ 0U ],
        blockSize._data[ 2U ] - bounds._data[ 1U ]
    );

    constexpr GXVec2 anchor ( lightLocation._data[ 0U ] + 0.5F * alpha._data[ 0U ],
        lightLocation._data[ 2U ] + 0.5F * alpha._data[ 1U ]
    );

    GXVec3 location ( 0.0F, 0.0F, 0.0F );

    for ( size_t x = 0U, ind = 0U; x < GRID_X; ++x, ind += GRID_Z )
    {
        location._data[ 0U ] = anchor._data[ 0U ] + static_cast<float> ( x ) * offset._data[ 0U ];

        for ( size_t z = 0U; z < GRID_Z; ++z )
        {
            location._data[ 2U ] = anchor._data[ 1U ] + static_cast<float> ( z ) * offset._data[ 1U ];
            size_t consumed;

            if ( !_actors[ ind + z ].Init ( renderer, _physics, consumed, cb, location, blockSize ) )
                return false;

            cb += consumed;
        }
    }

    _overlay = std::make_shared<android_vulkan::Texture2D> ();
    android_vulkan::Texture2D& t = *_overlay;
    constexpr uint8_t const color[ 4U ] = { 115U, 185U, 0U, 255U };

    result = t.UploadData ( renderer,
        color,
        sizeof ( color ),

        VkExtent2D
        {
            .width = 1U,
            .height = 1U
        },

        VK_FORMAT_R8G8B8A8_SRGB,
        false,
        *cb
    );

    if ( !result )
        return false;

    t.AssignName ( "Overlay" );

    result = android_vulkan::Renderer::CheckVkResult ( vkQueueWaitIdle ( renderer.GetQueue () ),
        "SweepTesting::CreateScene",
        "Can't run upload commands"
    );

    if ( !result )
        return false;

    t.FreeTransferResources ( device );

    for ( auto& actor : _actors )
        actor.FreeTransferResources ( device );

    return true;
}

} // namespace pbr::sweep_testing