// FUCK - windows and android separation

#ifndef PBR_ANDROID_EXPOSURE_PROGRAM_HPP
#define PBR_ANDROID_EXPOSURE_PROGRAM_HPP


#include "compute_program.hpp"
#include "exposure_descriptor_set_layout.hpp"
#include <vulkan_utils.hpp>


// FUCK - remove namespace
namespace pbr::android {

class ExposureProgram final : public ComputeProgram
{
    public:
        AV_DX_ALIGNMENT_BEGIN

        struct PushConstants final
        {
            [[maybe_unused]] float          _exposureCompensation;
            [[maybe_unused]] float          _eyeAdaptation;
            [[maybe_unused]] float          _maxLuma;
            [[maybe_unused]] float          _minLuma;
        };

        AV_DX_ALIGNMENT_END

    private:
        ExposureDescriptorSetLayout         _layout {};

    public:
        explicit ExposureProgram () noexcept;

        ExposureProgram ( ExposureProgram const & ) = delete;
        ExposureProgram &operator = ( ExposureProgram const & ) = delete;

        ExposureProgram ( ExposureProgram && ) = delete;
        ExposureProgram &operator = ( ExposureProgram && ) = delete;

        ~ExposureProgram () override = default;

        [[nodiscard]] bool Init ( android_vulkan::Renderer const &renderer,
            SpecializationData specializationData
        ) noexcept override;

        void Destroy ( VkDevice device ) noexcept override;

        void SetDescriptorSet ( VkCommandBuffer commandBuffer, VkDescriptorSet set ) const noexcept;

    private:
        [[nodiscard]] bool InitLayout ( VkDevice device, VkPipelineLayout &layout ) noexcept override;

        [[nodiscard]] bool InitShaderInfo ( android_vulkan::Renderer const &renderer,
            SpecializationData specializationData,
            VkSpecializationInfo* specializationInfo,
            VkPipelineShaderStageCreateInfo &targetInfo
        ) noexcept override;
};

} // namespace pbr::android


#endif // PBR_ANDROID_EXPOSURE_PROGRAM_HPP
