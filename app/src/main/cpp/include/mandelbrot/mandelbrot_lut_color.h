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
        [[nodiscard]] bool OnInitDevice ( android_vulkan::Renderer &renderer ) override;
        void OnDestroyDevice ( VkDevice device ) override;

        [[nodiscard]] bool OnSwapchainCreated ( android_vulkan::Renderer &renderer ) override;
        void OnSwapchainDestroyed ( VkDevice device ) override;

        [[nodiscard]] bool CreatePipelineLayout ( android_vulkan::Renderer &renderer ) override;
        void DestroyPipelineLayout ( VkDevice device ) override;

        [[nodiscard]] bool CreateCommandBuffer ( android_vulkan::Renderer &renderer );
        void DestroyCommandBuffer ( VkDevice device );

        [[nodiscard]] bool CreateDescriptorSet ( android_vulkan::Renderer &renderer );
        void DestroyDescriptorSet ( VkDevice device );

        [[nodiscard]] bool CreateLUT ( android_vulkan::Renderer &renderer );
        void DestroyLUT ( VkDevice device );

        [[nodiscard]] bool UploadLUTSamples ( android_vulkan::Renderer &renderer );

        static void InitLUTSamples ( uint8_t* samples );
};

} // namespace mandelbrot


#endif // MANDELBROT_LUT_COLOR_H
