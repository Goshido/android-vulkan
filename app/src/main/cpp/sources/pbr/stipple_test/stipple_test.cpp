#include <precompiled_headers.hpp>
#include <pbr/stipple_test/stipple_test.hpp>
#include <pbr/coordinate_system.hpp>
#include <pbr/material_manager.hpp>
#include <pbr/mesh_manager.hpp>
#include <pbr/stipple_material.hpp>


namespace pbr::stipple_test {

namespace {

constexpr float FIELD_OF_VIEW = 75.0F;
constexpr float Z_NEAR = 0.1F;
constexpr float Z_FAR = 1.0e+4F;

constexpr uint32_t RESOLUTION_SCALE_WIDTH = 80U;
constexpr uint32_t RESOLUTION_SCALE_HEIGHT = 70U;

} // end of anonymous namespace

//----------------------------------------------------------------------------------------------------------------------

bool StippleTest::IsReady () noexcept
{
    return static_cast<bool> ( _stipple );
}

bool StippleTest::OnFrame ( android_vulkan::Renderer &renderer, double deltaTime ) noexcept
{
    auto const dt = static_cast<float> ( deltaTime );
    _camera.Update ( dt );
    Animate ( dt );

    _renderSession.Begin ( _camera.GetLocalMatrix (), _camera.GetProjectionMatrix () );

    _light.Submit ( _renderSession );
    _floor->Submit ( _renderSession );
    _stipple->Submit ( _renderSession );

    return _renderSession.End ( renderer, deltaTime );
}

bool StippleTest::OnInitDevice ( android_vulkan::Renderer &renderer ) noexcept
{
    VkCommandPoolCreateInfo const poolInfo
    {
        .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0U,
        .queueFamilyIndex = renderer.GetQueueFamilyIndex ()
    };

    VkDevice device = renderer.GetDevice ();

    bool result = android_vulkan::Renderer::CheckVkResult (
        vkCreateCommandPool ( device, &poolInfo, nullptr, &_commandPool ),
        "StippleTest::OnInitDevice",
        "Can't create command buffer"
    );

    if ( !result ) [[unlikely]]
        return false;

    AV_SET_VULKAN_OBJECT_NAME ( device, _commandPool, VK_OBJECT_TYPE_COMMAND_POOL, "StippleTest::_commandPool" )

    if ( !_renderSession.OnInitDevice ( renderer ) || !CreateScene ( renderer ) ) [[unlikely]]
        return false;

    _renderSession.FreeTransferResources ( renderer );
    return true;
}

void StippleTest::OnDestroyDevice ( android_vulkan::Renderer &renderer ) noexcept
{
    _renderSession.OnDestroyDevice ( renderer );

    if ( _commandPool != VK_NULL_HANDLE ) [[likely]]
    {
        vkDestroyCommandPool ( renderer.GetDevice (), _commandPool, nullptr );
        _commandPool = VK_NULL_HANDLE;
    }

    _floor = nullptr;
    _stipple = nullptr;

    MeshManager::Destroy ( renderer );
    MaterialManager::Destroy ( renderer );
}

bool StippleTest::OnSwapchainCreated ( android_vulkan::Renderer &renderer ) noexcept
{
    _camera.CaptureInput ();
    VkExtent2D const &surfaceResolution = renderer.GetViewportResolution ();

    _camera.SetProjection ( GXDegToRad ( FIELD_OF_VIEW ),
        static_cast<float> ( surfaceResolution.width ) / static_cast<float> ( surfaceResolution.height ),
        Z_NEAR,
        Z_FAR
    );

    VkExtent2D const &viewport = renderer.GetViewportResolution ();

    VkExtent2D const resolution
    {
        .width = ( viewport.width * RESOLUTION_SCALE_WIDTH / 100U ),
        .height = ( viewport.height * RESOLUTION_SCALE_HEIGHT / 100U )
    };

    return _renderSession.OnSwapchainCreated ( renderer, resolution );
}

void StippleTest::OnSwapchainDestroyed ( android_vulkan::Renderer &renderer ) noexcept
{
    _renderSession.OnSwapchainDestroyed ( renderer.GetDevice () );
    _camera.ReleaseInput ();
}

void StippleTest::Animate ( float deltaTime ) noexcept
{
    constexpr GXVec3 scale ( 0.5F, 2.0F, 0.3F );

    constexpr GXVec3 renderScale ( scale._data[ 0U ] * UNITS_IN_METER,
        scale._data[ 1U ] * UNITS_IN_METER,
        scale._data[ 2U ] * UNITS_IN_METER
    );

    constexpr GXVec3 location ( 0.0F, 2.5F, 0.0F );

    constexpr GXVec3 renderLocation ( location._data[ 0U ] * UNITS_IN_METER,
        location._data[ 1U ] * UNITS_IN_METER,
        location._data[ 2U ] * UNITS_IN_METER
    );

    constexpr GXVec3 rotationAxis ( 0.454F, 0.8677F, -0.20246F );
    constexpr float rotationPeriod = 4.0F;
    constexpr float rotationSpeed = GX_MATH_DOUBLE_PI / rotationPeriod;

    _stippleAnimation += deltaTime * rotationSpeed;

    if ( _stippleAnimation > GX_MATH_DOUBLE_PI )
        _stippleAnimation -= GX_MATH_DOUBLE_PI;

    GXQuat r {};
    r.FromAxisAngle ( rotationAxis, _stippleAnimation );

    GXMat4 transform {};
    transform.From ( r, renderLocation );

    GXVec3 &x = *reinterpret_cast<GXVec3*> ( transform._data[ 0U ] );
    x.Multiply ( x, renderScale._data[ 0U ] );

    GXVec3 &y = *reinterpret_cast<GXVec3*> ( transform._data[ 1U ] );
    y.Multiply ( y, renderScale._data[ 1U ] );

    GXVec3 &z = *reinterpret_cast<GXVec3*> ( transform._data[ 2U ] );
    z.Multiply ( z, renderScale._data[ 2U ] );

    _stipple->SetTransform ( transform );

    _stippleColor._data[ 3U ] = static_cast<uint8_t> ( std::abs ( 128.0F + 127.0F * std::sin ( _stippleAnimation ) ) );
    _stipple->SetColor0 ( _stippleColor );
}

bool StippleTest::CreateScene ( android_vulkan::Renderer &renderer ) noexcept
{
    constexpr uint32_t meshCommandBuffers = 2U;
    constexpr uint32_t materialCommandBuffers = MaterialManager::MaxCommandBufferPerMaterial ();
    constexpr uint32_t commandBufferCount = meshCommandBuffers + materialCommandBuffers;

    std::array<VkCommandBuffer, commandBufferCount> commandBuffers {};
    VkCommandBuffer* cb = commandBuffers.data ();

    VkCommandBufferAllocateInfo const allocateInfo
    {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
        .pNext = nullptr,
        .commandPool = _commandPool,
        .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
        .commandBufferCount = commandBufferCount
    };

    VkDevice device = renderer.GetDevice ();

    bool result = android_vulkan::Renderer::CheckVkResult (
        vkAllocateCommandBuffers ( device, &allocateInfo, cb ),
        "StippleTest::CreateScene",
        "Can't allocate command buffers"
    );

    if ( !result ) [[unlikely]]
        return false;

#if defined ( AV_ENABLE_VVL ) || defined ( AV_ENABLE_RENDERDOC )

    for ( size_t i = 0U; i < commandBufferCount; ++i )
        AV_SET_VULKAN_OBJECT_NAME ( device, cb[ i ], VK_OBJECT_TYPE_COMMAND_BUFFER, "Asset #%zu", i )

#endif // AV_ENABLE_VVL || AV_ENABLE_RENDERDOC

    size_t consumed;
    bool success;

    _floor = std::make_unique<StaticMeshComponent> ( renderer,
        success,
        consumed,
        "pbr/system/unit-cube.mesh2",
        "pbr/assets/System/DefaultCSG.mtl",
        cb,
        nullptr,
        "Floor"
    );

    if ( !success ) [[unlikely]]
        return false;

    constexpr GXVec3 floorLocation ( 0.0F, -0.25F, 0.0F );

    constexpr GXVec3 floorRenderLocation ( floorLocation._data[ 0U ] * UNITS_IN_METER,
        floorLocation._data[ 1U ] * UNITS_IN_METER,
        floorLocation._data[ 2U ] * UNITS_IN_METER
    );

    constexpr GXVec3 floorScale ( 10.0F, 0.5F, 30.0F );

    constexpr GXVec3 floorRenderScale ( floorScale._data[ 0U ] * UNITS_IN_METER,
        floorScale._data[ 1U ] * UNITS_IN_METER,
        floorScale._data[ 2U ] * UNITS_IN_METER
    );

    GXMat4 transform {};
    transform.Translation ( floorRenderLocation );
    transform._data[ 0U ][ 0U ] = floorRenderScale._data[ 0U ];
    transform._data[ 1U ][ 1U ] = floorRenderScale._data[ 1U ];
    transform._data[ 2U ][ 2U ] = floorRenderScale._data[ 2U ];
    _floor->SetTransform ( transform );

    cb += consumed;

    MaterialRef stippleMaterial = std::make_shared<StippleMaterial> ();

    _stipple = std::make_unique<StaticMeshComponent> ( renderer,
        success,
        consumed,
        "pbr/system/unit-cube.mesh2",
        stippleMaterial,
        cb,
        nullptr
    );

    if ( !success ) [[unlikely]]
        return false;

    _stipple->SetColor0 ( _stippleColor );

    result = android_vulkan::Renderer::CheckVkResult ( vkQueueWaitIdle ( renderer.GetQueue () ),
        "StippleTest::CreateScene",
        "Can't run upload commands"
    );

    if ( !result ) [[unlikely]]
        return false;

    _floor->FreeTransferResources ( renderer );
    _stipple->FreeTransferResources ( renderer );
    MaterialManager::GetInstance ().FreeTransferResources ( renderer );

    constexpr GXVec3 lightBounds ( 100.0F, 100.0F, 100.0F );

    constexpr GXVec3 lightRenderBounds ( lightBounds._data[ 0U ] * UNITS_IN_METER,
        lightBounds._data[ 1U ] * UNITS_IN_METER,
        lightBounds._data[ 2U ] * UNITS_IN_METER
    );

    _light.SetBoundDimensions ( lightRenderBounds );
    _light.SetIntensity ( 500.0F );

    constexpr GXVec3 lightLocation ( 0.0F, 15.0F, -1.5F );

    constexpr GXVec3 lightRenderLocation ( lightLocation._data[ 0U ] * UNITS_IN_METER,
        lightLocation._data[ 1U ] * UNITS_IN_METER,
        lightLocation._data[ 2U ] * UNITS_IN_METER
    );

    _light.SetLocation ( lightRenderLocation );

    constexpr GXVec3 cameraLocation ( 15.49F, 133.74F, -29.841F );

    _camera.SetLocation ( cameraLocation );
    _camera.SetRotation ( 0.9661F, -0.3127F );
    _camera.Update ( 0.0F );

    return true;
}

} // namespace pbr::stipple_test
