#ifndef RENDER_SESSION_H
#define RENDER_SESSION_H


#include <GXCommon/GXMath.h>
#include "gbuffer.h"
#include "opaque_call.h"
#include "opaque_material.h"
#include "opaque_program.h"
#include "texture_present_program.h"


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

        VkCommandBuffer                         _geometryPassCommandBuffer;
        VkFence                                 _geometryPassFence;

        bool                                    _isFreeTransferResources;

        size_t                                  _maximumOpaqueBatchCount;
        size_t                                  _meshCount;

        std::map<OpaqueMaterial, OpaqueCall>    _opaqueCalls;

        OpaqueProgram                           _opaqueProgram;
        TexturePresentProgram                   _texturePresentProgram;

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

        [[nodiscard]] const VkExtent2D& GetResolution () const;

        [[nodiscard]] bool Init ( android_vulkan::Renderer &renderer, VkRenderPass presentRenderPass );
        void Destroy ( android_vulkan::Renderer &renderer );

        void SubmitMesh ( MeshRef &mesh, MaterialRef &material, GXMat4 const &local );

    private:
        [[nodiscard]] bool CreateGBufferFramebuffer ( android_vulkan::Renderer &renderer );
        [[nodiscard]] bool CreateGBufferRenderPass ( android_vulkan::Renderer &renderer );
        void SubmitOpaqueCall ( MeshRef &mesh, MaterialRef &material, GXMat4 const &local );
};

} // namespace pbr


#endif // RENDER_SESSION_H
