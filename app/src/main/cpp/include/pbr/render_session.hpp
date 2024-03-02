#ifndef PBR_RENDER_SESSION_HPP
#define PBR_RENDER_SESSION_HPP


#include "command_buffer_count.hpp"
#include "default_texture_manager.hpp"
#include "exposure_pass.hpp"
#include "geometry_pass.hpp"
#include "light_pass.hpp"
#include "present_render_pass.hpp"
#include "reflection_global_pass.hpp"
#include "shadow_casters.hpp"
#include "tone_mapper_pass.hpp"
#include "ui_pass.hpp"


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
            VkCommandBuffer         _buffer = VK_NULL_HANDLE;
            VkFence                 _fence = VK_NULL_HANDLE;
            VkCommandPool           _pool = VK_NULL_HANDLE;
            VkSemaphore             _acquire = VK_NULL_HANDLE;
        };

    private:
        float                       _brightnessBalance = 0.0F;
        bool                        _brightnessChanged = false;

        CommandInfo                 _commandInfo[ DUAL_COMMAND_BUFFER ];
        size_t                      _writingCommandInfo = 0U;

        GXMat4                      _cvvToView {};
        GXMat4                      _view {};
        GXMat4                      _viewProjection {};
        GXMat4                      _viewerLocal {};

        DefaultTextureManager       _defaultTextureManager {};
        ExposurePass                _exposurePass {};
        GXProjectionClipPlanes      _frustum {};

        GBuffer                     _gBuffer {};
        GeometryPass                _geometryPass {};

        LightHandler                _lightHandlers[ 3U ] {};
        LightPass                   _lightPass {};

        MeshHandler                 _meshHandlers[ 2U ] {};
        size_t                      _opaqueMeshCount = 0U;

        PresentRenderPass           _presentRenderPass {};

        VkRenderPassBeginInfo       _renderPassInfo {};
        RenderSessionStats          _renderSessionStats {};
        SamplerManager              _samplerManager {};
        ToneMapperPass              _toneMapperPass {};

        UIPass                      _uiPass {};

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
        [[nodiscard]] size_t GetWritingCommandBufferIndex () const noexcept;

        [[nodiscard]] bool OnInitDevice ( android_vulkan::Renderer &renderer ) noexcept;
        void OnDestroyDevice ( android_vulkan::Renderer &renderer ) noexcept;

        [[nodiscard]] bool OnSwapchainCreated ( android_vulkan::Renderer &renderer,
            VkExtent2D const &resolution
        ) noexcept;

        void OnSwapchainDestroyed ( VkDevice device ) noexcept;

        // Brightness balance should be in range [-1.0F, 1.0F].
        void SetBrightness ( float brightnessBalance ) noexcept;

        // More about exposure value:
        // See <repo>/docs/auto-exposure.md
        void SetExposureCompensation ( float exposureValue ) noexcept;
        void SetExposureMaximumBrightness ( float exposureValue ) noexcept;
        void SetExposureMinimumBrightness ( float exposureValue ) noexcept;

        void SetEyeAdaptationSpeed ( float speed ) noexcept;
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
        [[nodiscard]] bool CreateFramebuffer ( VkDevice device ) noexcept;
        [[nodiscard]] bool CreateRenderPass ( VkDevice device ) noexcept;

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

        [[nodiscard]] bool UpdateBrightness ( android_vulkan::Renderer &renderer ) noexcept;

        [[nodiscard]] static bool AllocateCommandInfo ( CommandInfo &info,
            VkDevice device,
            uint32_t queueIndex,
            size_t frameInFlightIndex
        ) noexcept;
};

} // namespace pbr


#endif // PBR_RENDER_SESSION_HPP
