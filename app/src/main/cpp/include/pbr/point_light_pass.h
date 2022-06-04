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
        VkCommandPool                           _commandPool = VK_NULL_HANDLE;
        VkFence                                 _fence = VK_NULL_HANDLE;
        std::vector<Interact>                   _interacts {};

        VkDescriptorPool                        _lightDescriptorPool = VK_NULL_HANDLE;
        std::vector<VkDescriptorSet>            _lightDescriptorSets {};
        VkSubmitInfo                            _lightSubmitInfoTransfer {};
        VkCommandBuffer                         _lightTransferCommandBuffer = VK_NULL_HANDLE;
        std::vector<VkDescriptorBufferInfo>     _lightUniformInfo {};
        UniformBufferPool                       _lightUniformPool { eUniformPoolSize::Tiny_4M };
        std::vector<VkWriteDescriptorSet>       _lightWriteSets {};

        LightPassNotifier*                      _lightPassNotifier = nullptr;
        PointLightLightup                       _lightup {};

        VkDescriptorPool                        _shadowmapDescriptorPool = VK_NULL_HANDLE;
        std::vector<VkDescriptorSet>            _shadowmapDescriptorSets {};
        PointLightShadowmapGeneratorProgram     _shadowmapProgram {};
        VkCommandBuffer                         _shadowmapRenderCommandBuffer = VK_NULL_HANDLE;
        VkRenderPass                            _shadowmapRenderPass = VK_NULL_HANDLE;
        VkRenderPassBeginInfo                   _shadowmapRenderPassInfo {};
        VkSubmitInfo                            _shadowmapSubmitInfoRender {};
        VkSubmitInfo                            _shadowmapSubmitInfoTransfer {};
        VkCommandBuffer                         _shadowmapTransferCommandBuffer = VK_NULL_HANDLE;
        std::vector<VkDescriptorBufferInfo>     _shadowmapUniformInfo {};
        UniformBufferPool                       _shadowmapUniformPool { eUniformPoolSize::Huge_64M };
        std::vector<VkWriteDescriptorSet>       _shadowmapWriteSets {};
        std::vector<PointLightShadowmapInfo>    _shadowmaps {};

        size_t                                  _usedShadowmaps = 0U;

    public:
        PointLightPass () = default;

        PointLightPass ( PointLightPass const & ) = delete;
        PointLightPass& operator = ( PointLightPass const & ) = delete;

        PointLightPass ( PointLightPass && ) = delete;
        PointLightPass& operator = ( PointLightPass && ) = delete;

        ~PointLightPass () = default;

        [[nodiscard]] bool ExecuteLightupPhase ( android_vulkan::Renderer &renderer,
            LightVolume &lightVolume,
            android_vulkan::MeshGeometry &unitCube,
            VkCommandBuffer commandBuffer,
            GXMat4 const &viewerLocal,
            GXMat4 const &view,
            GXMat4 const &viewProjection
        ) noexcept;

        [[nodiscard]] bool ExecuteShadowPhase ( android_vulkan::Renderer &renderer,
            SceneData const &sceneData,
            size_t opaqueMeshCount
        ) noexcept;

        [[nodiscard]] bool Init ( android_vulkan::Renderer &renderer,
            LightPassNotifier &notifier,
            VkCommandPool commandPool,
            VkExtent2D const &resolution,
            VkRenderPass lightupRenderPass
        ) noexcept;

        void Destroy ( VkDevice device ) noexcept;

        [[nodiscard]] size_t GetPointLightCount () const noexcept;
        [[nodiscard]] PointLightInfo GetPointLightInfo ( size_t lightIndex ) const noexcept;

        void Reset () noexcept;
        void Submit ( LightRef const &light ) noexcept;

    private:
        [[nodiscard]] bool AllocateLightDescriptorSets ( android_vulkan::Renderer &renderer,
            size_t neededSets
        ) noexcept;

        [[nodiscard]] bool AllocateShadowmapDescriptorSets ( android_vulkan::Renderer &renderer,
            size_t neededSets
        ) noexcept;

        // The method returns nullptr if it fails. Otherwise the method returns a valid pointer.
        [[nodiscard]] PointLightShadowmapInfo* AcquirePointLightShadowmap (
            android_vulkan::Renderer &renderer
        ) noexcept;

        [[nodiscard]] bool CreateShadowmapRenderPass ( VkDevice device ) noexcept;
        void DestroyLightDescriptorPool ( VkDevice device ) noexcept;
        void DestroyShadowmapDescriptorPool ( VkDevice device ) noexcept;
        [[nodiscard]] bool GenerateShadowmaps ( android_vulkan::Renderer &renderer ) noexcept;

        [[nodiscard]] bool UpdateShadowmapGPUData ( android_vulkan::Renderer &renderer,
            SceneData const &sceneData,
            size_t opaqueMeshCount
        ) noexcept;

        [[nodiscard]] bool UpdateLightGPUData ( android_vulkan::Renderer &renderer,
            GXMat4 const &viewProjection
        ) noexcept;
};

} // namespace pbr


#endif // PBR_POINT_LIGHT_PASS_H
