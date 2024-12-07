#ifndef PBR_DUMMY_LIGHT_PROGRAM_HPP
#define PBR_DUMMY_LIGHT_PROGRAM_HPP


#include "dummy_program.hpp"
#include "lightup_common_descriptor_set_layout.hpp"


namespace pbr {

class DummyLightProgram final : public DummyProgram
{
    private:
        LightupCommonDescriptorSetLayout    _layout {};

    public:
        explicit DummyLightProgram () noexcept;

        DummyLightProgram ( DummyLightProgram const & ) = delete;
        DummyLightProgram &operator = ( DummyLightProgram const & ) = delete;

        DummyLightProgram ( DummyLightProgram && ) = delete;
        DummyLightProgram &operator = ( DummyLightProgram && ) = delete;

        ~DummyLightProgram () override = default;

        void Destroy ( VkDevice device ) noexcept override;

    private:
        [[nodiscard]] VkPipelineColorBlendStateCreateInfo const* InitColorBlendInfo (
            VkPipelineColorBlendStateCreateInfo &info,
            VkPipelineColorBlendAttachmentState* attachments
        ) const noexcept override;

        [[nodiscard]] VkPipelineDepthStencilStateCreateInfo const* InitDepthStencilInfo (
            VkPipelineDepthStencilStateCreateInfo &info
        ) const noexcept override;

        [[nodiscard]] bool InitLayout ( VkDevice device, VkPipelineLayout &layout ) noexcept override;

        [[nodiscard]] VkPipelineRasterizationStateCreateInfo const* InitRasterizationInfo (
            VkPipelineRasterizationStateCreateInfo &info
        ) const noexcept override;
};

} // namespace pbr


#endif // PBR_DUMMY_LIGHT_PROGRAM_HPP
