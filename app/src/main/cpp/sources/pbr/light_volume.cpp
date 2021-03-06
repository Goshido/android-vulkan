#include <pbr/light_volume.h>
#include <pbr/light_lightup_base_program.h>


namespace pbr {

LightVolume::LightVolume () noexcept:
    _program {}
{
    // NOTHING
}

void LightVolume::Execute ( uint32_t vertexCount, VkDescriptorSet transform, VkCommandBuffer commandBuffer )
{
    _program.Bind ( commandBuffer );
    _program.SetTransform ( commandBuffer, transform );
    vkCmdDrawIndexed ( commandBuffer, vertexCount, 1U, 0U, 0, 0U );
}

bool LightVolume::Init ( android_vulkan::Renderer &renderer,
    GBuffer &gBuffer,
    VkRenderPass renderPass
)
{
    VkDevice device = renderer.GetDevice ();

    if ( _program.Init ( renderer, renderPass, 0U, gBuffer.GetResolution () ) )
        return true;

    Destroy ( device );
    return false;
}

void LightVolume::Destroy ( VkDevice device )
{
    _program.Destroy ( device );
}

} // namespace pbr
