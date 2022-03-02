#ifndef PBR_OPAQUE_PASS_H
#define PBR_OPAQUE_PASS_H


#include "gbuffer.h"
#include "opaque_program.h"
#include "render_session_stats.h"
#include "sampler_manager.h"
#include "scene_data.h"
#include "uniform_buffer_pool.h"
#include <primitive_types.h>


namespace pbr {

class OpaquePass final
{
    private:
        constexpr static size_t     DEFAULT_TEXTURE_COUNT = 5U;

    private:
        Texture2DRef                _albedoDefault {};
        Texture2DRef                _emissionDefault {};
        Texture2DRef                _maskDefault {};
        Texture2DRef                _normalDefault {};
        Texture2DRef                _paramDefault {};

        bool                        _isFreeTransferResources = false;

        VkCommandPool               _commandPool = VK_NULL_HANDLE;
        VkDescriptorPool            _descriptorPool = VK_NULL_HANDLE;
        VkFence                     _fence = VK_NULL_HANDLE;
        OpaqueProgram               _program {};
        VkCommandBuffer             _renderCommandBuffer = VK_NULL_HANDLE;
        VkRenderPassBeginInfo       _renderPassInfo {};
        SceneData                   _sceneData {};
        VkSubmitInfo                _submitInfoTransfer {};
        VkCommandBuffer             _textureCommandBuffers[ DEFAULT_TEXTURE_COUNT ] {};
        VkCommandBuffer             _transferCommandBuffer = VK_NULL_HANDLE;
        UniformBufferPool           _uniformPool { eUniformPoolSize::Huge_64M };

    public:
        OpaquePass () = default;

        OpaquePass ( OpaquePass const & ) = delete;
        OpaquePass& operator = ( OpaquePass const & ) = delete;

        OpaquePass ( OpaquePass && ) = delete;
        OpaquePass& operator = ( OpaquePass && ) = delete;

        ~OpaquePass () = default;

        [[nodiscard]] SceneData& GetSceneData () noexcept;

        [[nodiscard]] bool Init ( android_vulkan::Renderer &renderer,
            VkCommandPool commandPool,
            VkExtent2D const &resolution,
            VkRenderPass renderPass,
            VkFramebuffer framebuffer
        ) noexcept;

        void Destroy ( VkDevice device ) noexcept;

        // The method returns command buffer in recording state if success. Otherwise the method returns VK_NULL_HANDLE.
        [[nodiscard]] VkCommandBuffer Execute ( android_vulkan::Renderer &renderer,
            GXProjectionClipPlanes const &frustum,
            GXMat4 const &view,
            GXMat4 const &viewProjection,
            SamplerManager &samplerManager,
            RenderSessionStats &renderSessionStats
        ) noexcept;

        [[nodiscard]] VkFence GetFence () const noexcept;
        void Reset () noexcept;

        void Submit ( MeshRef &mesh,
            MaterialRef const &material,
            GXMat4 const &local,
            GXAABB const &worldBounds,
            GXColorRGB const &color0,
            GXColorRGB const &color1,
            GXColorRGB const &color2,
            GXColorRGB const &emission
        ) noexcept;

        // Return value optimization friendly method.
        [[nodiscard]] static VkSubpassDescription GetSubpassDescription () noexcept;

    private:
        [[nodiscard]] size_t AggregateUniformCount () const noexcept;

        void AppendDrawcalls ( VkDescriptorSet const* textureSets,
            VkDescriptorSet const* instanceSets,
            RenderSessionStats &renderSessionStats
        ) noexcept;

        [[nodiscard]] bool BeginRenderPass ( android_vulkan::Renderer &renderer ) noexcept;
        void CleanupTransferResources ( android_vulkan::Renderer &renderer ) noexcept;

        void InitCommonStructures ( VkRenderPass renderPass,
            VkFramebuffer framebuffer,
            VkExtent2D const &resolution
        ) noexcept;

        [[nodiscard]] bool InitDefaultTextures ( android_vulkan::Renderer &renderer ) noexcept;

        void DestroyDefaultTextures ( VkDevice device ) noexcept;
        void DestroyDescriptorPool ( VkDevice device ) noexcept;

        [[nodiscard]] bool UpdateGPUData ( android_vulkan::Renderer &renderer,
            GXProjectionClipPlanes const &frustum,
            GXMat4 const &view,
            GXMat4 const &viewProjection,
            std::vector<VkDescriptorSet> &descriptorSetStorage,
            SamplerManager &samplerManager
        ) noexcept;
};

} // namespace pbr


#endif // PBR_OPAQUE_PASS_H
