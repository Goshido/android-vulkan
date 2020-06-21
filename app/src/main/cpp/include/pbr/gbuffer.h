#ifndef GBUFFER_H
#define GBUFFER_H


#include <texture2D.h>


namespace pbr {

class GBuffer final
{
    private:
        android_vulkan::Texture2D       _albedo;
        android_vulkan::Texture2D       _emission;
        android_vulkan::Texture2D       _normal;
        android_vulkan::Texture2D       _params;

        android_vulkan::Texture2D       _depthStencil;

        VkExtent2D                      _resolution;

    public:
        GBuffer ();
        ~GBuffer () = default;

        GBuffer ( const GBuffer &other ) = delete;
        GBuffer& operator = ( const GBuffer &other ) = delete;

        [[nodiscard]] bool Init ( const VkExtent2D &resolution, android_vulkan::Renderer &renderer );
        void Destroy ( android_vulkan::Renderer &renderer );

        [[nodiscard]] [[maybe_unused]] const VkExtent2D& GetResolution () const;
};

} // namespace pbr


#endif // GBUFFER_H