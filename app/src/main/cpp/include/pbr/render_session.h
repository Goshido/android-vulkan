#ifndef PBR_RENDER_SESSION_H
#define PBR_RENDER_SESSION_H


#include "default_texture_manager.h"
#include "geometry_pass.h"
#include "light_pass.h"
#include "present_pass.h"
#include "reflection_global_pass.h"
#include "shadow_casters.h"


namespace pbr {

// Single threaded class
class RenderSession final
{
    private:
        using LightHandler = void ( RenderSession::* ) ( LightRef &light ) noexcept;

        using MeshHandler = void ( RenderSession::* ) ( MeshRef &mesh,
            MaterialRef const &material,
            GXMat4 const &local,
            GXAABB const &worldBounds,
            GXColorRGB const &color0,
            GXColorRGB const &color1,
            GXColorRGB const &color2,
            GXColorRGB const &emission
        ) noexcept;

    private:
        VkCommandPool                           _commandPool = VK_NULL_HANDLE;

        GXMat4                                  _cvvToView {};
        GXMat4                                  _view {};
        GXMat4                                  _viewProjection {};
        GXMat4                                  _viewerLocal {};

        DefaultTextureManager                   _defaultTextureManager {};
        GXProjectionClipPlanes                  _frustum {};

        GBuffer                                 _gBuffer {};
        VkDescriptorPool                        _gBufferDescriptorPool = VK_NULL_HANDLE;
        VkFramebuffer                           _gBufferFramebuffer = VK_NULL_HANDLE;
        VkImageMemoryBarrier                    _gBufferImageBarrier {};
        VkRenderPass                            _gBufferRenderPass = VK_NULL_HANDLE;
        VkDescriptorSet                         _gBufferSlotMapper = VK_NULL_HANDLE;

        GeometryPass                            _geometryPass {};

        LightHandler                            _lightHandlers[ 3U ] {};
        LightPass                               _lightPass {};

        MeshHandler                             _meshHandlers[ 2U ] {};
        size_t                                  _opaqueMeshCount = 0U;

        PresentPass                             _presentPass {};
        RenderSessionStats                      _renderSessionStats {};
        SamplerManager                          _samplerManager {};
        TexturePresentDescriptorSetLayout       _texturePresentDescriptorSetLayout {};

    public:
        RenderSession () = default;

        RenderSession ( RenderSession const & ) = delete;
        RenderSession& operator = ( RenderSession const & ) = delete;

        RenderSession ( RenderSession && ) = delete;
        RenderSession& operator = ( RenderSession && ) = delete;

        ~RenderSession () = default;

        void Begin ( GXMat4 const &viewerLocal, GXMat4 const &projection ) noexcept;
        [[nodiscard]] bool End ( android_vulkan::Renderer &renderer, double deltaTime ) noexcept;

        void FreeTransferResources ( VkDevice device ) noexcept;

        [[nodiscard]] bool OnInitDevice ( android_vulkan::Renderer &renderer ) noexcept;
        void OnDestroyDevice ( VkDevice device ) noexcept;

        [[nodiscard]] bool OnSwapchainCreated ( android_vulkan::Renderer &renderer,
            VkExtent2D const &resolution
        ) noexcept;

        void OnSwapchainDestroyed ( VkDevice device ) noexcept;

        void SubmitLight ( LightRef &light ) noexcept;

        void SubmitMesh ( MeshRef &mesh,
            MaterialRef const &material,
            GXMat4 const &local,
            GXAABB const &worldBounds,
            GXColorRGB const &color0,
            GXColorRGB const &color1,
            GXColorRGB const &color2,
            GXColorRGB const &emission
        ) noexcept;

    private:
        [[nodiscard]] bool CreateGBufferFramebuffer ( android_vulkan::Renderer &renderer ) noexcept;
        [[nodiscard]] bool CreateGBufferRenderPass ( android_vulkan::Renderer &renderer ) noexcept;

        [[nodiscard]] bool CreateGBufferResources ( android_vulkan::Renderer &renderer,
            VkExtent2D const &resolution
        ) noexcept;

        [[nodiscard]] bool CreateGBufferSlotMapper ( android_vulkan::Renderer &renderer ) noexcept;

        void DestroyGBufferResources ( VkDevice device ) noexcept;

        void SubmitOpaqueCall ( MeshRef &mesh,
            MaterialRef const &material,
            GXMat4 const &local,
            GXAABB const &worldBounds,
            GXColorRGB const &color0,
            GXColorRGB const &color1,
            GXColorRGB const &color2,
            GXColorRGB const &emission
        ) noexcept;

        void SubmitStippleCall ( MeshRef &mesh,
            MaterialRef const &material,
            GXMat4 const &local,
            GXAABB const &worldBounds,
            GXColorRGB const &color0,
            GXColorRGB const &color1,
            GXColorRGB const &color2,
            GXColorRGB const &emission
        ) noexcept;

        void SubmitPointLight ( LightRef &light ) noexcept;
        void SubmitReflectionGlobal ( LightRef &light ) noexcept;
        void SubmitReflectionLocal ( LightRef &light ) noexcept;
};

} // namespace pbr


#endif // PBR_RENDER_SESSION_H
