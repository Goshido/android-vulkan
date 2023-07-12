#ifndef PBR_SAMPLER_HPP
#define PBR_SAMPLER_HPP


#include <renderer.h>


namespace pbr {

class Sampler final
{
    private:
        VkSampler       _sampler = VK_NULL_HANDLE;

    public:
        Sampler () = default;

        Sampler ( Sampler const & ) = delete;
        Sampler &operator = ( Sampler const & ) = delete;

        Sampler ( Sampler && ) = delete;
        Sampler &operator = ( Sampler && ) = delete;

        ~Sampler () = default;

        [[nodiscard]] bool Init ( VkDevice device, VkSamplerCreateInfo const &info ) noexcept;
        void Destroy ( VkDevice device ) noexcept;

        [[nodiscard]] VkSampler GetSampler () const noexcept;
};

} // namespace pbr


#endif // PBR_SAMPLER_HPP
