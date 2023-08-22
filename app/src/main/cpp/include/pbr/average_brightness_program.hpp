#ifndef PBR_AVERAGE_BRIGHTNESS_PROGRAM_HPP
#define PBR_AVERAGE_BRIGHTNESS_PROGRAM_HPP


#include "average_brightness_descriptor_set_layout.hpp"
#include "compute_program.hpp"


namespace pbr {

class AverageBrightnessProgram : public ComputeProgram
{
    public:
        struct SpecializationInfo final
        {
            [[maybe_unused]] uint32_t           _workgroupCount;
            [[maybe_unused]] VkExtent2D         _mip5Resolution;
            [[maybe_unused]] float              _normalizeW;
            [[maybe_unused]] float              _normalizeH;
        };

    private:
        AverageBrightnessDescriptorSetLayout    _layout {};

    public:
        explicit AverageBrightnessProgram () noexcept;

        AverageBrightnessProgram ( AverageBrightnessProgram const & ) = delete;
        AverageBrightnessProgram &operator = ( AverageBrightnessProgram const & ) = delete;

        AverageBrightnessProgram ( AverageBrightnessProgram && ) = delete;
        AverageBrightnessProgram &operator = ( AverageBrightnessProgram && ) = delete;

        ~AverageBrightnessProgram () override = default;

        [[nodiscard]] bool Init ( android_vulkan::Renderer &renderer,
            SpecializationData specializationData
        ) noexcept override final;

        void Destroy ( VkDevice device ) noexcept override final;

        void SetDescriptorSet ( VkCommandBuffer commandBuffer, VkDescriptorSet set ) const noexcept;

        static void GetMetaInfo ( VkExtent3D &dispatch,
            VkExtent2D &mipChainResolution,
            SpecializationInfo &specializationInfo,
            VkExtent2D const &imageResolution
        ) noexcept;

    private:
        void DestroyShaderModule ( VkDevice device ) noexcept override final;
        [[nodiscard]] bool InitLayout ( VkDevice device, VkPipelineLayout &layout ) noexcept override final;

        [[nodiscard]] bool InitShaderInfo ( android_vulkan::Renderer &renderer,
            SpecializationData specializationData,
            VkSpecializationInfo* specializationInfo,
            VkPipelineShaderStageCreateInfo &targetInfo
        ) noexcept override final;
};

} // namespace pbr


#endif // PBR_AVERAGE_BRIGHTNESS_PROGRAM_HPP
