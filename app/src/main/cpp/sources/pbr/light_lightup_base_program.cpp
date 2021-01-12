#include <pbr/light_lightup_base_program.h>


namespace pbr {

[[maybe_unused]] void LightLightupBaseProgram::SetViewData ( VkCommandBuffer commandBuffer, VkDescriptorSet set )
{
    vkCmdBindDescriptorSets ( commandBuffer,
        VK_PIPELINE_BIND_POINT_GRAPHICS,
        _pipelineLayout,
        0U,
        1U,
        &set,
        0U,
        nullptr
    );
}

LightLightupBaseProgram::LightLightupBaseProgram ( std::string &&name ) noexcept:
    Program ( std::move ( name ) )
{
    // NOTHING
}

} // namespace pbr
