#ifndef RENDER_SESSION_H
#define RENDER_SESSION_H


#include <GXCommon/GXMath.h>
#include "gbuffer.h"
#include "opaque_program.h"
#include "texture_present_program.h"
#include "types.h"


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
        Texture2DRef                _albedoDefault;
        Texture2DRef                _emissionDefault;
        Texture2DRef                _normalDefault;
        Texture2DRef                _paramDefault;

        VkCommandPool               _commandPool;

        GBuffer                     _gBuffer;
        VkFramebuffer               _gBufferFramebuffer;
        VkRenderPass                _gBufferRenderPass;

        bool                        _isFreeTransferResources;

        size_t                      _meshCount;

        OpaqueProgram               _opaqueProgram;
        TexturePresentProgram       _texturePresentProgram;

        GXMat4                      _viewProjection;

    public:
        RenderSession ();
        ~RenderSession () = default;

        RenderSession ( const RenderSession &other ) = delete;
        RenderSession& operator = ( const RenderSession &other ) = delete;

        [[maybe_unused]] void AddMesh ( MeshRef &mesh, MaterialRef &material, const GXMat4 &local );

        [[maybe_unused]] void Begin ( const GXMat4 &view, const GXMat4 &projection );
        [[maybe_unused]] void End ( ePresentTarget target, android_vulkan::Renderer &renderer );

        [[maybe_unused]] [[nodiscard]] bool Init ( android_vulkan::Renderer &renderer, VkRenderPass presentRenderPass );
        void Destroy ( android_vulkan::Renderer &renderer );

    private:
        [[nodiscard]] bool CreateGBufferFramebuffer ( android_vulkan::Renderer &renderer );
        [[nodiscard]] bool CreateGBufferRenderPass ( android_vulkan::Renderer &renderer );
};

} // namespace pbr


#endif // RENDER_SESSION_H
