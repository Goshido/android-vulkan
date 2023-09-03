#ifndef PBR_SAMPLER_MANAGER_HPP
#define PBR_SAMPLER_MANAGER_HPP


#include "sampler.hpp"


namespace pbr {

class SamplerManager final
{
    private:
        Sampler     _clampToEdgeSampler {};
        Sampler     _pointSampler {};
        Sampler     _materialSampler {};

    public:
        SamplerManager () = default;

        SamplerManager ( SamplerManager const & ) = delete;
        SamplerManager &operator = ( SamplerManager const & ) = delete;

        SamplerManager ( SamplerManager && ) = delete;
        SamplerManager &operator = ( SamplerManager && ) = delete;

        ~SamplerManager () = default;

        [[nodiscard]] bool Init ( VkDevice device ) noexcept;
        void Destroy ( VkDevice device ) noexcept;

        [[nodiscard]] VkSampler GetClampToEdgeSampler () const noexcept;
        [[nodiscard]] VkSampler GetMaterialSampler () const noexcept;
        [[nodiscard]] VkSampler GetPointSampler () const noexcept;
};

} // namespace pbr


#endif // PBR_SAMPLER_MANAGER_HPP
