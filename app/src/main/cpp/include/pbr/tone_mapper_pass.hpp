#ifndef PBR_TONE_MAPPER_PASS_HPP
#define PBR_TONE_MAPPER_PASS_HPP


#include "tone_mapper_program.hpp"


namespace pbr {

class ToneMapperPass final
{
    private:
        VkDescriptorPool                    _descriptorPool = VK_NULL_HANDLE;
        VkDescriptorSet                     _descriptorSet = VK_NULL_HANDLE;
        ToneMapperDescriptorSetLayout       _layout {};
        ToneMapperProgram                   _program {};

    public:
        ToneMapperPass () = default;

        ToneMapperPass ( ToneMapperPass const & ) = delete;
        ToneMapperPass &operator = ( ToneMapperPass const & ) = delete;

        ToneMapperPass ( ToneMapperPass && ) = delete;
        ToneMapperPass &operator = ( ToneMapperPass && ) = delete;

        ~ToneMapperPass () = default;

        [[nodiscard]] bool Init ( VkDevice device ) noexcept;
        void Destroy ( VkDevice device ) noexcept;

        [[maybe_unused]] void Execute ( VkCommandBuffer commandBuffer ) noexcept;

        [[nodiscard]] bool SetTarget ( android_vulkan::Renderer &renderer,
            VkRenderPass renderPass,
            uint32_t subpass,
            VkExtent2D const &viewport,
            VkImageView hdrView,
            VkBuffer exposure,
            VkSampler pointSampler
        ) noexcept;
};

} // namespace pbr


#endif // PBR_TONE_MAPPER_PASS_HPP
