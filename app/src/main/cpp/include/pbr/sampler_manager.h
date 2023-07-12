#ifndef PBR_SAMPLER_MANAGER_H
#define PBR_SAMPLER_MANAGER_H


#include "types.h"


namespace pbr {

class SamplerManager final
{
    private:
        SamplerRef      _pointSampler {};
        SamplerRef      _materialSampler {};

    public:
        SamplerManager () = default;

        SamplerManager ( SamplerManager const & ) = delete;
        SamplerManager &operator = ( SamplerManager const & ) = delete;

        SamplerManager ( SamplerManager && ) = delete;
        SamplerManager &operator = ( SamplerManager && ) = delete;

        ~SamplerManager () = default;

        [[nodiscard]] bool Init ( VkDevice device ) noexcept;
        void Destroy ( VkDevice device ) noexcept;

        [[nodiscard]] SamplerRef const &GetMaterialSampler () const noexcept;
        [[nodiscard]] SamplerRef const &GetPointSampler () const noexcept;
};

} // namespace pbr


#endif // PBR_SAMPLER_MANAGER_H
