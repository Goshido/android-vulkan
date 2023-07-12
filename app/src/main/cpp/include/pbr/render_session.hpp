#ifndef PBR_RENDER_SESSION_HPP
#define PBR_RENDER_SESSION_HPP


#include "default_texture_manager.h"
#include "geometry_pass.h"
#include "light_pass.h"
#include "reflection_global_pass.h"
#include "shadow_casters.h"
#include "ui_pass.h"


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

        struct CommandInfo final
        {
            VkCommandBuffer                     _buffer = VK_NULL_HANDLE;
            VkFence                             _fence = VK_NULL_HANDLE;
            VkCommandPool                       _pool = VK_NULL_HANDLE;
        };

    private:
        std::vector<CommandInfo>                _commandInfo {};

        GXMat4                                  _cvvToView {};
        GXMat4                                  _view {};
        GXMat4                                  _viewProjection {};
        GXMat4                                  _viewerLocal {};

        DefaultTextureManager                   _defaultTextureManager {};
        VkFramebuffer                           _framebuffer = VK_NULL_HANDLE;
        GXProjectionClipPlanes                  _frustum {};

        GBuffer                                 _gBuffer {};
        GeometryPass                            _geometryPass {};

        LightHandler                            _lightHandlers[ 3U ] {};
        LightPass                               _lightPass {};

        MeshHandler                             _meshHandlers[ 2U ] {};
        size_t                                  _opaqueMeshCount = 0U;

        VkRenderPass                            _renderPass = VK_NULL_HANDLE;
        VkRenderPassBeginInfo                   _renderPassInfo {};
        RenderSessionStats                      _renderSessionStats {};
        SamplerManager                          _samplerManager {};

        UIPass                                  _uiPass;

    public:
        RenderSession () = default;

        RenderSession ( RenderSession const & ) = delete;
        RenderSession &operator = ( RenderSession const & ) = delete;

        RenderSession ( RenderSession && ) = delete;
        RenderSession &operator = ( RenderSession && ) = delete;

        ~RenderSession () = default;

        void Begin ( GXMat4 const &viewerLocal, GXMat4 const &projection ) noexcept;
        [[nodiscard]] bool End ( android_vulkan::Renderer &renderer, double deltaTime ) noexcept;

        void FreeTransferResources ( android_vulkan::Renderer &renderer ) noexcept;
        [[nodiscard]] UIPass &GetUIPass () noexcept;

        [[nodiscard]] bool OnInitDevice ( android_vulkan::Renderer &renderer ) noexcept;
        void OnDestroyDevice ( android_vulkan::Renderer &renderer ) noexcept;

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
        [[nodiscard]] bool CreateFramebuffer ( android_vulkan::Renderer &renderer ) noexcept;
        [[nodiscard]] bool CreateRenderPass ( android_vulkan::Renderer &renderer ) noexcept;

        [[nodiscard]] bool CreateGBufferResources ( android_vulkan::Renderer &renderer,
            VkExtent2D const &resolution
        ) noexcept;

        void DestroyGBufferResources ( android_vulkan::Renderer &renderer ) noexcept;

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

        [[nodiscard]] static bool AllocateCommandInfo ( CommandInfo &info,
            VkDevice device,
            uint32_t queueIndex
        ) noexcept;
};

} // namespace pbr


#endif // PBR_RENDER_SESSION_HPP
