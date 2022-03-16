#include <pbr/stipple_test/stipple_test.h>
#include <pbr/material_manager.h>
#include <pbr/mesh_manager.h>
#include <pbr/stipple_material.h>


namespace pbr::stipple_test {

constexpr static float FIELD_OF_VIEW = 75.0F;
constexpr static float Z_NEAR = 0.1F;
constexpr static float Z_FAR = 1.0e+4F;

constexpr static uint32_t RESOLUTION_SCALE_WIDTH = 80U;
constexpr static uint32_t RESOLUTION_SCALE_HEIGHT = 70U;

constexpr static float RENDERER_SCALE = 32.0F;

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
        .flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
        .queueFamilyIndex = renderer.GetQueueFamilyIndex ()
    };

    VkDevice device = renderer.GetDevice ();

    bool result = android_vulkan::Renderer::CheckVkResult (
        vkCreateCommandPool ( device, &poolInfo, nullptr, &_commandPool ),
        "StippleTest::OnInitDevice",
        "Can't create command buffer"
    );

    if ( !result )
        return false;

    AV_REGISTER_COMMAND_POOL ( "StippleTest::_commandPool" )

    if ( !_renderSession.OnInitDevice ( renderer, _commandPool ) )
    {
        OnDestroyDevice ( device );
        return false;
    }

    if ( !CreateScene ( renderer ) )
    {
        OnDestroyDevice ( device );
        return false;
    }

    _renderSession.FreeTransferResources ( device, _commandPool );
    return true;
}

void StippleTest::OnDestroyDevice ( VkDevice device ) noexcept
{
    _renderSession.OnDestroyDevice ( device );

    if ( _commandPool != VK_NULL_HANDLE )
    {
        vkDestroyCommandPool ( device, _commandPool, nullptr );
        _commandPool = VK_NULL_HANDLE;
        AV_UNREGISTER_COMMAND_POOL ( "StippleTest::_commandPool" )
    }

    _floor = nullptr;

    auto freeTexture = [ device ] ( Texture2DRef &texture ) noexcept {
        if ( !texture )
            return;

        texture->FreeResources ( device );
    };

    // NOLINTNEXTLINE
    auto& m = static_cast<GeometryPassMaterial&> ( *_stipple->GetMaterial () );
    freeTexture ( m.GetAlbedo () );
    freeTexture ( m.GetEmission () );
    freeTexture ( m.GetMask () );
    freeTexture ( m.GetNormal () );
    freeTexture ( m.GetParam () );

    _stipple = nullptr;

    MeshManager::Destroy ( device );
    MaterialManager::Destroy ( device );
}

bool StippleTest::OnSwapchainCreated ( android_vulkan::Renderer &renderer ) noexcept
{
    _camera.CaptureInput ();
    VkExtent2D const& surfaceResolution = renderer.GetViewportResolution ();

    _camera.SetProjection ( GXDegToRad ( FIELD_OF_VIEW ),
        static_cast<float> ( surfaceResolution.width ) / static_cast<float> ( surfaceResolution.height ),
        Z_NEAR,
        Z_FAR
    );

    VkExtent2D const& viewport = renderer.GetViewportResolution ();

    VkExtent2D const resolution
    {
        .width = ( viewport.width * RESOLUTION_SCALE_WIDTH / 100U ),
        .height = ( viewport.height * RESOLUTION_SCALE_HEIGHT / 100U )
    };

    return _renderSession.OnSwapchainCreated ( renderer, resolution, _commandPool );
}

void StippleTest::OnSwapchainDestroyed ( VkDevice device ) noexcept
{
    _renderSession.OnSwapchainDestroyed ( device );
    _camera.ReleaseInput ();
}

void StippleTest::Animate ( float deltaTime ) noexcept
{
    constexpr GXVec3 scale ( 0.5F, 2.0F, 0.3F );

    constexpr GXVec3 renderScale ( scale._data[ 0U ] * RENDERER_SCALE,
        scale._data[ 1U ] * RENDERER_SCALE,
        scale._data[ 2U ] * RENDERER_SCALE
    );

    constexpr GXVec3 location ( 0.0F, 2.5F, 0.0F );

    constexpr GXVec3 renderLocation ( location._data[ 0U ] * RENDERER_SCALE,
        location._data[ 1U ] * RENDERER_SCALE,
        location._data[ 2U ] * RENDERER_SCALE
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

    GXVec3& x = *reinterpret_cast<GXVec3*> ( &transform._m[ 0U ][ 0U ] );
    x.Multiply ( x, renderScale._data[ 0U ] );

    GXVec3& y = *reinterpret_cast<GXVec3*> ( &transform._m[ 1U ][ 0U ] );
    y.Multiply ( y, renderScale._data[ 1U ] );

    GXVec3& z = *reinterpret_cast<GXVec3*> ( &transform._m[ 2U ][ 0U ] );
    z.Multiply ( z, renderScale._data[ 2U ] );

    _stipple->SetTransform ( transform );
}

bool StippleTest::CreateScene ( android_vulkan::Renderer &renderer ) noexcept
{
    constexpr uint32_t meshCommandBuffers = 2U;
    constexpr uint32_t materialCommandBuffers = 2U * MaterialManager::MaxCommandBufferPerMaterial ();
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

    if ( !result )
        return false;

    size_t consumed;
    bool success;

    _floor = std::make_unique<StaticMeshComponent> ( renderer,
        success,
        consumed,
        "pbr/system/unit-cube.mesh2",
        "pbr/assets/System/DefaultCSG.mtl",
        cb
    );

    if ( !success )
        return false;

    constexpr GXVec3 floorLocation ( 0.0F, -0.25F, 0.0F );

    constexpr GXVec3 floorRenderLocation ( floorLocation._data[ 0U ] * RENDERER_SCALE,
        floorLocation._data[ 1U ] * RENDERER_SCALE,
        floorLocation._data[ 2U ] * RENDERER_SCALE
    );

    constexpr GXVec3 floorScale ( 10.0F, 0.5F, 30.0F );

    constexpr GXVec3 floorRenderScale ( floorScale._data[ 0U ] * RENDERER_SCALE,
        floorScale._data[ 1U ] * RENDERER_SCALE,
        floorScale._data[ 2U ] * RENDERER_SCALE
    );

    GXMat4 transform {};
    transform.Translation ( floorRenderLocation );
    transform._m[ 0U ][ 0U ] = floorRenderScale._data[ 0U ];
    transform._m[ 1U ][ 1U ] = floorRenderScale._data[ 1U ];
    transform._m[ 2U ][ 2U ] = floorRenderScale._data[ 2U ];
    _floor->SetTransform ( transform );

    cb += consumed;

    MaterialRef stippleMaterial = std::make_shared<StippleMaterial> ();

    // NOLINTNEXTLINE
    auto& sm = static_cast<StippleMaterial&> ( *stippleMaterial );

    Texture2DRef t = std::make_shared<android_vulkan::Texture2D> ();

    result = t->UploadData ( renderer,
        "pbr/assets/Textures/CSG/Tile/PTile16x16_D.ktx",
        android_vulkan::eFormat::sRGB,
        true,
        *cb
    );

    if ( !result )
        return false;

    ++cb;
    sm.SetAlbedo ( t );
    t = std::make_shared<android_vulkan::Texture2D> ();

    result = t->UploadData ( renderer,
        "pbr/assets/Textures/CSG/Tile/PTile16x16_N.ktx",
        android_vulkan::eFormat::Unorm,
        true,
        *cb
    );

    if ( !result )
        return false;

    ++cb;
    sm.SetNormal ( t );
    t = std::make_shared<android_vulkan::Texture2D> ();

    result = t->UploadData ( renderer,
        "pbr/assets/Textures/CSG/Tile/PTile16x16_S.ktx",
        android_vulkan::eFormat::Unorm,
        true,
        *cb
    );

    if ( !result )
        return false;

    ++cb;
    sm.SetParam ( t );

    _stipple = std::make_unique<StaticMeshComponent> ( renderer,
        success,
        consumed,
        "pbr/system/unit-cube.mesh2",
        stippleMaterial,
        cb
    );

    if ( !success )
        return false;

    result = android_vulkan::Renderer::CheckVkResult ( vkQueueWaitIdle ( renderer.GetQueue () ),
        "StippleTest::CreateScene",
        "Can't run upload commands"
    );

    if ( !result )
        return false;

    _floor->FreeTransferResources ( device );
    _stipple->FreeTransferResources ( device );

    constexpr GXVec3 lightBounds ( 100.0F, 100.0F, 100.0F );

    constexpr GXVec3 lightRenderBounds ( lightBounds._data[ 0U ] * RENDERER_SCALE,
        lightBounds._data[ 1U ] * RENDERER_SCALE,
        lightBounds._data[ 2U ] * RENDERER_SCALE
    );

    _light.SetBoundDimensions ( lightRenderBounds );
    _light.SetIntensity ( 500.0F );

    constexpr GXVec3 lightLocation ( 0.0F, 15.0F, -1.5F );

    constexpr GXVec3 lightRenderLocation ( lightLocation._data[ 0U ] * RENDERER_SCALE,
        lightLocation._data[ 1U ] * RENDERER_SCALE,
        lightLocation._data[ 2U ] * RENDERER_SCALE
    );

    _light.SetLocation ( lightRenderLocation );

    constexpr GXVec3 cameraLocation ( 15.49F, 133.74F, -29.841F );

    _camera.SetLocation ( cameraLocation );
    _camera.SetRotation ( 0.9661F, -0.3127F );
    _camera.Update ( 0.0F );

    return true;
}

} // namespace pbr::stipple_test
