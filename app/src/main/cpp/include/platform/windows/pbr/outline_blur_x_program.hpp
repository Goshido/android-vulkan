#ifndef PBR_OUTLINE_BLUR_X_PROGRAM_HPP
#define PBR_OUTLINE_BLUR_X_PROGRAM_HPP


#include <GXCommon/GXMath.hpp>
#include "compute_program.hpp"
#include <vulkan_utils.hpp>


namespace pbr {

class OutlineBlurXProgram final : public ComputeProgram
{
    public:
        AV_DX_ALIGNMENT_BEGIN

        struct PushConstants final
        {
            [[maybe_unused]] uint32_t       _border;
            [[maybe_unused]] uint32_t       _blurX;
            [[maybe_unused]] VkExtent2D     _resolution;
            [[maybe_unused]] GXVec2         _invResolution;
            [[maybe_unused]] GXVec2         _halfPixelMove;
        };

        AV_DX_ALIGNMENT_END

    public:
        explicit OutlineBlurXProgram () noexcept;

        OutlineBlurXProgram ( OutlineBlurXProgram const & ) = delete;
        OutlineBlurXProgram &operator = ( OutlineBlurXProgram const & ) = delete;

        OutlineBlurXProgram ( OutlineBlurXProgram && ) = delete;
        OutlineBlurXProgram &operator = ( OutlineBlurXProgram && ) = delete;

        ~OutlineBlurXProgram () override = default;

        [[nodiscard]] bool Init ( VkDevice device, SpecializationData specializationData ) noexcept override;
        void Destroy ( VkDevice device ) noexcept override;

    private:
        [[nodiscard]] VkPipelineShaderStageCreateInfo InitShaderInfo ( std::vector<uint8_t> &cs,
            VkShaderModuleCreateInfo &moduleInfo,
            SpecializationData specializationData,
            VkSpecializationInfo* specializationInfo
        ) noexcept override;
};

} // namespace pbr


#endif // PBR_OUTLINE_BLUR_X_PROGRAM_HPP
