#ifndef PBR_POINT_LIGHT_PASS_H
#define PBR_POINT_LIGHT_PASS_H


#include "gbuffer.h"
#include "lightup_common_descriptor_set.h"
#include "light_pass_notifier.h"
#include "light_volume.h"
#include "point_light_lightup.h"
#include "point_light_shadowmap_generator_program.h"
#include "scene_data.h"
#include "shadow_casters.h"
#include "uniform_buffer_pool.h"


namespace pbr {

class PointLightPass final
{
    public:
        using PointLightInfo = std::pair<PointLight*, android_vulkan::TextureCube const*>;

    private:
        using Interact = std::pair<LightRef, ShadowCasters>;
        using PointLightShadowmapInfo = std::pair<TextureCubeRef, VkFramebuffer>;

    private:
        VkCommandPool                           _commandPool;
        VkFence                                 _fence;
        std::vector<Interact>                   _interacts;

        VkDescriptorPool                        _lightDescriptorPool;
        std::vector<VkDescriptorSet>            _lightDescriptorSets;
        VkSubmitInfo                            _lightSubmitInfoTransfer;
        VkCommandBuffer                         _lightTransferCommandBuffer;
        std::vector<VkDescriptorBufferInfo>     _lightUniformInfo;
        UniformBufferPool                       _lightUniformPool;
        std::vector<VkWriteDescriptorSet>       _lightWriteSets;

        LightPassNotifier*                      _lightPassNotifier;
        PointLightLightup                       _lightup;

        VkDescriptorPool                        _shadowmapDescriptorPool;
        std::vector<VkDescriptorSet>            _shadowmapDescriptorSets;
        PointLightShadowmapGeneratorProgram     _shadowmapProgram;
        VkCommandBuffer                         _shadowmapRenderCommandBuffer;
        VkRenderPass                            _shadowmapRenderPass;
        VkRenderPassBeginInfo                   _shadowmapRenderPassInfo;
        VkSubmitInfo                            _shadowmapSubmitInfoRender;
        VkSubmitInfo                            _shadowmapSubmitInfoTransfer;
        VkCommandBuffer                         _shadowmapTransferCommandBuffer;
        std::vector<VkDescriptorBufferInfo>     _shadowmapUniformInfo;
        UniformBufferPool                       _shadowmapUniformPool;
        std::vector<VkWriteDescriptorSet>       _shadowmapWriteSets;
        std::vector<PointLightShadowmapInfo>    _shadowmaps;

        size_t                                  _usedShadowmaps;

    public:
        PointLightPass () noexcept;

        PointLightPass ( PointLightPass const & ) = delete;
        PointLightPass& operator = ( PointLightPass const & ) = delete;

        PointLightPass ( PointLightPass && ) = delete;
        PointLightPass& operator = ( PointLightPass && ) = delete;

        ~PointLightPass () = default;

        [[nodiscard]] bool ExecuteLightupPhase ( android_vulkan::Renderer &renderer,
            LightVolume &lightVolume,
            VkCommandBuffer commandBuffer,
            GXMat4 const &viewerLocal,
            GXMat4 const &view,
            GXMat4 const &viewProjection
        );

        [[nodiscard]] bool ExecuteShadowPhase ( android_vulkan::Renderer &renderer,
            SceneData const &sceneData,
            size_t opaqueMeshCount
        );

        [[nodiscard]] bool Init ( android_vulkan::Renderer &renderer,
            LightPassNotifier &notifier,
            VkExtent2D const &resolution,
            VkRenderPass lightupRenderPass
        );

        void Destroy ( VkDevice device );

        [[nodiscard]] size_t GetPointLightCount () const;
        [[nodiscard]] PointLightInfo GetPointLightInfo ( size_t lightIndex ) const;

        void Reset ();
        void Submit ( LightRef const &light );

    private:
        [[nodiscard]] bool AllocateLightDescriptorSets ( android_vulkan::Renderer &renderer, size_t neededSets );
        [[nodiscard]] bool AllocateShadowmapDescriptorSets ( android_vulkan::Renderer &renderer, size_t neededSets );

        // The method returns nullptr if it fails. Otherwise the method returns a valid pointer.
        [[nodiscard]] PointLightShadowmapInfo* AcquirePointLightShadowmap ( android_vulkan::Renderer &renderer );

        [[nodiscard]] bool CreateShadowmapRenderPass ( VkDevice device );
        void DestroyLightDescriptorPool ( VkDevice device );
        void DestroyShadowmapDescriptorPool ( VkDevice device );
        [[nodiscard]] bool GenerateShadowmaps ( android_vulkan::Renderer &renderer );

        [[nodiscard]] bool UpdateShadowmapGPUData ( android_vulkan::Renderer &renderer,
            SceneData const &sceneData,
            size_t opaqueMeshCount
        );

        [[nodiscard]] bool UpdateLightGPUData ( android_vulkan::Renderer &renderer, GXMat4 const &viewProjection );
};

} // namespace pbr


#endif // PBR_POINT_LIGHT_PASS_H
