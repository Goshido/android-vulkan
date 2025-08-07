#ifndef PBR_SAMPLER_HPP
#define PBR_SAMPLER_HPP


#include <GXCommon/GXWarning.hpp>

GX_DISABLE_COMMON_WARNINGS

#include <vulkan/vulkan_core.h>

GX_RESTORE_WARNING_STATE


namespace pbr {

class Sampler final
{
    private:
        VkSampler       _sampler = VK_NULL_HANDLE;

    public:
        explicit Sampler () = default;

        Sampler ( Sampler const & ) = delete;
        Sampler &operator = ( Sampler const & ) = delete;

        Sampler ( Sampler && ) = delete;
        Sampler &operator = ( Sampler && ) = delete;

        ~Sampler () = default;

        [[nodiscard]] bool Init ( VkDevice device, VkSamplerCreateInfo const &info, char const* name ) noexcept;
        void Destroy ( VkDevice device ) noexcept;

        [[nodiscard]] VkSampler const &GetSampler () const noexcept;
};

} // namespace pbr


#endif // PBR_SAMPLER_HPP
