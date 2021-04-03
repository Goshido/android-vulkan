#ifndef PBR_LIGHT_PASS_H
#define PBR_LIGHT_PASS_H


#include "gbuffer.h"
#include "lightup_common_descriptor_set.h"
#include "light_pass_notifier.h"
#include "light_volume.h"
#include "point_light_pass.h"
#include "reflection_global_pass.h"


namespace pbr {

class LightPass final : public LightPassNotifier
{
    private:
        VkCommandBuffer                 _commandBuffer;
        VkCommandPool                   _commandPool;
        VkRenderPassBeginInfo           _lightupRenderPassInfo;
        LightVolume                     _lightVolume;
        LightupCommonDescriptorSet      _lightupCommonDescriptorSet;
        size_t                          _lightupRenderPassCounter;
        PointLightPass                  _pointLightPass;
        ReflectionGlobalPass            _reflectionGlobalPass;

    public:
        LightPass () noexcept;

        LightPass ( LightPass const & ) = delete;
        LightPass& operator = ( LightPass const & ) = delete;

        LightPass ( LightPass && ) = delete;
        LightPass& operator = ( LightPass && ) = delete;

        ~LightPass () = default;

        [[nodiscard]] bool Init ( android_vulkan::Renderer &renderer, GBuffer &gBuffer );
        void Destroy ( VkDevice device );

        [[nodiscard]] size_t GetPointLightCount () const;
        void OnFreeTransferResources ( VkDevice device );

        [[nodiscard]] bool OnPreGeometryPass ( android_vulkan::Renderer &renderer,
            VkExtent2D const &resolution,
            SceneData const &sceneData,
            size_t opaqueMeshCount,
            GXMat4 const &viewerLocal,
            GXMat4 const &cvvToView
        );

        [[nodiscard]] bool OnPostGeometryPass ( android_vulkan::Renderer &renderer,
            VkCommandBuffer commandBuffer,
            GXMat4 const &viewerLocal,
            GXMat4 const &view,
            GXMat4 const &viewProjection
        );

        void Reset ();

        void SubmitPointLight ( LightRef const &light );
        void SubmitReflectionGlobal ( TextureCubeRef &prefilter );

        [[maybe_unused]] static void SubmitReflectionLocal ( TextureCubeRef &prefilter,\
            GXVec3 const &location,
            float size
        );

    private:
        void OnBeginLightWithVolume ( VkCommandBuffer commandBuffer ) override;
        void OnEndLightWithVolume ( VkCommandBuffer commandBuffer ) override;

        [[nodiscard]] bool CreateLightupFramebuffer ( VkDevice device, GBuffer &gBuffer );
        [[nodiscard]] bool CreateLightupRenderPass ( VkDevice device, GBuffer &gBuffer );
        void DestroyCommandPool ( VkDevice device );
};

} // namespace pbr


#endif // PBR_LIGHT_PASS_H
