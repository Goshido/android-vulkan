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
        MandelbrotLUTColor ();
        ~MandelbrotLUTColor () override = default;

        MandelbrotLUTColor ( const MandelbrotLUTColor &other ) = delete;
        MandelbrotLUTColor& operator = ( const MandelbrotLUTColor &other ) = delete;

    private:
        bool OnInit ( android_vulkan::Renderer &renderer ) override;
        bool OnDestroy ( android_vulkan::Renderer &renderer ) override;

        bool CreatePipelineLayout ( android_vulkan::Renderer &renderer ) override;
        void DestroyPipelineLayout ( android_vulkan::Renderer &renderer ) override;

        bool CreateCommandBuffer ( android_vulkan::Renderer &renderer );
        void DestroyCommandBuffer ( android_vulkan::Renderer &renderer );

        bool CreateDescriptorSet ( android_vulkan::Renderer &renderer );
        void DestroyDescriptorSet ( android_vulkan::Renderer &renderer );

        bool CreateLUT ( android_vulkan::Renderer &renderer );
        void DestroyLUT ( android_vulkan::Renderer &renderer );

        void InitLUTSamples ( uint8_t* samples ) const;
        bool UploadLUTSamples ( android_vulkan::Renderer &renderer );
};

} // namespace mandelbrot


#endif // MANDELBROT_LUT_COLOR_H
