#ifndef PBR_LIGHTUP_COMMON_DESCRIPTOR_SET_HPP
#define PBR_LIGHTUP_COMMON_DESCRIPTOR_SET_HPP


#include "fif_count.hpp"
#include "gbuffer.hpp"
#include "lightup_common_descriptor_set_layout.hpp"
#include "sampler.hpp"
#include "uma_uniform_buffer.hpp"


namespace pbr {

class LightupCommonDescriptorSet final
{
    private:
        android_vulkan::Texture2D           _brdfLUT {};
        Sampler                             _brdfLUTSampler {};
        VkDescriptorPool                    _descriptorPool = VK_NULL_HANDLE;
        LightupCommonDescriptorSetLayout    _layout {};

        VkPipelineLayout                    _pipelineLayout = VK_NULL_HANDLE;
        Sampler                             _prefilterSampler {};
        Sampler                             _shadowSampler {};
        VkDescriptorSet                     _sets[ FIF_COUNT ]{};
        UMAUniformBuffer                    _uniforms{};
        VkMappedMemoryRange                 _uniformRanges[ FIF_COUNT ]{};

    public:
        explicit LightupCommonDescriptorSet () = default;

        LightupCommonDescriptorSet ( LightupCommonDescriptorSet const & ) = delete;
        LightupCommonDescriptorSet &operator = ( LightupCommonDescriptorSet const & ) = delete;

        LightupCommonDescriptorSet ( LightupCommonDescriptorSet && ) = delete;
        LightupCommonDescriptorSet &operator = ( LightupCommonDescriptorSet && ) = delete;

        ~LightupCommonDescriptorSet () = default;

        void Bind ( VkCommandBuffer commandBuffer, size_t commandBufferIndex ) noexcept;

        [[nodiscard]] bool Init ( android_vulkan::Renderer &renderer,
            VkCommandPool commandPool,
            GBuffer &gBuffer
        ) noexcept;

        void Destroy ( android_vulkan::Renderer &renderer ) noexcept;
        void OnFreeTransferResources ( android_vulkan::Renderer &renderer ) noexcept;

        [[nodiscard]] bool UploadGPUData ( VkDevice device,
            size_t commandBufferIndex,
            VkExtent2D const &resolution,
            GXMat4 const &viewerLocal,
            GXMat4 const &cvvToView
        ) noexcept;
};

} // namespace pbr


#endif // PBR_LIGHTUP_COMMON_DESCRIPTOR_SET_HPP
