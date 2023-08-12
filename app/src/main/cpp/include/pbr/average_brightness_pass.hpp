#ifndef PBR_AVERAGE_BRIGHTNESS_PASS_HPP
#define PBR_AVERAGE_BRIGHTNESS_PASS_HPP


#include <texture2D.hpp>


namespace pbr {

class AverageBrightnessPass final
{
    private:
        struct Memory final
        {
            [[maybe_unused]] VkDeviceMemory     _memory = VK_NULL_HANDLE;
            VkDeviceSize                        _offset = 0U;
        };

    private:
        [[maybe_unused]] VkExtent3D             _dispatch {};
        [[maybe_unused]] VkDescriptorPool       _descriptorPool = VK_NULL_HANDLE;
        [[maybe_unused]] VkDescriptorSet        _descriptorSet = VK_NULL_HANDLE;

        [[maybe_unused]] VkImage                _mips = VK_NULL_HANDLE;
        [[maybe_unused]] Memory                 _mipsMemory {};

        [[maybe_unused]] VkBuffer               _globalCounter = VK_NULL_HANDLE;
        [[maybe_unused]] Memory                 _globalCounterMemory {};

    public:
        AverageBrightnessPass () = default;

        AverageBrightnessPass ( AverageBrightnessPass const & ) = delete;
        AverageBrightnessPass &operator = ( AverageBrightnessPass const & ) = delete;

        AverageBrightnessPass ( AverageBrightnessPass && ) = delete;
        AverageBrightnessPass &operator = ( AverageBrightnessPass && ) = delete;

        ~AverageBrightnessPass () = default;

        [[nodiscard]] bool Init ( android_vulkan::Renderer &renderer ) noexcept;
        void Destroy ( android_vulkan::Renderer &renderer ) noexcept;

        [[nodiscard]] bool SetTarget ( android_vulkan::Renderer &renderer,
            android_vulkan::Texture2D const &hdrImage ) noexcept;

        // This method makes sure that target resolution will be compatible with internal implementation.
        // Resolution should be multiple of 64 pixels and not bigger that 2048.
        [[nodiscard]] static VkExtent2D AdjustResolution ( VkExtent2D const &desiredResolution ) noexcept;
};

} // namespace pbr


#endif // PBR_AVERAGE_BRIGHTNESS_PASS_HPP
