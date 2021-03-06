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
    public:
        constexpr static const size_t DEFAULT_TEXTURE_COUNT = 5U;

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
        VkCommandBuffer             _textureCommandBuffers[ DEFAULT_TEXTURE_COUNT ];
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

        [[nodiscard]] bool Init ( android_vulkan::Renderer &renderer,
            VkCommandPool commandPool,
            VkExtent2D const &resolution,
            VkRenderPass renderPass,
            VkFramebuffer framebuffer
        );

        void Destroy ( VkDevice device );

        // The method returns command buffer in recording state if success. Otherwise the method returns VK_NULL_HANDLE.
        [[nodiscard]] VkCommandBuffer Execute ( android_vulkan::Renderer &renderer,
            GXProjectionClipPlanes const &frustum,
            GXMat4 const &view,
            GXMat4 const &viewProjection,
            SamplerManager &samplerManager,
            RenderSessionStats &renderSessionStats
        );

        [[nodiscard]] VkFence GetFence () const;
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

        // Return value optimization friendly method.
        [[nodiscard]] static VkSubpassDescription GetSubpassDescription ();

    private:
        void AppendDrawcalls ( VkDescriptorSet const* textureSets,
            VkDescriptorSet const* instanceSets,
            RenderSessionStats &renderSessionStats
        );

        [[nodiscard]] bool BeginRenderPass ( android_vulkan::Renderer &renderer );

        void CleanupTransferResources ( android_vulkan::Renderer &renderer );
        void InitCommonStructures ( VkRenderPass renderPass, VkFramebuffer framebuffer, VkExtent2D const &resolution );
        [[nodiscard]] bool InitDefaultTextures ( android_vulkan::Renderer &renderer );

        void DestroyDefaultTextures ( VkDevice device );
        void DestroyDescriptorPool ( VkDevice device );

        [[nodiscard]] bool UpdateGPUData ( android_vulkan::Renderer &renderer,
            GXProjectionClipPlanes const &frustum,
            GXMat4 const &view,
            GXMat4 const &viewProjection,
            std::vector<VkDescriptorSet> &descriptorSetStorage,
            SamplerManager &samplerManager
        );
};

} // namespace pbr


#endif // PBR_GEOMETRY_PASS_H
