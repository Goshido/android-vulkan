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
        android_vulkan::Texture2D           _brdfLUT;
        Sampler                             _brdfLUTSampler;
        VkCommandBuffer                     _brdfTransfer;
        VkCommandPool                       _commandPool;
        VkDescriptorPool                    _descriptorPool;
        LightupCommonDescriptorSetLayout    _layout;
        VkPipelineLayout                    _pipelineLayout;
        Sampler                             _prefilterSampler;
        VkDescriptorSet                     _set;
        android_vulkan::UniformBuffer       _uniformBuffer;

    public:
        LightupCommonDescriptorSet () noexcept;

        LightupCommonDescriptorSet ( LightupCommonDescriptorSet const & ) = delete;
        LightupCommonDescriptorSet& operator = ( LightupCommonDescriptorSet const & ) = delete;

        LightupCommonDescriptorSet ( LightupCommonDescriptorSet && ) = delete;
        LightupCommonDescriptorSet& operator = ( LightupCommonDescriptorSet && ) = delete;

        ~LightupCommonDescriptorSet () = default;

        void Bind ( VkCommandBuffer commandBuffer );

        [[nodiscard]] bool Init ( android_vulkan::Renderer &renderer, GBuffer &gBuffer );
        void Destroy ( VkDevice device );
        void OnFreeTransferResources ( VkDevice device );

        [[nodiscard]] bool Update ( android_vulkan::Renderer &renderer,
            VkExtent2D const &resolution,
            GXMat4 const &viewerLocal,
            GXMat4 const &cvvToView
        );
};

} // namespace pbr


#endif // PBR_LIGHTUP_COMMON_DESCRIPTOR_SET_H
