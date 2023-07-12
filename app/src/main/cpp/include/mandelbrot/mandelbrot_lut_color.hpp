#ifndef MANDELBROT_LUT_COLOR_HPP
#define MANDELBROT_LUT_COLOR_HPP


#include "mandelbrot_base.h"


namespace mandelbrot {

class MandelbrotLUTColor final : public MandelbrotBase
{
    private:
        VkDescriptorPool            _descriptorPool = VK_NULL_HANDLE;
        VkDescriptorSet             _descriptorSet = VK_NULL_HANDLE;
        VkDescriptorSetLayout       _descriptorSetLayout = VK_NULL_HANDLE;

        VkImage                     _lut = VK_NULL_HANDLE;
        VkDeviceMemory              _lutMemory = VK_NULL_HANDLE;
        VkDeviceSize                _lutOffset = std::numeric_limits<VkDeviceSize>::max ();

        VkImageView                 _lutView = VK_NULL_HANDLE;
        VkSampler                   _sampler = VK_NULL_HANDLE;

    public:
        MandelbrotLUTColor () noexcept;

        MandelbrotLUTColor ( MandelbrotLUTColor const & ) = delete;
        MandelbrotLUTColor &operator = ( MandelbrotLUTColor const & ) = delete;

        MandelbrotLUTColor ( MandelbrotLUTColor && ) = delete;
        MandelbrotLUTColor &operator = ( MandelbrotLUTColor && ) = delete;

        ~MandelbrotLUTColor () override = default;

    private:
        [[nodiscard]] bool OnInitDevice ( android_vulkan::Renderer &renderer ) noexcept override;
        void OnDestroyDevice ( android_vulkan::Renderer &renderer ) noexcept override;

        [[nodiscard]] bool OnSwapchainCreated ( android_vulkan::Renderer &renderer ) noexcept override;
        void OnSwapchainDestroyed ( android_vulkan::Renderer &renderer ) noexcept override;

        [[nodiscard]] bool CreatePipelineLayout ( android_vulkan::Renderer &renderer ) noexcept override;
        void DestroyPipelineLayout ( VkDevice device ) noexcept override;

        [[nodiscard]] bool CreateCommandBuffer ( android_vulkan::Renderer &renderer ) noexcept;
        void DestroyCommandBuffer ( VkDevice device ) noexcept;

        [[nodiscard]] bool CreateDescriptorSet ( android_vulkan::Renderer &renderer ) noexcept;
        void DestroyDescriptorSet ( VkDevice device ) noexcept;

        [[nodiscard]] bool CreateLUT ( android_vulkan::Renderer &renderer ) noexcept;
        void DestroyLUT ( android_vulkan::Renderer &renderer  ) noexcept;

        [[nodiscard]] bool UploadLUTSamples ( android_vulkan::Renderer &renderer ) noexcept;

        static void InitLUTSamples ( uint8_t* samples ) noexcept;
};

} // namespace mandelbrot


#endif // MANDELBROT_LUT_COLOR_HPP
