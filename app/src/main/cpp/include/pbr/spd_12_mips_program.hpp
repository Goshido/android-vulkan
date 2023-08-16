#ifndef PBR_SPD_MIP12_PROGRAM_HPP
#define PBR_SPD_MIP12_PROGRAM_HPP


#include "average_brightness_descriptor_set_layout.hpp"
#include "compute_program.hpp"


namespace pbr {

class [[maybe_unused]] SPD12MipsProgram final : public ComputeProgram
{
    private:
        AverageBrightnessDescriptorSetLayout    _layout {};

    public:
        explicit SPD12MipsProgram () noexcept;

        SPD12MipsProgram ( SPD12MipsProgram const & ) = delete;
        SPD12MipsProgram &operator = ( SPD12MipsProgram const & ) = delete;

        SPD12MipsProgram ( SPD12MipsProgram && ) = delete;
        SPD12MipsProgram &operator = ( SPD12MipsProgram && ) = delete;

        ~SPD12MipsProgram () override = default;

        [[nodiscard]] bool Init ( android_vulkan::Renderer &renderer ) noexcept override;
        void Destroy ( VkDevice device ) noexcept override;

        void SetDescriptorSet ( VkCommandBuffer commandBuffer, VkDescriptorSet set ) const noexcept;

    private:
        [[nodiscard]] bool InitLayout ( VkDevice device, VkPipelineLayout &layout ) noexcept override;

        [[nodiscard]] bool InitShaderInfo ( android_vulkan::Renderer &renderer,
            VkPipelineShaderStageCreateInfo &targetInfo
        ) noexcept override;
};

} // namespace pbr


#endif // PBR_SPD_MIP12_PROGRAM_HPP
