#ifndef GBUFFER_H
#define GBUFFER_H


#include <texture2D.h>


namespace pbr {

class GBuffer final
{
    private:
        android_vulkan::Texture2D       _albedo;
        android_vulkan::Texture2D       _emission;
        android_vulkan::Texture2D       _hdrAccumulator;
        android_vulkan::Texture2D       _normal;
        android_vulkan::Texture2D       _params;

        android_vulkan::Texture2D       _depthStencil;

        VkExtent2D                      _resolution;

    public:
        GBuffer ();
        ~GBuffer () = default;

        GBuffer ( const GBuffer &other ) = delete;
        GBuffer& operator = ( const GBuffer &other ) = delete;

        [[nodiscard]] android_vulkan::Texture2D& GetAlbedo ();
        [[nodiscard]] android_vulkan::Texture2D& GetEmission ();
        [[maybe_unused]] [[nodiscard]] android_vulkan::Texture2D& GetHDRAccumulator ();
        [[nodiscard]] android_vulkan::Texture2D& GetNormal ();
        [[nodiscard]] android_vulkan::Texture2D& GetParams ();
        [[nodiscard]] android_vulkan::Texture2D& GetDepthStencil ();

        [[nodiscard]] VkExtent2D const& GetResolution () const;

        [[nodiscard]] bool Init ( VkExtent2D const &resolution, android_vulkan::Renderer &renderer );
        void Destroy ( android_vulkan::Renderer &renderer );
};

} // namespace pbr


#endif // GBUFFER_H
