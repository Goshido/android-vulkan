#ifndef PBR_LIGHTUP_COMMON_DESCRIPTOR_SET_H
#define PBR_LIGHTUP_COMMON_DESCRIPTOR_SET_H


#include <uniform_buffer.h>
#include "gbuffer.h"
#include "lightup_common_descriptor_set_layout.h"
#include "sampler.h"


namespace pbr {

class LightupCommonDescriptorSet final
{
    private:
        android_vulkan::Texture2D           _brdfLUT {};
        Sampler                             _brdfLUTSampler {};
        VkCommandBuffer                     _brdfTransfer = VK_NULL_HANDLE;
        VkDescriptorPool                    _descriptorPool = VK_NULL_HANDLE;
        LightupCommonDescriptorSetLayout    _layout {};

        VkCommandPool                       _persistentCommandPool = VK_NULL_HANDLE;
        VkCommandPool                       _workingCommandPool = VK_NULL_HANDLE;

        VkPipelineLayout                    _pipelineLayout = VK_NULL_HANDLE;
        Sampler                             _prefilterSampler {};
        VkDescriptorSet                     _set = VK_NULL_HANDLE;
        android_vulkan::UniformBuffer       _uniformBuffer {};

    public:
        LightupCommonDescriptorSet () = default;

        LightupCommonDescriptorSet ( LightupCommonDescriptorSet const & ) = delete;
        LightupCommonDescriptorSet& operator = ( LightupCommonDescriptorSet const & ) = delete;

        LightupCommonDescriptorSet ( LightupCommonDescriptorSet && ) = delete;
        LightupCommonDescriptorSet& operator = ( LightupCommonDescriptorSet && ) = delete;

        ~LightupCommonDescriptorSet () = default;

        void Bind ( VkCommandBuffer commandBuffer ) noexcept;

        [[nodiscard]] bool Init ( android_vulkan::Renderer &renderer,
            VkCommandPool commandPool,
            GBuffer &gBuffer
        ) noexcept;

        void Destroy ( VkDevice device ) noexcept;
        void OnFreeTransferResources ( VkDevice device ) noexcept;

        [[nodiscard]] bool Update ( android_vulkan::Renderer &renderer,
            VkExtent2D const &resolution,
            GXMat4 const &viewerLocal,
            GXMat4 const &cvvToView
        ) noexcept;
};

} // namespace pbr


#endif // PBR_LIGHTUP_COMMON_DESCRIPTOR_SET_H
