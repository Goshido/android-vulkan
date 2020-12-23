#ifndef PBR_SAMPLER_H
#define PBR_SAMPLER_H


#include <renderer.h>


namespace pbr {

class Sampler final
{
    private:
        VkSamplerCreateInfo     _info;
        VkSampler               _sampler;

    public:
        Sampler ();
        ~Sampler () = default;

        Sampler ( const Sampler &other ) = delete;
        Sampler& operator = ( const Sampler &other ) = delete;

        [[nodiscard]] bool Init ( const VkSamplerCreateInfo &info,
            android_vulkan::Renderer &renderer
        );

        void Destroy ( android_vulkan::Renderer &renderer );
        [[nodiscard]] VkSampler GetSampler () const;
};


} // namespace pbr


#endif // PBR_SAMPLER_H
