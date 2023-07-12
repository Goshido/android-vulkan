#ifndef PBR_GBUFFER_HPP
#define PBR_GBUFFER_HPP


#include <texture2D.h>


namespace pbr {

class GBuffer final
{
    private:
        android_vulkan::Texture2D       _albedo {};
        android_vulkan::Texture2D       _depthStencil {};
        android_vulkan::Texture2D       _hdrAccumulator {};
        android_vulkan::Texture2D       _normal {};
        android_vulkan::Texture2D       _params {};

    public:
        GBuffer () = default;

        GBuffer ( GBuffer const & ) = delete;
        GBuffer &operator = ( GBuffer const & ) = delete;

        GBuffer ( GBuffer && ) = delete;
        GBuffer &operator = ( GBuffer && ) = delete;

        ~GBuffer () = default;

        [[nodiscard]] android_vulkan::Texture2D &GetAlbedo ()  noexcept;
        [[nodiscard]] android_vulkan::Texture2D &GetDepthStencil () noexcept;
        [[nodiscard]] android_vulkan::Texture2D &GetHDRAccumulator () noexcept;
        [[nodiscard]] android_vulkan::Texture2D &GetNormal () noexcept;
        [[nodiscard]] android_vulkan::Texture2D &GetParams () noexcept;

        [[nodiscard]] VkExtent2D const &GetResolution () const noexcept;

        [[nodiscard]] bool Init ( android_vulkan::Renderer &renderer, VkExtent2D const &resolution ) noexcept;
        void Destroy ( android_vulkan::Renderer &renderer ) noexcept;
};

} // namespace pbr


#endif // PBR_GBUFFER_HPP
