#ifndef PBR_POINT_LIGHT_PASS_H
#define PBR_POINT_LIGHT_PASS_H


#include "point_light_shadowmap_generator_program.h"
#include "scene_data.h"
#include "shadow_casters.h"
#include "uniform_buffer_pool.h"


namespace pbr {

class PointLightPass final
{
    private:
        using Interact = std::pair<LightRef, ShadowCasters>;
        using PointLightShadowmapInfo = std::pair<TextureCubeRef, VkFramebuffer>;

    private:
        VkCommandPool                           _commandPool;
        VkDescriptorPool                        _descriptorPool;
        VkFence                                 _fence;
        std::vector<Interact>                   _interacts;
        PointLightShadowmapGeneratorProgram     _program;
        VkCommandBuffer                         _renderCommandBuffer;
        VkRenderPass                            _renderPass;
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

        [[nodiscard]] bool Execute ( SceneData const &sceneData,
            size_t opaqueMeshCount,
            android_vulkan::Renderer &renderer
        );

        [[nodiscard]] bool Init ( android_vulkan::Renderer &renderer );
        void Destroy ( android_vulkan::Renderer &renderer );

        void Reset ();
        void Submit ( LightRef const &light );

    private:
        // The method returns nullptr if it fails. Otherwise the method returns a valid pointer.
        [[nodiscard]] PointLightShadowmapInfo* AcquirePointLightShadowmap ( android_vulkan::Renderer &renderer );

        [[nodiscard]] bool CreateRenderPass ( android_vulkan::Renderer &renderer );
        void DestroyDescriptorPool ( android_vulkan::Renderer &renderer );

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
