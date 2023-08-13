#ifndef PBR_AVERAGE_BRIGHTNESS_PASS_HPP
#define PBR_AVERAGE_BRIGHTNESS_PASS_HPP


#include <texture2D.hpp>


namespace pbr {

class AverageBrightnessPass final
{
    private:
        struct Memory final
        {
            VkDeviceMemory                      _memory = VK_NULL_HANDLE;
            VkDeviceSize                        _offset = std::numeric_limits<VkDeviceSize>::max ();
        };

    private:
        VkCommandBuffer                         _commandBuffer = VK_NULL_HANDLE;

        [[maybe_unused]] VkExtent3D             _dispatch {};
        [[maybe_unused]] VkDescriptorPool       _descriptorPool = VK_NULL_HANDLE;
        [[maybe_unused]] VkDescriptorSet        _descriptorSet = VK_NULL_HANDLE;

        [[maybe_unused]] VkImage                _mips = VK_NULL_HANDLE;
        [[maybe_unused]] Memory                 _mipsMemory {};

        VkBuffer                                _globalCounter = VK_NULL_HANDLE;
        Memory                                  _globalCounterMemory {};

        VkBuffer                                _transferBuffer = VK_NULL_HANDLE;
        Memory                                  _transferBufferMemory {};

    public:
        AverageBrightnessPass () = default;

        AverageBrightnessPass ( AverageBrightnessPass const & ) = delete;
        AverageBrightnessPass &operator = ( AverageBrightnessPass const & ) = delete;

        AverageBrightnessPass ( AverageBrightnessPass && ) = delete;
        AverageBrightnessPass &operator = ( AverageBrightnessPass && ) = delete;

        ~AverageBrightnessPass () = default;

        void FreeTransferResources ( android_vulkan::Renderer &renderer, VkCommandPool commandPool ) noexcept;

        [[nodiscard]] bool Init ( android_vulkan::Renderer &renderer, VkCommandPool commandPool ) noexcept;
        void Destroy ( android_vulkan::Renderer &renderer ) noexcept;

        [[nodiscard]] bool SetTarget ( android_vulkan::Renderer &renderer,
            android_vulkan::Texture2D const &hdrImage
        ) noexcept;

        // This method makes sure that target resolution will be compatible with internal implementation.
        // Hint: Resolution should be multiple of 64 pixels and not bigger that 2048.
        [[nodiscard]] static VkExtent2D AdjustResolution ( VkExtent2D const &desiredResolution ) noexcept;

    private:
        [[nodiscard]] bool CreateGlobalCounter ( android_vulkan::Renderer &renderer,
            VkDevice device,
            VkCommandPool commandPool
        ) noexcept;

        [[nodiscard]] bool CreateMips ( android_vulkan::Renderer &renderer, VkDevice device ) noexcept;
        void FreeTransferBuffer ( android_vulkan::Renderer &renderer, VkDevice device ) noexcept;
};

} // namespace pbr


#endif // PBR_AVERAGE_BRIGHTNESS_PASS_HPP
