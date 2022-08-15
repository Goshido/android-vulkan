#ifndef PBR_LIGHTUP_COMMON_DESCRIPTOR_SET_H
#define PBR_LIGHTUP_COMMON_DESCRIPTOR_SET_H


#include "gbuffer.h"
#include "lightup_common_descriptor_set_layout.h"
#include "sampler.h"
#include "uniform_buffer_pool.h"


namespace pbr {

class LightupCommonDescriptorSet final
{
    private:
        VkBufferMemoryBarrier               _barrier {};
        android_vulkan::Texture2D           _brdfLUT {};
        Sampler                             _brdfLUTSampler {};
        VkDescriptorBufferInfo              _bufferInfo {};
        VkDescriptorPool                    _descriptorPool = VK_NULL_HANDLE;
        LightupCommonDescriptorSetLayout    _layout {};

        VkPipelineLayout                    _pipelineLayout = VK_NULL_HANDLE;
        Sampler                             _prefilterSampler {};
        std::vector<VkDescriptorSet>        _sets {};
        UniformBufferPool                   _uniforms { eUniformPoolSize::Tiny_4M, false };
        VkWriteDescriptorSet                _writeInfo {};

    public:
        LightupCommonDescriptorSet () = default;

        LightupCommonDescriptorSet ( LightupCommonDescriptorSet const & ) = delete;
        LightupCommonDescriptorSet& operator = ( LightupCommonDescriptorSet const & ) = delete;

        LightupCommonDescriptorSet ( LightupCommonDescriptorSet && ) = delete;
        LightupCommonDescriptorSet& operator = ( LightupCommonDescriptorSet && ) = delete;

        ~LightupCommonDescriptorSet () = default;

        void Bind ( VkCommandBuffer commandBuffer, size_t swapchainImageIndex ) noexcept;

        [[nodiscard]] bool Init ( android_vulkan::Renderer &renderer,
            VkCommandPool commandPool,
            GBuffer &gBuffer
        ) noexcept;

        void Destroy ( VkDevice device ) noexcept;
        void OnFreeTransferResources ( VkDevice device ) noexcept;

        void Update ( android_vulkan::Renderer &renderer,
            VkCommandBuffer commandBuffer,
            size_t swapchainImageIndex,
            VkExtent2D const &resolution,
            GXMat4 const &viewerLocal,
            GXMat4 const &cvvToView
        ) noexcept;
};

} // namespace pbr


#endif // PBR_LIGHTUP_COMMON_DESCRIPTOR_SET_H
