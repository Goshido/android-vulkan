#ifndef PBR_SPD_MIP12_PROGRAM_HPP
#define PBR_SPD_MIP12_PROGRAM_HPP


#include "compute_program.hpp"
#include "spd_12_mips_descriptor_set_layout.hpp"


namespace pbr {

class SPD12MipsProgram final : public ComputeProgram
{
    public:
        struct SpecializationInfo final
        {
            [[maybe_unused]] uint32_t           _workgroupCount;
        };

    private:
        SPD12MipsDescriptorSetLayout            _layout {};

    public:
        explicit SPD12MipsProgram () noexcept;

        SPD12MipsProgram ( SPD12MipsProgram const & ) = delete;
        SPD12MipsProgram &operator = ( SPD12MipsProgram const & ) = delete;

        SPD12MipsProgram ( SPD12MipsProgram && ) = delete;
        SPD12MipsProgram &operator = ( SPD12MipsProgram && ) = delete;

        ~SPD12MipsProgram () override = default;

        [[nodiscard]] bool Init ( android_vulkan::Renderer &renderer,
            SpecializationData specializationData
        ) noexcept override;

        void Destroy ( VkDevice device ) noexcept override;

        void SetDescriptorSet ( VkCommandBuffer commandBuffer, VkDescriptorSet set ) const noexcept;

        static void GetMetaInfo ( VkExtent3D &dispatch,
            SpecializationInfo &specializationInfo,
            VkExtent2D const &imageResolution
        ) noexcept;

    private:
        void DestroyShaderModule ( VkDevice device ) noexcept override;
        [[nodiscard]] bool InitLayout ( VkDevice device, VkPipelineLayout &layout ) noexcept override;

        [[nodiscard]] bool InitShaderInfo ( android_vulkan::Renderer &renderer,
            SpecializationData specializationData,
            VkSpecializationInfo* specializationInfo,
            VkPipelineShaderStageCreateInfo &targetInfo
        ) noexcept override;
};

} // namespace pbr


#endif // PBR_SPD_MIP12_PROGRAM_HPP
