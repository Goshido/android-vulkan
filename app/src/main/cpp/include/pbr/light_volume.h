#ifndef PBR_LIGHT_VOLUME_H
#define PBR_LIGHT_VOLUME_H


#include <uniform_buffer.h>
#include "gbuffer.h"
#include "light_volume_program.h"


namespace pbr {

class LightVolume final
{
    private:
        LightVolumeProgram      _program;

    public:
        LightVolume () noexcept;

        LightVolume ( LightVolume const & ) = delete;
        LightVolume& operator = ( LightVolume const & ) = delete;

        LightVolume ( LightVolume && ) = delete;
        LightVolume& operator = ( LightVolume && ) = delete;

        ~LightVolume () = default;

        void Execute ( uint32_t vertexCount, VkDescriptorSet transform, VkCommandBuffer commandBuffer );

        [[nodiscard]] bool Init ( android_vulkan::Renderer &renderer,
            GBuffer &gBuffer,
            VkRenderPass renderPass
        );

        void Destroy ( VkDevice device );

        [[nodiscard]] constexpr static uint32_t GetLightupSubpass ()
        {
            return 1U;
        }
};

} // namespace pbr

#endif // PBR_LIGHT_VOLUME_H
