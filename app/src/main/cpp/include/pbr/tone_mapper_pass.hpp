#ifndef PBR_TONE_MAPPER_PASS_HPP
#define PBR_TONE_MAPPER_PASS_HPP


#include "full_screen_triangle_descriptor_set_layout.hpp"
#include "tone_mapper_program.hpp"
#include "uniform_pool.hpp"


namespace pbr {

class ToneMapperPass final
{
    private:
        float                                       _brightnessBalance = 0.0F;
        VkDescriptorSet                             _descriptorSets[ 2U ] = { VK_NULL_HANDLE, VK_NULL_HANDLE };
        ToneMapperProgram                           _program {};
        ToneMapperDescriptorSetLayout               _resourceLayout {};
        FullScreenTriangleDescriptorSetLayout       _transformLayout {};
        VkDescriptorPool                            _resourceDescriptorPool = VK_NULL_HANDLE;

        UniformPool                                 _transformUniformPool
        {
            eUniformSize::Nanoscopic_64KB,
            VK_PIPELINE_STAGE_VERTEX_SHADER_BIT
        };

        bool                                        _transformUpdated = true;

    public:
        ToneMapperPass () = default;

        ToneMapperPass ( ToneMapperPass const & ) = delete;
        ToneMapperPass &operator = ( ToneMapperPass const & ) = delete;

        ToneMapperPass ( ToneMapperPass && ) = delete;
        ToneMapperPass &operator = ( ToneMapperPass && ) = delete;

        ~ToneMapperPass () = default;

        [[nodiscard]] bool Init ( android_vulkan::Renderer &renderer ) noexcept;
        void Destroy ( android_vulkan::Renderer &renderer ) noexcept;

        void Execute ( VkCommandBuffer commandBuffer ) noexcept;

        [[nodiscard]] bool SetBrightness ( android_vulkan::Renderer &renderer,
            VkRenderPass renderPass,
            uint32_t subpass,
            float brightnessBalance
        ) noexcept;

        [[nodiscard]] bool SetTarget ( android_vulkan::Renderer &renderer,
            VkRenderPass renderPass,
            uint32_t subpass,
            VkImageView hdrView,
            VkBuffer exposure,
            VkSampler clampToEdgeSampler
        ) noexcept;

        void UploadGPUData ( android_vulkan::Renderer &renderer, VkCommandBuffer commandBuffer ) noexcept;
};

} // namespace pbr


#endif // PBR_TONE_MAPPER_PASS_HPP
