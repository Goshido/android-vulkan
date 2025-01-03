#include <precompiled_headers.hpp>
#include <pbr/sweep_testing/sweep_testing.hpp>
#include <pbr/coordinate_system.hpp>
#include <pbr/material_manager.hpp>
#include <pbr/mesh_manager.hpp>
#include <gamepad.hpp>


namespace pbr::sweep_testing {

namespace {

constexpr float FIELD_OF_VIEW = 75.0F;
constexpr float Z_NEAR = 0.1F;
constexpr float Z_FAR = 1.0e+4F;

constexpr uint32_t RESOLUTION_SCALE_WIDTH = 80U;
constexpr uint32_t RESOLUTION_SCALE_HEIGHT = 70U;

} // end of anonymous namespace

//----------------------------------------------------------------------------------------------------------------------

bool SweepTesting::IsReady () noexcept
{
    return static_cast<bool> ( _overlay );
}

bool SweepTesting::OnFrame ( android_vulkan::Renderer &renderer, double deltaTime ) noexcept
{
    auto const dt = static_cast<float> ( deltaTime );

    _renderSession.Begin ( _camera.GetLocalMatrix (), _camera.GetProjectionMatrix () );
    _light.Submit ( _renderSession );

    for ( auto &body : _bodies )
        body.Update ( dt );

    _physics.Simulate ( dt );
    _camera.Update ( dt );
    _sweep.Update ( dt );

    DoSweepTest ();

    for ( auto &body : _bodies )
        body.Submit ( _renderSession );

    _sweep.Submit ( _renderSession );
    return _renderSession.End ( renderer, deltaTime );
}

bool SweepTesting::OnInitDevice ( android_vulkan::Renderer &renderer ) noexcept
{
    if ( !CreateCommandPool ( renderer ) || !_renderSession.OnInitDevice ( renderer ) || !CreateScene ( renderer ) )
    {
        [[unlikely]]
        return false;
    }

    _renderSession.FreeTransferResources ( renderer );
    return true;
}

void SweepTesting::OnDestroyDevice ( android_vulkan::Renderer &renderer ) noexcept
{
    _renderSession.OnDestroyDevice ( renderer );
    _sweep.Destroy ();

    for ( auto &body : _bodies )
        body.Destroy ();

    if ( _overlay )
    {
        _overlay->FreeResources ( renderer );
        _overlay.reset ();
    }

    DestroyCommandPool ( renderer.GetDevice () );
    _physics.Reset ();

    MeshManager::Destroy ( renderer );
    MaterialManager::Destroy ( renderer );
}

bool SweepTesting::OnSwapchainCreated ( android_vulkan::Renderer &renderer ) noexcept
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

    BindControls ();
    _physics.Resume ();
    return true;
}

void SweepTesting::OnSwapchainDestroyed ( android_vulkan::Renderer &renderer ) noexcept
{
    UnbindControls ();
    _physics.Pause ();
    _renderSession.OnSwapchainDestroyed ( renderer.GetDevice () );
}

void SweepTesting::BindControls () noexcept
{
    android_vulkan::Gamepad &gamepad = android_vulkan::Gamepad::GetInstance ();

    gamepad.BindKey ( this,
        &SweepTesting::OnSwitchControls,
        android_vulkan::eGamepadKey::Y,
        android_vulkan::eButtonState::Down
    );

    if ( _controlType == eControlType::Camera )
    {
        _camera.CaptureInput ();
        return;
    }

    _sweep.CaptureInput ( _camera.GetLocalMatrix () );
}

void SweepTesting::UnbindControls () noexcept
{
    android_vulkan::Gamepad &gamepad = android_vulkan::Gamepad::GetInstance ();
    gamepad.UnbindKey ( android_vulkan::eGamepadKey::Y,android_vulkan::eButtonState::Down );

    if ( _controlType == eControlType::Camera )
    {
        _camera.ReleaseInput ();
        return;
    }

    _sweep.ReleaseInput ();
}

bool SweepTesting::CreateCommandPool ( android_vulkan::Renderer &renderer ) noexcept
{
    VkCommandPoolCreateInfo const info
    {
        .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0U,
        .queueFamilyIndex = renderer.GetQueueFamilyIndex ()
    };

    bool const result = android_vulkan::Renderer::CheckVkResult (
        vkCreateCommandPool ( renderer.GetDevice (), &info, nullptr, &_commandPool ),
        "SweepTesting::CreateCommandPool",
        "Can't create command pool"
    );

    if ( !result ) [[unlikely]]
        return false;

    AV_SET_VULKAN_OBJECT_NAME ( renderer.GetDevice (),
        _commandPool,
        VK_OBJECT_TYPE_COMMAND_POOL,
        "pbr::sweep_testing::SweepTesting::_commandPool"
    )

    return true;
}

void SweepTesting::DestroyCommandPool ( VkDevice device ) noexcept
{
    if ( _commandPool == VK_NULL_HANDLE ) [[unlikely]]
        return;

    vkDestroyCommandPool ( device, _commandPool, nullptr );
    _commandPool = VK_NULL_HANDLE;
}

bool SweepTesting::CreateScene ( android_vulkan::Renderer &renderer ) noexcept
{
    _camera.SetLocation ( GXVec3 ( 66.5348F, 53.0677F, -25.9031F ) );
    _camera.SetRotation ( 0.9079F, -1.044F );
    _camera.Update ( 0.0F );

    constexpr GXVec3 lightLocation ( 0.5F, 2.0F, 0.0F );

    GXVec3 lightLocationRender {};
    lightLocationRender.Multiply ( lightLocation, UNITS_IN_METER );
    _light.SetLocation ( lightLocationRender );

    constexpr float lightBounds = 160.0F;
    _light.SetBoundDimensions ( lightBounds, lightBounds, lightBounds );
    _light.SetIntensity ( 16.0F );

    constexpr size_t actorMeshCommandBuffers = 2U;
    constexpr size_t materialCommandBuffers = 2U;
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

    if ( !result ) [[unlikely]]
        return false;

#if defined ( AV_ENABLE_VVL ) || defined ( AV_ENABLE_RENDERDOC )

    for ( size_t i = 0U; i < comBuffs; ++i )
        AV_SET_VULKAN_OBJECT_NAME ( device, cb[ i ], VK_OBJECT_TYPE_COMMAND_BUFFER, "Asset #%zu", i )

#endif // AV_ENABLE_VVL || AV_ENABLE_RENDERDOC

    _overlay = std::make_shared<android_vulkan::Texture2D> ();
    android_vulkan::Texture2D &t = *_overlay;
    constexpr uint8_t const color[ 4U ] = { 255U, 255U, 255U, 255U };

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
        *cb,
        VK_NULL_HANDLE
    );

    if ( !result ) [[unlikely]]
        return false;

    ++cb;

    t.AssignName ( "Overlay" );

    constexpr GXVec3 blockSize ( 0.5F, 0.2F, 0.4F );
    constexpr float gapSize = 0.3F;

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
    size_t consumed;

    for ( size_t x = 0U, ind = 0U; x < GRID_X; ++x, ind += GRID_Z )
    {
        location._data[ 0U ] = anchor._data[ 0U ] + static_cast<float> ( x ) * offset._data[ 0U ];

        for ( size_t z = 0U; z < GRID_Z; ++z )
        {
            location._data[ 2U ] = anchor._data[ 1U ] + static_cast<float> ( z ) * offset._data[ 1U ];

            if ( !_bodies[ ind + z ].Init ( renderer, _physics, consumed, cb, location, blockSize ) )
                return false;

            cb += consumed;
        }
    }

    if ( !_sweep.Init ( renderer, consumed, cb, GXVec3 ( 0.0F, 0.5F, 0.0F ), GXVec3 ( 0.6F, 0.2F, 0.7F ) ) )
    {
        [[unlikely]]
        return false;
    }

    result = android_vulkan::Renderer::CheckVkResult ( vkQueueWaitIdle ( renderer.GetQueue () ),
        "SweepTesting::CreateScene",
        "Can't run upload commands"
    );

    if ( !result ) [[unlikely]]
        return false;

    _sweep.FreeTransferResources ( renderer );
    _sweep.SetOverlay ( _overlay );

    t.FreeTransferResources ( renderer );

    for ( auto &body : _bodies )
    {
        body.FreeTransferResources ( renderer );
        body.SetOverlay ( _overlay );
    }

    MaterialManager::GetInstance ().FreeTransferResources ( renderer );
    return true;
}

void SweepTesting::DoSweepTest () noexcept
{
    constexpr uint32_t groups = 0b00000000'00000000'00000000'00000110U;
    _physics.SweepTest ( _sweepResult, _sweep.GetShape (), groups );

    for ( auto &body : _bodies )
        body.DisableOverlay ();

    for ( auto const &s : _sweepResult )
    {
        auto &actorBody = *static_cast<ActorBody*> ( s->GetContext () );
        actorBody.EnableOverlay ();
    }
}

void SweepTesting::OnSwitchControls ( void* context ) noexcept
{
    auto &s = *static_cast<SweepTesting*> ( context );

    s.UnbindControls ();
    s._controlType = s._controlType == eControlType::Camera ? eControlType::SweepObject : eControlType::Camera;
    s.BindControls ();
}

} // namespace pbr::sweep_testing
