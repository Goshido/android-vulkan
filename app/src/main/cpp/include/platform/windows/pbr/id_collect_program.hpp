#ifndef PBR_ID_COLLECT_PROGRAM_HPP
#define PBR_ID_COLLECT_PROGRAM_HPP


#include "compute_program.hpp"
#include <vulkan_utils.hpp>


namespace pbr {

class IDCollectProgram final : public ComputeProgram
{
    public:
        AV_DX_ALIGNMENT_BEGIN

        struct PushConstants final
        {
            [[maybe_unused]] uint32_t       _idImage;
            [[maybe_unused]] uint32_t       _idSet;
            [[maybe_unused]] uint32_t       _capacity;
        };

        AV_DX_ALIGNMENT_END

    public:
        explicit IDCollectProgram () noexcept;

        IDCollectProgram ( IDCollectProgram const & ) = delete;
        IDCollectProgram &operator = ( IDCollectProgram const & ) = delete;

        IDCollectProgram ( IDCollectProgram && ) = delete;
        IDCollectProgram &operator = ( IDCollectProgram && ) = delete;

        ~IDCollectProgram () override = default;

        [[nodiscard]] bool Init ( VkDevice device, SpecializationData specializationData ) noexcept override;
        void Destroy ( VkDevice device ) noexcept override;

        // FUCK - use actual selection rectangle
        [[nodiscard]] static VkExtent3D DispatchParams ( VkExtent2D const &resolution ) noexcept;

    private:
        [[nodiscard]] VkPipelineShaderStageCreateInfo InitShaderInfo ( std::vector<uint8_t> &cs,
            VkShaderModuleCreateInfo &moduleInfo,
            SpecializationData specializationData,
            VkSpecializationInfo* specializationInfo
        ) noexcept override;
};

} // namespace pbr


#endif // PBR_ID_COLLECT_PROGRAM_HPP
