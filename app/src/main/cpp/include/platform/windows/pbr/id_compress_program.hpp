#ifndef PBR_ID_COMPRESS_PROGRAM_HPP
#define PBR_ID_COMPRESS_PROGRAM_HPP


#include "compute_program.hpp"
#include <vulkan_utils.hpp>


namespace pbr {

class IDCompressProgram final : public ComputeProgram
{
    public:
        AV_DX_ALIGNMENT_BEGIN

        struct PushConstants final
        {
            [[maybe_unused]] uint32_t       _idSet;
            [[maybe_unused]] uint32_t       _uniqueIDs;
            [[maybe_unused]] uint32_t       _capacity;
        };

        AV_DX_ALIGNMENT_END

    public:
        explicit IDCompressProgram () noexcept;

        IDCompressProgram ( IDCompressProgram const & ) = delete;
        IDCompressProgram &operator = ( IDCompressProgram const & ) = delete;

        IDCompressProgram ( IDCompressProgram && ) = delete;
        IDCompressProgram &operator = ( IDCompressProgram && ) = delete;

        ~IDCompressProgram () override = default;

        [[nodiscard]] bool Init ( VkDevice device, SpecializationData specializationData ) noexcept override;
        void Destroy ( VkDevice device ) noexcept override;

        [[nodiscard]] static VkExtent3D DispatchParams ( uint32_t capacity ) noexcept;

    private:
        [[nodiscard]] VkPipelineShaderStageCreateInfo InitShaderInfo ( std::vector<uint8_t> &cs,
            VkShaderModuleCreateInfo &moduleInfo,
            SpecializationData specializationData,
            VkSpecializationInfo* specializationInfo
        ) noexcept override;
};

} // namespace pbr


#endif // PBR_ID_COMPRESS_PROGRAM_HPP
