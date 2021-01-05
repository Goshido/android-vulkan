#ifndef PBR_GBUFFER_H
#define PBR_GBUFFER_H


#include <texture2D.h>


namespace pbr {

class GBuffer final
{
    private:
        android_vulkan::Texture2D       _albedo;
        android_vulkan::Texture2D       _depthStencil;
        android_vulkan::Texture2D       _hdrAccumulator;
        android_vulkan::Texture2D       _normal;
        android_vulkan::Texture2D       _params;

    public:
        GBuffer ();

        GBuffer ( GBuffer const & ) = delete;
        GBuffer& operator = ( GBuffer const & ) = delete;

        GBuffer ( GBuffer && ) = delete;
        GBuffer& operator = ( GBuffer && ) = delete;

        ~GBuffer () = default;

        [[nodiscard]] android_vulkan::Texture2D& GetAlbedo () ;
        [[nodiscard]] android_vulkan::Texture2D& GetDepthStencil ();
        [[nodiscard]] android_vulkan::Texture2D& GetHDRAccumulator ();
        [[nodiscard]] android_vulkan::Texture2D& GetNormal ();
        [[nodiscard]] android_vulkan::Texture2D& GetParams ();

        [[nodiscard]] VkExtent2D const& GetResolution () const;

        [[nodiscard]] bool Init ( VkExtent2D const &resolution, android_vulkan::Renderer &renderer );
        void Destroy ( android_vulkan::Renderer &renderer );
};

} // namespace pbr


#endif // PBR_GBUFFER_H
