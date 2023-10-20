#ifndef PBR_SKIN_PROGRAM_HPP
#define PBR_SKIN_PROGRAM_HPP


#include "compute_program.hpp"
#include "skin_descriptor_set_layout.hpp"
#include <vulkan_utils.hpp>


namespace pbr {

class SkinProgram : public ComputeProgram
{
    public:
        AV_DX_ALIGNMENT_BEGIN

        struct PushConstants final
        {
            [[maybe_unused]] uint32_t       _vertexCount;
        };

        AV_DX_ALIGNMENT_END

    private:
        SkinDescriptorSetLayout             _layout {};

    public:
        explicit SkinProgram () noexcept;

        SkinProgram ( SkinProgram const & ) = delete;
        SkinProgram &operator = ( SkinProgram const & ) = delete;

        SkinProgram ( SkinProgram && ) = delete;
        SkinProgram &operator = ( SkinProgram && ) = delete;

        ~SkinProgram () override = default;

        [[nodiscard]] bool Init ( android_vulkan::Renderer &renderer,
            SpecializationData specializationData
        ) noexcept override final;

        void Destroy ( VkDevice device ) noexcept override final;

        void SetDescriptorSet ( VkCommandBuffer commandBuffer, VkDescriptorSet set ) const noexcept;

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


#endif // PBR_SKIN_PROGRAM_HPP
