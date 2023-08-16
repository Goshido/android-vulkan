#ifndef PBR_AVERAGE_BRIGHTNESS_PASS_HPP
#define PBR_AVERAGE_BRIGHTNESS_PASS_HPP


#include "average_brightness_descriptor_set_layout.hpp"
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
        VkDescriptorPool                        _descriptorPool = VK_NULL_HANDLE;
        VkDescriptorSet                         _descriptorSet = VK_NULL_HANDLE;

        VkImage                                 _mips = VK_NULL_HANDLE;
        Memory                                  _mipsMemory {};
        VkImageView                             _mipView = VK_NULL_HANDLE;
        VkImageView                             _syncMip5 = VK_NULL_HANDLE;

        VkBuffer                                _globalCounter = VK_NULL_HANDLE;
        Memory                                  _globalCounterMemory {};

        AverageBrightnessDescriptorSetLayout    _layout {};

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
        [[nodiscard]] bool CreateDescriptorSet ( VkDevice device ) noexcept;

        [[nodiscard]] bool CreateGlobalCounter ( android_vulkan::Renderer &renderer,
            VkDevice device,
            VkCommandPool commandPool
        ) noexcept;

        void BindTargetToDescriptorSet ( VkDevice device, android_vulkan::Texture2D const &hdrImage ) noexcept;

        [[nodiscard]] bool CreateMips ( android_vulkan::Renderer &renderer,
            VkDevice device,
            VkExtent2D resolution
        ) noexcept;

        void FreeTransferBuffer ( android_vulkan::Renderer &renderer, VkDevice device ) noexcept;
};

} // namespace pbr


#endif // PBR_AVERAGE_BRIGHTNESS_PASS_HPP
