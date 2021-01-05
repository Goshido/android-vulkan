#ifndef PBR_LIGHT_VOLUME_H
#define PBR_LIGHT_VOLUME_H


#include "gbuffer.h"
#include "light_volume_program.h"
#include "types.h"


namespace pbr {

class LightVolume final
{
    private:
        LightVolumeProgram      _program;
        VkRenderPass            _renderPass;

    public:
        LightVolume () noexcept;

        LightVolume ( LightVolume const & ) = delete;
        LightVolume& operator = ( LightVolume const & ) = delete;

        LightVolume ( LightVolume && ) = delete;
        LightVolume& operator = ( LightVolume && ) = delete;

        ~LightVolume () = default;

        [[maybe_unused]] [[nodiscard]] bool Execute ( MeshRef const &mesh,
            GXMat4 const &transform,
            VkCommandBuffer commandBuffer,
            android_vulkan::Renderer &renderer
        );

        [[nodiscard]] bool Init ( GBuffer &gBuffer, android_vulkan::Renderer &renderer );
        void Destroy ( android_vulkan::Renderer &renderer );

    private:
        [[nodiscard]] bool CreateRenderPass ( GBuffer &gBuffer, android_vulkan::Renderer &renderer );
};

} // namespace pbr

#endif // PBR_LIGHT_VOLUME_H
