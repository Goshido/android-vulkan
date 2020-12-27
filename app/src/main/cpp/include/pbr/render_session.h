#ifndef PBR_RENDER_SESSION_H
#define PBR_RENDER_SESSION_H


#include <GXCommon/GXMath.h>
#include "gbuffer.h"
#include "opaque_program.h"
#include "opaque_call.h"
#include "opaque_material.h"
#include "point_light.h"
#include "point_light_shadowmap_generator_program.h"
#include "render_session_stats.h"
#include "shadow_casters.h"
#include "texture_present_program.h"
#include "uniform_buffer_pool.h"


namespace pbr {

constexpr static size_t const GBUFFER_ATTACHMENT_COUNT = 5U;

enum class ePresentTarget : uint8_t
{
    Albedo [[maybe_unused]],
    Emission [[maybe_unused]],
    Normal [[maybe_unused]],
    Param [[maybe_unused]]
};

// Single threaded class
class RenderSession final
{
    private:
        using LightInteract = std::pair<LightRef, ShadowCasters>;

    private:
        Texture2DRef                            _albedoDefault;
        Texture2DRef                            _emissionDefault;
        Texture2DRef                            _maskDefault;
        Texture2DRef                            _normalDefault;
        Texture2DRef                            _paramDefault;

        VkCommandPool                           _commandPool;
        GXProjectionClipPlanes                  _frustum;

        GBuffer                                 _gBuffer;
        VkFramebuffer                           _gBufferFramebuffer;
        VkImageMemoryBarrier                    _gBufferImageBarrier;
        VkRenderPass                            _gBufferRenderPass;

        VkRenderPassBeginInfo                   _geometryPassBeginInfo;
        VkClearValue                            _geometryPassClearValue[ GBUFFER_ATTACHMENT_COUNT ];
        VkDescriptorPool                        _geometryPassDescriptorPool;
        VkFence                                 _geometryPassFence;
        VkCommandBuffer                         _geometryPassRendering;
        VkCommandBuffer                         _geometryPassTransfer;
        UniformBufferPool                       _geometryPassUniformBufferPool;

        bool                                    _isFreeTransferResources;

        // The variable is needed for theoretical maximum uniform buffer elements estimation.
        size_t                                  _maxBatchCount;
        size_t                                  _maxUniqueCount;

        std::map<OpaqueMaterial, OpaqueCall>    _opaqueCalls;

        OpaqueProgram                           _opaqueBatchProgram;
        PointLightShadowmapGeneratorProgram     _pointLightShadowmapGeneratorProgram;
        TexturePresentProgram                   _texturePresentProgram;

        std::vector<LightInteract>              _pointLightCalls;
        VkDescriptorPool                        _pointLightShadowmapDescriptorPool;
        std::vector<TextureCubeRef>             _pointLightShadowmaps;
        UniformBufferPool                       _pointLightShadowmapPassUniformBufferPool;
        VkRenderPass                            _pointLightShadowmapRenderPass;
        VkCommandBuffer                         _pointLightShadowmapTransfer;

        VkPresentInfoKHR                        _presentInfo;
        VkRenderPassBeginInfo                   _presentBeginInfo;
        VkClearValue                            _presentClearValue;
        std::vector<VkFramebuffer>              _presentFramebuffers;
        VkRenderPass                            _presentRenderPass;
        VkSemaphore                             _presentRenderPassEndSemaphore;
        VkSemaphore                             _presentRenderTargetAcquiredSemaphore;

        RenderSessionStats                      _renderSessionStats;

        VkSubmitInfo                            _submitInfoRender;
        VkSubmitInfo                            _submitInfoTransferGeometryPass;
        VkSubmitInfo                            _submitInfoTransferPointLightShadowmap;

        GXMat4                                  _view;
        GXMat4                                  _viewProjection;

    public:
        RenderSession () noexcept;

        RenderSession ( RenderSession const & ) = delete;
        RenderSession& operator = ( RenderSession const & ) = delete;

        RenderSession ( RenderSession && ) = delete;
        RenderSession& operator = ( RenderSession && ) = delete;

        ~RenderSession () = default;

        void Begin ( GXMat4 const &view, GXMat4 const &projection );
        [[nodiscard]] bool End ( ePresentTarget target, double deltaTime, android_vulkan::Renderer &renderer );

        [[nodiscard]] bool Init ( android_vulkan::Renderer &renderer, VkExtent2D const &resolution );
        void Destroy ( android_vulkan::Renderer &renderer );

        void SubmitMesh ( MeshRef &mesh,
            MaterialRef const &material,
            GXMat4 const &local,
            GXAABB const &worldBounds,
            android_vulkan::Half4 const &color0,
            android_vulkan::Half4 const &color1,
            android_vulkan::Half4 const &color2,
            android_vulkan::Half4 const &color3
        );

        void SubmitLight ( LightRef const &light );

    private:
        [[nodiscard]] bool BeginGeometryRenderPass ( android_vulkan::Renderer &renderer );
        void CleanupTransferResources ( android_vulkan::Renderer &renderer );

        [[nodiscard]] bool CreateGBufferFramebuffer ( android_vulkan::Renderer &renderer );
        [[nodiscard]] bool CreateGBufferRenderPass ( android_vulkan::Renderer &renderer );
        [[nodiscard]] bool CreatePointLightShadowmapRenderPass ( android_vulkan::Renderer &renderer );

        [[nodiscard]] bool CreatePresentFramebuffers ( android_vulkan::Renderer &renderer );
        void DestroyPresentFramebuffers ( android_vulkan::Renderer &renderer );

        [[nodiscard]] bool CreatePresentRenderPass ( android_vulkan::Renderer &renderer );

        [[nodiscard]] bool CreateSyncPrimitives ( android_vulkan::Renderer &renderer );
        void DestroySyncPrimitives ( android_vulkan::Renderer &renderer );

        void DestroyGeometryPassDescriptorPool ( android_vulkan::Renderer &renderer );
        void DestroyPointLightShadowmapDescriptorPool ( android_vulkan::Renderer &renderer );
        void DrawOpaque ( VkDescriptorSet const* textureSets, VkDescriptorSet const* instanceSets );

        void InitCommonStructures ();

        void SubmitOpaqueCall ( MeshRef &mesh,
            MaterialRef const &material,
            GXMat4 const &local,
            GXAABB const &worldBounds,
            android_vulkan::Half4 const &color0,
            android_vulkan::Half4 const &color1,
            android_vulkan::Half4 const &color2,
            android_vulkan::Half4 const &color3
        );

        void SubmitPointLight ( LightRef const &light );

        [[nodiscard]] bool UpdateGeometryPassGPUData ( std::vector<VkDescriptorSet> &descriptorSetStorage,
            android_vulkan::Renderer &renderer
        );

        [[nodiscard]] bool UpdatePointLightShadowmapGPUData ( android_vulkan::Renderer &renderer );
};

} // namespace pbr


#endif // PBR_RENDER_SESSION_H
