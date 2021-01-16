#ifndef PBR_RENDER_SESSION_H
#define PBR_RENDER_SESSION_H


#include <GXCommon/GXMath.h>
#include "gbuffer.h"
#include "geometry_pass.h"
#include "lightup_common_descriptor_set.h"
#include "point_light_pass.h"
#include "present_pass.h"
#include "shadow_casters.h"


namespace pbr {

enum class ePresentTarget : uint8_t
{
    Albedo = 0U,
    Emission = 1U,
    Normal = 2U,
    Param = 3U,
    TargetCount = 4U
};

// Single threaded class
class RenderSession final
{
    private:
        GXProjectionClipPlanes          _frustum;

        GBuffer                         _gBuffer;
        VkDescriptorPool                _gBufferDescriptorPool;
        VkFramebuffer                   _gBufferFramebuffer;
        VkImageMemoryBarrier            _gBufferImageBarrier;
        VkRenderPass                    _gBufferRenderPass;
        VkDescriptorSet                 _gBufferSlotMapper[ static_cast<size_t> ( ePresentTarget::TargetCount ) ];

        GeometryPass                    _geometryPass;
        LightVolume                     _lightVolume;
        LightupCommonDescriptorSet      _lightupCommonDescriptorSet;
        size_t                          _opaqueMeshCount;

        TexturePresentProgram           _texturePresentProgram;
        PointLightPass                  _pointLightPass;
        PresentPass                     _presentPass;
        RenderSessionStats              _renderSessionStats;
        SamplerManager                  _samplerManager;

        GXMat4                          _cvvToView;
        GXMat4                          _view;
        GXMat4                          _viewProjection;
        GXMat4                          _viewerLocal;

    public:
        RenderSession () noexcept;

        RenderSession ( RenderSession const & ) = delete;
        RenderSession& operator = ( RenderSession const & ) = delete;

        RenderSession ( RenderSession && ) = delete;
        RenderSession& operator = ( RenderSession && ) = delete;

        ~RenderSession () = default;

        void Begin ( GXMat4 const &viewerLocal, GXMat4 const &projection );
        [[nodiscard]] bool End ( android_vulkan::Renderer &renderer, ePresentTarget target, double deltaTime );

        [[nodiscard]] bool Init ( android_vulkan::Renderer &renderer, VkExtent2D const &resolution );
        void Destroy ( VkDevice device );

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
        [[nodiscard]] bool CreateGBufferFramebuffer ( android_vulkan::Renderer &renderer );
        [[nodiscard]] bool CreateGBufferRenderPass ( android_vulkan::Renderer &renderer );
        [[nodiscard]] bool CreateGBufferSlotMapper ( android_vulkan::Renderer &renderer );

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
};

} // namespace pbr


#endif // PBR_RENDER_SESSION_H
