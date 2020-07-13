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
    public:
        Texture2DRef                            _albedoDefault;
        Texture2DRef                            _emissionDefault;
        Texture2DRef                            _normalDefault;
        Texture2DRef                            _paramDefault;

        VkCommandPool                           _commandPool;

        GBuffer                                 _gBuffer;
        VkFramebuffer                           _gBufferFramebuffer;
        VkRenderPass                            _gBufferRenderPass;

        bool                                    _isFreeTransferResources;

        size_t                                  _meshCount;

        std::map<OpaqueMaterial, OpaqueCall>    _opaqueCalls;

        OpaqueProgram                           _opaqueProgram;
        TexturePresentProgram                   _texturePresentProgram;

        GXMat4                                  _viewProjection;

    public:
        RenderSession ();
        ~RenderSession () = default;

        RenderSession ( const RenderSession &other ) = delete;
        RenderSession& operator = ( const RenderSession &other ) = delete;

        void Begin ( const GXMat4 &view, const GXMat4 &projection );
        void End ( ePresentTarget target, android_vulkan::Renderer &renderer );

        [[nodiscard]] const VkExtent2D& GetResolution () const;

        [[nodiscard]] bool Init ( android_vulkan::Renderer &renderer, VkRenderPass presentRenderPass );
        void Destroy ( android_vulkan::Renderer &renderer );

        void SubmitMesh ( MeshRef &mesh, MaterialRef &material, const GXMat4 &local );

    private:
        [[nodiscard]] bool CreateGBufferFramebuffer ( android_vulkan::Renderer &renderer );
        [[nodiscard]] bool CreateGBufferRenderPass ( android_vulkan::Renderer &renderer );
        void SubmitOpaqueCall ( MeshRef &mesh, MaterialRef &material, const GXMat4 &local );
};

} // namespace pbr


#endif // RENDER_SESSION_H
