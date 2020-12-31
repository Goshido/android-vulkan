#ifndef PBR_SAMPLER_MANAGER_H
#define PBR_SAMPLER_MANAGER_H


#include "types.h"


namespace pbr {

class SamplerManager final
{
    private:
        // 1 2 4 8 16 32 64 128 256 512 1024 2048 4096
        constexpr static size_t const MAX_SUPPORTED_MIP_COUNT = 13U;

    private:
        SamplerRef      _pointSampler;
        SamplerRef      _storage[ MAX_SUPPORTED_MIP_COUNT ];

    public:
        SamplerManager () noexcept = default;

        SamplerManager ( SamplerManager const & ) = delete;
        SamplerManager& operator = ( SamplerManager const & ) = delete;

        SamplerManager ( SamplerManager && ) = delete;
        SamplerManager& operator = ( SamplerManager && ) = delete;

        ~SamplerManager () = default;

        void FreeResources ( android_vulkan::Renderer &renderer );

        [[nodiscard]] SamplerRef GetPointSampler ( android_vulkan::Renderer &renderer );
        [[nodiscard]] SamplerRef GetSampler ( uint8_t mips, android_vulkan::Renderer &renderer );
};

} // namespace pbr


#endif // PBR_SAMPLER_MANAGER_H
