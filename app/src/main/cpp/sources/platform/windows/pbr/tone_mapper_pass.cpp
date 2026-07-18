#include <precompiled_headers.hpp>
#include <platform/windows/pbr/tone_mapper_pass.hpp>
#include <vulkan_utils.hpp>


namespace pbr {

void ToneMapperPass::Destroy ( VkDevice device ) noexcept
{
    _program.Destroy ( device );
}

void ToneMapperPass::Execute ( VkCommandBuffer commandBuffer, std::optional<uint32_t> &&outlineBlurX ) noexcept
{
    AV_VULKAN_GROUP ( commandBuffer, "Tone mapping" )
    _program.Bind ( commandBuffer );
    _pushConstants._outlineBlurX = outlineBlurX ? *outlineBlurX : 0U;
    _program.SetPushConstants ( commandBuffer, &_pushConstants );
    vkCmdDraw ( commandBuffer, 3U, 1U, 0U, 0U );
}

bool ToneMapperPass::SetBrightness ( android_vulkan::Renderer const &renderer, float brightnessBalance ) noexcept
{
    _brightnessInfo = BrightnessInfo ( brightnessBalance );

    if ( !RecreateProgram ( renderer ) ) [[unlikely]]
        return false;

    UpdateTransform ( renderer );
    return true;
}

bool ToneMapperPass::SetTarget ( android_vulkan::Renderer const &renderer,
    uint32_t hdrImage,
    uint32_t exposure
) noexcept
{
    if ( !RecreateProgram ( renderer ) ) [[unlikely]]
        return false;

    _pushConstants._exposure = exposure;
    _pushConstants._hdrImage = hdrImage;
    _pushConstants._outlineBlurX = 0U;
    UpdateTransform ( renderer );
    return true;
}

bool ToneMapperPass::RecreateProgram ( android_vulkan::Renderer const &renderer ) noexcept
{
    VkDevice device = renderer.GetDevice ();
    _program.Destroy ( device );

    VkExtent2D const &r = renderer.GetSurfaceSize ();
    GXVec2 &resolution = _pushConstants._resolution;
    resolution = GXVec2 ( static_cast<float> ( r.width ), static_cast<float> ( r.height ) );

    GXVec2 &halfPixelMove = _pushConstants._halfPixelMove;
    halfPixelMove = GXVec2 ( 1.0F / resolution._data[ 0U ], 1.0F / resolution._data[ 1U ] );
    halfPixelMove.Multiply ( halfPixelMove, 0.5F );

    return _program.Init ( device, renderer.GetSurfaceFormat (), _brightnessInfo, r );
}

void ToneMapperPass::UpdateTransform ( android_vulkan::Renderer const &renderer ) noexcept
{
    GXMat4 const &orientation = renderer.GetPresentationEngineTransform ();
    _pushConstants._transformRow0 = *reinterpret_cast<GXVec2 const*> ( orientation._data[ 0U ] );
    _pushConstants._transformRow1 = *reinterpret_cast<GXVec2 const*> ( orientation._data[ 1U ] );
}

} // namespace pbr
