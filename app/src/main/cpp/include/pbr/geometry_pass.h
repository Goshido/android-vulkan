#ifndef PBR_GEOMETRY_PASS_H
#define PBR_GEOMETRY_PASS_H


#include "gbuffer.h"
#include "opaque_program.h"
#include "render_session_stats.h"
#include "sampler_manager.h"
#include "scene_data.h"
#include "uniform_buffer_pool.h"


namespace pbr {

class GeometryPass final
{
    private:
        Texture2DRef                _albedoDefault;
        Texture2DRef                _emissionDefault;
        Texture2DRef                _maskDefault;
        Texture2DRef                _normalDefault;
        Texture2DRef                _paramDefault;

        bool                        _isFreeTransferResources;

        // The variable is needed for theoretical maximum uniform buffer elements estimation.
        size_t                      _maxBatchCount;
        size_t                      _maxUniqueCount;

        VkCommandPool               _commandPool;
        VkDescriptorPool            _descriptorPool;
        VkFence                     _fence;
        OpaqueProgram               _program;
        VkCommandBuffer             _renderCommandBuffer;
        VkRenderPassBeginInfo       _renderPassInfo;
        SceneData                   _sceneData;
        VkSubmitInfo                _submitInfoTransfer;
        VkCommandBuffer             _transferCommandBuffer;
        UniformBufferPool           _uniformPool;

    public:
        GeometryPass () noexcept;

        GeometryPass ( GeometryPass const & ) = delete;
        GeometryPass& operator = ( GeometryPass const & ) = delete;

        GeometryPass ( GeometryPass && ) = delete;
        GeometryPass& operator = ( GeometryPass && ) = delete;

        ~GeometryPass () = default;

        [[nodiscard]] SceneData& GetSceneData ();

        [[nodiscard]] bool Init ( VkExtent2D const &resolution,
            VkRenderPass renderPass,
            VkFramebuffer framebuffer,
            android_vulkan::Renderer &renderer
        );

        void Destroy ( android_vulkan::Renderer &renderer );

        // The method returns command buffer in recording state if success. Otherwise the method returns VK_NULL_HANDLE.
        [[nodiscard]] VkCommandBuffer Execute ( GXProjectionClipPlanes const &frustum,
            GXMat4 const &view,
            GXMat4 const &viewProjection,
            SamplerManager &samplerManager,
            RenderSessionStats &renderSessionStats,
            android_vulkan::Renderer &renderer
        );

        [[nodiscard]] VkFence GetFence () const;
        [[nodiscard]] size_t GetMaxUniqueCount () const;
        void Reset ();

        void Submit ( MeshRef &mesh,
            MaterialRef const &material,
            GXMat4 const &local,
            GXAABB const &worldBounds,
            android_vulkan::Half4 const &color0,
            android_vulkan::Half4 const &color1,
            android_vulkan::Half4 const &color2,
            android_vulkan::Half4 const &color3
        );

    private:
        void AppendDrawcalls ( VkDescriptorSet const* textureSets,
            VkDescriptorSet const* instanceSets,
            RenderSessionStats &renderSessionStats
        );

        [[nodiscard]] bool BeginRenderPass ( android_vulkan::Renderer &renderer );

        void CleanupTransferResources ( android_vulkan::Renderer &renderer );
        void InitCommonStructures ( VkRenderPass renderPass, VkFramebuffer framebuffer, VkExtent2D const &resolution );

        [[nodiscard]] bool InitDefaultTextures ( VkCommandBuffer const* commandBuffers,
            android_vulkan::Renderer &renderer
        );

        void DestroyDefaultTextures ( android_vulkan::Renderer &renderer );
        void DestroyDescriptorPool ( android_vulkan::Renderer &renderer );

        [[nodiscard]] bool UpdateGPUData ( GXProjectionClipPlanes const &frustum,
            GXMat4 const &view,
            GXMat4 const &viewProjection,
            std::vector<VkDescriptorSet> &descriptorSetStorage,
            SamplerManager &samplerManager,
            android_vulkan::Renderer &renderer
        );
};

} // namespace pbr


#endif // PBR_GEOMETRY_PASS_H
