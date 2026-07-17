#ifndef PBR_OUTLINE_BORDER_PROGRAM_HPP
#define PBR_OUTLINE_BORDER_PROGRAM_HPP


#include <GXCommon/GXMath.hpp>
#include "compute_program.hpp"
#include <vulkan_utils.hpp>


namespace pbr {

class OutlineBorderProgram final : public ComputeProgram
{
    public:
        AV_DX_ALIGNMENT_BEGIN

        struct PushConstants final
        {
            [[maybe_unused]] uint32_t       _idMask;
            [[maybe_unused]] uint32_t       _outline;
            [[maybe_unused]] VkExtent2D     _resolution;
            [[maybe_unused]] GXVec2         _invResolution;
            [[maybe_unused]] GXVec4         _halfPixelMove;
        };

        AV_DX_ALIGNMENT_END

    public:
        explicit OutlineBorderProgram () noexcept;

        OutlineBorderProgram ( OutlineBorderProgram const & ) = delete;
        OutlineBorderProgram &operator = ( OutlineBorderProgram const & ) = delete;

        OutlineBorderProgram ( OutlineBorderProgram && ) = delete;
        OutlineBorderProgram &operator = ( OutlineBorderProgram && ) = delete;

        ~OutlineBorderProgram () override = default;

        [[nodiscard]] bool Init ( VkDevice device, SpecializationData specializationData ) noexcept override;
        void Destroy ( VkDevice device ) noexcept override;

        [[nodiscard]] static VkExtent3D DispatchParams ( VkExtent2D const &resolution ) noexcept;

    private:
        [[nodiscard]] VkPipelineShaderStageCreateInfo InitShaderInfo ( std::vector<uint8_t> &cs,
            VkShaderModuleCreateInfo &moduleInfo,
            SpecializationData specializationData,
            VkSpecializationInfo* specializationInfo
        ) noexcept override;
};

} // namespace pbr


#endif // PBR_OUTLINE_BORDER_PROGRAM_HPP
