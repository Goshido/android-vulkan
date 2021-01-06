#ifndef PBR_SAMPLER_H
#define PBR_SAMPLER_H


#include <renderer.h>


namespace pbr {

class Sampler final
{
    private:
        VkSampler       _sampler;

    public:
        Sampler () noexcept;

        Sampler ( Sampler const & ) = delete;
        Sampler& operator = ( Sampler const & ) = delete;

        Sampler ( Sampler && ) = delete;
        Sampler& operator = ( Sampler && ) = delete;

        ~Sampler () = default;

        [[nodiscard]] bool Init ( const VkSamplerCreateInfo &info, android_vulkan::Renderer &renderer );
        void Destroy ( android_vulkan::Renderer &renderer );

        [[nodiscard]] VkSampler GetSampler () const;
};

} // namespace pbr


#endif // PBR_SAMPLER_H
