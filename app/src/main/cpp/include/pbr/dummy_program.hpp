#ifndef PBR_DUMMY_PROGRAM_HPP
#define PBR_DUMMY_PROGRAM_HPP


#include "graphics_program.hpp"


namespace pbr {

class DummyProgram : public GraphicsProgram
{
    private:
        uint32_t const              _colorRenderTargets = 0U;
        std::string_view const      _fragmentShaderSource;
        uint32_t const              _subpass = 0U;

    public:
        DummyProgram () = delete;

        DummyProgram ( DummyProgram const & ) = delete;
        DummyProgram &operator = ( DummyProgram const & ) = delete;

        DummyProgram ( DummyProgram && ) = delete;
        DummyProgram &operator = ( DummyProgram && ) = delete;

        ~DummyProgram () override = default;

        [[nodiscard]] DescriptorSetInfo const &GetResourceInfo () const noexcept override;

        [[nodiscard]] bool Init ( android_vulkan::Renderer &renderer,
            VkRenderPass renderPass,
            VkExtent2D const &viewport
        ) noexcept;

    protected:
        explicit DummyProgram ( std::string_view &&name,
            std::string_view &&fragmentShader,
            uint32_t colorRenderTargets,
            uint32_t subpass
        ) noexcept;

        [[nodiscard]] bool InitLayoutInternal ( VkDevice device,
            VkPipelineLayout &layout,
            VkDescriptorSetLayout descriptorSetLayout
        ) noexcept;

    private:
        [[nodiscard]] VkPipelineDynamicStateCreateInfo const* InitDynamicStateInfo (
            VkPipelineDynamicStateCreateInfo* info
        ) const noexcept override;

        [[nodiscard]] VkPipelineInputAssemblyStateCreateInfo const* InitInputAssemblyInfo (
            VkPipelineInputAssemblyStateCreateInfo &info
        ) const noexcept override;

        [[nodiscard]] VkPipelineMultisampleStateCreateInfo const* InitMultisampleInfo (
            VkPipelineMultisampleStateCreateInfo &info
        ) const noexcept override;

        [[nodiscard]] bool InitShaderInfo ( android_vulkan::Renderer &renderer,
            VkPipelineShaderStageCreateInfo const* &targetInfo,
            SpecializationData specializationData,
            VkSpecializationInfo* specializationInfo,
            VkPipelineShaderStageCreateInfo* sourceInfo
        ) noexcept override;

        [[nodiscard]] VkPipelineViewportStateCreateInfo const* InitViewportInfo (
            VkPipelineViewportStateCreateInfo &info,
            VkRect2D* scissorInfo,
            VkViewport* viewportInfo,
            VkExtent2D const* viewport
        ) const noexcept override;

        [[nodiscard]] VkPipelineVertexInputStateCreateInfo const* InitVertexInputInfo (
            VkPipelineVertexInputStateCreateInfo &info,
            VkVertexInputAttributeDescription* attributes,
            VkVertexInputBindingDescription* binds
        ) const noexcept override;
};

} // namespace pbr


#endif // PBR_DUMMY_PROGRAM_HPP
