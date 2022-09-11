#ifndef MANDELBROT_LUT_COLOR_H
#define MANDELBROT_LUT_COLOR_H


#include "mandelbrot_base.h"


namespace mandelbrot {

class MandelbrotLUTColor final : public MandelbrotBase
{
    private:
        VkDescriptorPool            _descriptorPool;
        VkDescriptorSet             _descriptorSet;
        VkDescriptorSetLayout       _descriptorSetLayout;

        VkImage                     _lut;
        VkDeviceMemory              _lutDeviceMemory;
        VkImageView                 _lutView;

        VkSampler                   _sampler;

    public:
        MandelbrotLUTColor () noexcept;

        MandelbrotLUTColor ( MandelbrotLUTColor const & ) = delete;
        MandelbrotLUTColor& operator = ( MandelbrotLUTColor const & ) = delete;

        MandelbrotLUTColor ( MandelbrotLUTColor && ) = delete;
        MandelbrotLUTColor& operator = ( MandelbrotLUTColor && ) = delete;

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
        void DestroyLUT ( VkDevice device ) noexcept;

        [[nodiscard]] bool UploadLUTSamples ( android_vulkan::Renderer &renderer ) noexcept;

        static void InitLUTSamples ( uint8_t* samples ) noexcept;
};

} // namespace mandelbrot


#endif // MANDELBROT_LUT_COLOR_H
