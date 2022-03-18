#ifndef PBR_GEOMETRY_PASS_H
#define PBR_GEOMETRY_PASS_H


#include <renderer.h>
#include "opaque_pass.h"
#include "stipple_pass.h"

namespace pbr {

class GeometryPass final
{
    private:
        VkCommandBuffer             _commandBuffer = VK_NULL_HANDLE;
        VkFence                     _fence = VK_NULL_HANDLE;

        OpaquePass                  _opaquePass {};
        StipplePass                 _stipplePass {};

        VkRenderPassBeginInfo       _renderPassInfo {};

    public:
        GeometryPass () = default;

        GeometryPass ( GeometryPass const & ) = delete;
        GeometryPass& operator = ( GeometryPass const & ) = delete;

        GeometryPass ( GeometryPass && ) = delete;
        GeometryPass& operator = ( GeometryPass && ) = delete;

        ~GeometryPass () = default;

        [[nodiscard]] bool Init ( android_vulkan::Renderer &renderer,
            VkCommandPool commandPool,
            VkExtent2D const &resolution,
            VkRenderPass renderPass,
            VkFramebuffer framebuffer
        ) noexcept;

        void Destroy ( VkDevice device ) noexcept;

        // The method returns command buffer in recording state if success. Otherwise the method returns VK_NULL_HANDLE.
        [[nodiscard]] VkCommandBuffer Execute ( android_vulkan::Renderer &renderer,
            GXProjectionClipPlanes const &frustum,
            GXMat4 const &view,
            GXMat4 const &viewProjection,
            DefaultTextureManager const &defaultTextureManager,
            SamplerManager &samplerManager,
            RenderSessionStats &renderSessionStats
        ) noexcept;

        [[nodiscard]] VkFence GetFence () const noexcept;
        [[nodiscard]] OpaquePass& GetOpaquePass () noexcept;
        [[nodiscard]] StipplePass& GetStipplePass () noexcept;

        void Reset () noexcept;

        // Return value optimization friendly method.
        [[nodiscard]] static VkSubpassDescription GetSubpassDescription () noexcept;

    private:
        [[nodiscard]] bool BeginRenderPass ( android_vulkan::Renderer &renderer ) noexcept;
};

} // namespace pbr


#endif // PBR_GEOMETRY_PASS_H
