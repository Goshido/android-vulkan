#ifndef PBR_ID_COLLECT_PROGRAM_HPP
#define PBR_ID_COLLECT_PROGRAM_HPP


#include "compute_program.hpp"
#include "resource_heap_descriptor_set_layout.hpp"
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

    private:
        ResourceHeapDescriptorSetLayout     _layout {};

    public:
        explicit IDCollectProgram () noexcept;

        IDCollectProgram ( IDCollectProgram const & ) = delete;
        IDCollectProgram &operator = ( IDCollectProgram const & ) = delete;

        IDCollectProgram ( IDCollectProgram && ) = delete;
        IDCollectProgram &operator = ( IDCollectProgram && ) = delete;

        ~IDCollectProgram () override = default;

        [[nodiscard]] bool Init ( VkDevice device, SpecializationData specializationData ) noexcept override;

        void Destroy ( VkDevice device ) noexcept override;

    private:
        [[nodiscard]] bool InitLayout ( VkDevice device, VkPipelineLayout &layout ) noexcept override;

        [[nodiscard]] bool InitShaderInfo ( std::vector<uint8_t> &cs,
            VkShaderModuleCreateInfo &moduleInfo,
            SpecializationData specializationData,
            VkSpecializationInfo* specializationInfo,
            VkPipelineShaderStageCreateInfo &targetInfo
        ) noexcept override;
};

} // namespace pbr


#endif // PBR_ID_COLLECT_PROGRAM_HPP
