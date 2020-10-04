#ifndef PBR_RENDER_SESSION_H
#define PBR_RENDER_SESSION_H


#include <GXCommon/GXMath.h>
#include "gbuffer.h"
#include "opaque_program.h"
#include "opaque_call.h"
#include "opaque_material.h"
#include "texture_present_program.h"
#include "uniform_buffer_pool.h"


namespace pbr {

constexpr static size_t const GBUFFER_ATTACHMENT_COUNT = 5U;

enum class ePresentTarget : uint8_t
{
    Albedo,
    Emission,
    Normal,
    Param
};

// Single threaded class
class RenderSession final
{
    private:
        Texture2DRef                            _albedoDefault;
        Texture2DRef                            _emissionDefault;
        Texture2DRef                            _normalDefault;
        Texture2DRef                            _paramDefault;

        VkCommandPool                           _commandPool;
        VkDescriptorPool                        _descriptorPool;

        GBuffer                                 _gBuffer;
        VkFramebuffer                           _gBufferFramebuffer;
        VkImageMemoryBarrier                    _gBufferImageBarrier;
        VkRenderPass                            _gBufferRenderPass;

        VkRenderPassBeginInfo                   _geometryPassBeginInfo;
        VkClearValue                            _geometryPassClearValue[ GBUFFER_ATTACHMENT_COUNT ];
        VkFence                                 _geometryPassFence;
        VkCommandBuffer                         _geometryPassRendering;
        VkCommandBuffer                         _geometryPassTransfer;

        bool                                    _isFreeTransferResources;

        // The variable is needed for theoretical maximum uniform buffer elements estimation.
        size_t                                  _maxBatchCount;
        size_t                                  _maxUniqueCount;

        std::map<OpaqueMaterial, OpaqueCall>    _opaqueCalls;

        OpaqueProgram                      _opaqueBatchProgram;
        TexturePresentProgram                   _texturePresentProgram;

        VkPresentInfoKHR                        _presentInfo;
        VkRenderPassBeginInfo                   _presentBeginInfo;
        VkClearValue                            _presentClearValue;
        std::vector<VkFramebuffer>              _presentFramebuffers;
        VkRenderPass                            _presentRenderPass;
        VkSemaphore                             _presentRenderPassEndSemaphore;
        VkSemaphore                             _presentRenderTargetAcquiredSemaphore;

        VkSubmitInfo                            _submitInfoRender;
        VkSubmitInfo                            _submitInfoTransfer;

        UniformBufferPool                       _uniformBufferPool;

        GXMat4                                  _view;
        GXMat4                                  _viewProjection;

    public:
        RenderSession ();

        RenderSession ( RenderSession const &other ) = delete;
        RenderSession& operator = ( RenderSession const &other ) = delete;

        RenderSession ( RenderSession &&other ) = delete;
        RenderSession& operator = ( RenderSession &&other ) = delete;

        ~RenderSession () = default;

        void Begin ( GXMat4 const &view, GXMat4 const &projection );
        [[nodiscard]] bool End ( ePresentTarget target, android_vulkan::Renderer &renderer );

        [[nodiscard]] bool Init ( android_vulkan::Renderer &renderer, VkExtent2D const &resolution );
        void Destroy ( android_vulkan::Renderer &renderer );

        void SubmitMesh ( MeshRef &mesh, MaterialRef &material, GXMat4 const &local );

    private:
        [[nodiscard]] bool BeginGeometryRenderPass ( android_vulkan::Renderer &renderer );
        void CleanupTransferResources ( android_vulkan::Renderer &renderer );

        [[nodiscard]] bool CreateGBufferFramebuffer ( android_vulkan::Renderer &renderer );
        [[nodiscard]] bool CreateGBufferRenderPass ( android_vulkan::Renderer &renderer );

        [[nodiscard]] bool CreatePresentFramebuffers ( android_vulkan::Renderer &renderer );
        void DestroyPresentFramebuffers ( android_vulkan::Renderer &renderer );

        [[nodiscard]] bool CreatePresentRenderPass ( android_vulkan::Renderer &renderer );

        [[nodiscard]] bool CreateSyncPrimitives ( android_vulkan::Renderer &renderer );
        void DestroySyncPrimitives ( android_vulkan::Renderer &renderer );

        void DestroyDescriptorPool ( android_vulkan::Renderer &renderer );
        void DrawOpaque ( VkDescriptorSet const* textureSets, VkDescriptorSet const* instanceSets ) const;

        void InitCommonStructures ();
        void SubmitOpaqueCall ( MeshRef &mesh, MaterialRef &material, GXMat4 const &local );

        [[nodiscard]] bool UpdateGPUData ( std::vector<VkDescriptorSet> &descriptorSetStorage,
            android_vulkan::Renderer &renderer
        );
};

} // namespace pbr


#endif // PBR_RENDER_SESSION_H
