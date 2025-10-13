#include <precompiled_headers.hpp>
#include <pbr/brightness_factor.inc>
#include <platform/windows/pbr/tone_mapper_pass.hpp>
#include <vulkan_utils.hpp>


namespace pbr {

void ToneMapperPass::Destroy ( VkDevice device ) noexcept
{
    _program.Destroy ( device );
}

void ToneMapperPass::Execute ( VkCommandBuffer commandBuffer, ResourceHeap &resourceHeap ) noexcept
{
    AV_VULKAN_GROUP ( commandBuffer, "Tone mapping" )
    _program.Bind ( commandBuffer );
    resourceHeap.Bind ( commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, _program.GetPipelineLayout () );
    _program.SetPushConstants ( commandBuffer, &_pushConstants );
    vkCmdDraw ( commandBuffer, 3U, 1U, 0U, 0U );
}

bool ToneMapperPass::SetBrightness ( android_vulkan::Renderer const &renderer, float brightnessBalance ) noexcept
{
    _brightnessInfo = BrightnessInfo ( brightnessBalance );

    VkDevice device = renderer.GetDevice ();
    _program.Destroy ( device );

    bool const result = _program.Init ( device,
        renderer.GetSurfaceFormat (),
        _brightnessInfo,
        renderer.GetSurfaceSize ()
    );

    if ( !result ) [[unlikely]]
        return false;

    UpdateTransform ( renderer );
    return true;
}

void ToneMapperPass::SetTarget ( android_vulkan::Renderer const &renderer,
    uint32_t hdrImage,
    uint32_t exposure
) noexcept
{
    _pushConstants._exposure = exposure;
    _pushConstants._hdrImage = hdrImage;
    UpdateTransform ( renderer );
}

void ToneMapperPass::UpdateTransform ( android_vulkan::Renderer const &renderer ) noexcept
{
    GXMat4 const &orientation = renderer.GetPresentationEngineTransform ();
    _pushConstants._transformRow0 = *reinterpret_cast<GXVec2 const*> ( orientation._data[ 0U ] );
    _pushConstants._transformRow1 = *reinterpret_cast<GXVec2 const*> ( orientation._data[ 1U ] );
}

} // namespace pbr
