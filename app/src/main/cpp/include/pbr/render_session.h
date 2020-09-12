#ifndef PBR_RENDER_SESSION_H
#define PBR_RENDER_SESSION_H


#include <GXCommon/GXMath.h>
#include "gbuffer.h"
#include "opaque_batch_program.h"
#include "opaque_call.h"
#include "opaque_material.h"
#include "opaque_program.h"
#include "texture_present_program.h"
#include "uniform_buffer_pool.h"


namespace pbr {

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
        VkRenderPass                            _gBufferRenderPass;

        VkFence                                 _geometryPassFence;
        VkCommandBuffer                         _geometryPassRendering;
        VkCommandBuffer                         _geometryPassTransfer;

        bool                                    _isFreeTransferResources;

        size_t                                  _maximumOpaqueBatchCount;
        size_t                                  _meshCount;

        std::map<OpaqueMaterial, OpaqueCall>    _opaqueCalls;

        OpaqueBatchProgram                      _opaqueBatchProgram;
        OpaqueProgram                           _opaqueProgram;
        TexturePresentProgram                   _texturePresentProgram;

        GXMat4                                  _view;
        GXMat4                                  _viewProjection;

        UniformBufferPool                       _uniformBufferPool;

    public:
        RenderSession ();

        RenderSession ( RenderSession const &other ) = delete;
        RenderSession& operator = ( RenderSession const &other ) = delete;

        RenderSession ( RenderSession &&other ) = delete;
        RenderSession& operator = ( RenderSession &&other ) = delete;

        ~RenderSession () = default;

        void Begin ( GXMat4 const &view, GXMat4 const &projection );
        [[nodiscard]] bool End ( ePresentTarget target, android_vulkan::Renderer &renderer );

        [[nodiscard]] VkExtent2D const& GetResolution () const;

        [[nodiscard]] bool Init ( android_vulkan::Renderer &renderer, VkRenderPass presentRenderPass );
        void Destroy ( android_vulkan::Renderer &renderer );

        void SubmitMesh ( MeshRef &mesh, MaterialRef &material, GXMat4 const &local );

    private:
        [[nodiscard]] bool CreateGBufferFramebuffer ( android_vulkan::Renderer &renderer );
        [[nodiscard]] bool CreateGBufferRenderPass ( android_vulkan::Renderer &renderer );
        void DestroyDescriptorPool ( android_vulkan::Renderer &renderer );
        void SubmitOpaqueCall ( MeshRef &mesh, MaterialRef &material, GXMat4 const &local );
};

} // namespace pbr


#endif // PBR_RENDER_SESSION_H
