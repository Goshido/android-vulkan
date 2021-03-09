#ifndef PBR_POINT_LIGHT_PASS_H
#define PBR_POINT_LIGHT_PASS_H


#include "gbuffer.h"
#include "lightup_common_descriptor_set.h"
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
        VkDescriptorPool                        _descriptorPool;
        VkFence                                 _fence;
        std::vector<Interact>                   _interacts;
        PointLightLightup                       _lightup;
        PointLightShadowmapGeneratorProgram     _program;
        VkCommandBuffer                         _renderCommandBuffer;
        VkRenderPass                            _shadowmapRenderPass;
        std::vector<PointLightShadowmapInfo>    _shadowmaps;
        VkSubmitInfo                            _submitInfoRender;
        VkSubmitInfo                            _submitInfoTransfer;
        VkCommandBuffer                         _transferCommandBuffer;
        UniformBufferPool                       _uniformPool;
        size_t                                  _usedShadowmaps;

    public:
        PointLightPass () noexcept;

        PointLightPass ( PointLightPass const & ) = delete;
        PointLightPass& operator = ( PointLightPass const & ) = delete;

        PointLightPass ( PointLightPass && ) = delete;
        PointLightPass& operator = ( PointLightPass && ) = delete;

        ~PointLightPass () = default;

        [[nodiscard]] bool ExecuteLightupPhase ( android_vulkan::Renderer &renderer,
            bool &isCommonSetBind,
            LightVolume &lightVolume,
            LightupCommonDescriptorSet &lightupCommonDescriptorSet,
            VkRenderPassBeginInfo const &renderPassBeginInfo,
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
            VkExtent2D const &resolution,
            VkRenderPass lightupRenderPass
        );

        void Destroy ( VkDevice device );

        [[nodiscard]] size_t GetPointLightCount () const;
        [[nodiscard]] PointLightInfo GetPointLightInfo ( size_t lightIndex ) const;

        void Reset ();
        void Submit ( LightRef const &light );

    private:
        // The method returns nullptr if it fails. Otherwise the method returns a valid pointer.
        [[nodiscard]] PointLightShadowmapInfo* AcquirePointLightShadowmap ( android_vulkan::Renderer &renderer );

        [[nodiscard]] bool CreateShadowmapRenderPass ( VkDevice device );
        void DestroyDescriptorPool ( VkDevice device );

        [[nodiscard]] bool GenerateShadowmaps ( VkDescriptorSet const* descriptorSets,
            android_vulkan::Renderer &renderer
        );

        [[nodiscard]] bool UpdateGPUData ( std::vector<VkDescriptorSet> &descriptorSetStorage,
            SceneData const &sceneData,
            size_t opaqueMeshCount,
            android_vulkan::Renderer &renderer
        );
};

} // namespace pbr


#endif // PBR_POINT_LIGHT_PASS_H
