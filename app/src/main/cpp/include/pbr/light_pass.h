#ifndef PBR_LIGHT_PASS_H
#define PBR_LIGHT_PASS_H


#include "gbuffer.h"
#include "lightup_common_descriptor_set.h"
#include "light_pass_notifier.h"
#include "light_volume.h"
#include "point_light_pass.h"
#include "reflection_global_pass.h"
#include "reflection_local_pass.h"


namespace pbr {

class LightPass final : public LightPassNotifier
{
    private:
        constexpr static size_t         INPUT_ATTACHMENTS = 3U;

        VkCommandPool                   _commandPool = VK_NULL_HANDLE;
        VkImageMemoryBarrier            _imageBarriers[ INPUT_ATTACHMENTS ] {};
        VkRenderPassBeginInfo           _lightupRenderPassInfo {};
        LightVolume                     _lightVolume {};
        LightupCommonDescriptorSet      _lightupCommonDescriptorSet {};
        size_t                          _lightupRenderPassCounter = 0U;
        PointLightPass                  _pointLightPass {};
        ReflectionGlobalPass            _reflectionGlobalPass {};
        ReflectionLocalPass             _reflectionLocalPass {};
        VkCommandBuffer                 _transfer = VK_NULL_HANDLE;
        android_vulkan::MeshGeometry    _unitCube {};

    public:
        LightPass () = default;

        LightPass ( LightPass const & ) = delete;
        LightPass& operator = ( LightPass const & ) = delete;

        LightPass ( LightPass && ) = delete;
        LightPass& operator = ( LightPass && ) = delete;

        ~LightPass () = default;

        [[nodiscard]] bool Init ( android_vulkan::Renderer &renderer,
            VkCommandPool commandPool,
            GBuffer &gBuffer
        ) noexcept;

        void Destroy ( VkDevice device ) noexcept;

        [[nodiscard]] size_t GetPointLightCount () const noexcept;
        [[nodiscard]] size_t GetReflectionLocalCount () const noexcept;
        void OnFreeTransferResources ( VkDevice device ) noexcept;

        [[nodiscard]] bool OnPreGeometryPass ( android_vulkan::Renderer &renderer,
            VkExtent2D const &resolution,
            SceneData const &sceneData,
            size_t opaqueMeshCount,
            GXMat4 const &viewerLocal,
            GXMat4 const &view,
            GXMat4 const &viewProjection,
            GXMat4 const &cvvToView
        ) noexcept;

        [[nodiscard]] bool OnPostGeometryPass ( android_vulkan::Renderer &renderer,
            VkCommandBuffer commandBuffer,
            GXMat4 const &viewerLocal,
            GXMat4 const &view,
            GXMat4 const &viewProjection
        ) noexcept;

        void Reset () noexcept;

        void SubmitPointLight ( LightRef const &light ) noexcept;
        void SubmitReflectionGlobal ( TextureCubeRef &prefilter ) noexcept;
        void SubmitReflectionLocal ( TextureCubeRef &prefilter, GXVec3 const &location, float size ) noexcept;

    private:
        void OnBeginLightWithVolume ( VkCommandBuffer commandBuffer ) noexcept override;
        void OnEndLightWithVolume ( VkCommandBuffer commandBuffer ) noexcept override;

        void CreateImageBarriers ( GBuffer &gBuffer ) noexcept;
        [[nodiscard]] bool CreateLightupFramebuffer ( VkDevice device, GBuffer &gBuffer ) noexcept;
        [[nodiscard]] bool CreateLightupRenderPass ( VkDevice device, GBuffer &gBuffer ) noexcept;
        [[nodiscard]] bool CreateUnitCube ( android_vulkan::Renderer &renderer, VkCommandPool commandPool ) noexcept;
};

} // namespace pbr


#endif // PBR_LIGHT_PASS_H
