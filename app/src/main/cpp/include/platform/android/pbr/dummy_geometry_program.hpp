#ifndef PBR_DUMMY_GEOMETRY_PROGRAM_HPP
#define PBR_DUMMY_GEOMETRY_PROGRAM_HPP


#include "dummy_program.hpp"
#include "stub_descriptor_set_layout.hpp"


namespace pbr {

class DummyGeometryProgram final : public DummyProgram
{
    private:
        StubDescriptorSetLayout     _layout {};

    public:
        explicit DummyGeometryProgram () noexcept;

        DummyGeometryProgram ( DummyGeometryProgram const & ) = delete;
        DummyGeometryProgram &operator = ( DummyGeometryProgram const & ) = delete;

        DummyGeometryProgram ( DummyGeometryProgram && ) = delete;
        DummyGeometryProgram &operator = ( DummyGeometryProgram && ) = delete;

        ~DummyGeometryProgram () override = default;

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


#endif // PBR_DUMMY_GEOMETRY_PROGRAM_HPP
