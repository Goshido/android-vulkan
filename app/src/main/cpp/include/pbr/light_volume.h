#ifndef PBR_LIGHT_VOLUME_H
#define PBR_LIGHT_VOLUME_H


#include <uniform_buffer.h>
#include "gbuffer.h"
#include "lightup_common_descriptror_set_layout.h"
#include "light_volume_program.h"
#include "types.h"


namespace pbr {

class LightVolume final
{
    private:
        VkCommandPool                       _commandPool;
        VkDescriptorPool                    _descriptorPool;
        VkDescriptorSet                     _descriptorSet;
        LightupCommonDescriptorSetLayout    _lightupLayout;
        LightVolumeProgram                  _program;
        VkRenderPass                        _renderPass;
        VkRenderPassBeginInfo               _renderPassInfo;
        android_vulkan::UniformBuffer       _uniform;

    public:
        LightVolume () noexcept;

        LightVolume ( LightVolume const & ) = delete;
        LightVolume& operator = ( LightVolume const & ) = delete;

        LightVolume ( LightVolume && ) = delete;
        LightVolume& operator = ( LightVolume && ) = delete;

        ~LightVolume () = default;

        [[maybe_unused]] [[nodiscard]] bool Execute ( android_vulkan::Renderer &renderer,
            MeshRef const &mesh,
            GXMat4 const &transform,
            VkCommandBuffer commandBuffer
        );

        [[nodiscard]] bool Init ( android_vulkan::Renderer &renderer,
            GBuffer &gBuffer,
            VkFramebuffer framebuffer,
            GXMat4 const &cvvToView
        );

        void Destroy ( android_vulkan::Renderer &renderer );

        [[nodiscard]] VkRenderPass GetRenderPass () const;
        [[maybe_unused]] [[nodiscard]] VkDescriptorSet GetLighupCommonDescriptorSet () const;
        [[nodiscard]] static uint32_t GetLightupSubpass ();

    private:
        [[nodiscard]] bool CreateDescriptorSet ( android_vulkan::Renderer &renderer,
            GBuffer &gBuffer,
            GXMat4 const &cvvToView
        );

        [[nodiscard]] bool CreateRenderPass ( VkDevice device, GBuffer &gBuffer, VkFramebuffer framebuffer );
};

} // namespace pbr

#endif // PBR_LIGHT_VOLUME_H
