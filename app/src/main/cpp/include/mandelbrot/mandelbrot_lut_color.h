#ifndef MANDELBROT_LUT_COLOR_H
#define MANDELBROT_LUT_COLOR_H


#include "mandelbrot_base.h"


namespace mandelbrot {

class MandelbrotLUTColor final : public MandelbrotBase
{
    private:
        // Note VkImage is a VK_DEFINE_NON_DISPATCHABLE_HANDLE type.
        VkImage             _lut;

        // Note VkDeviceMemory is a VK_DEFINE_NON_DISPATCHABLE_HANDLE type.
        VkDeviceMemory      _lutDeviceMemory;

        // Note VkBuffer is a VK_DEFINE_NON_DISPATCHABLE_HANDLE type.
        VkBuffer            _transfer;

        // Note VkDeviceMemory is a VK_DEFINE_NON_DISPATCHABLE_HANDLE type.
        VkDeviceMemory      _transferDeviceMemory;

    public:
        MandelbrotLUTColor ();
        ~MandelbrotLUTColor () override = default;

        MandelbrotLUTColor ( const MandelbrotLUTColor &other ) = delete;
        MandelbrotLUTColor& operator = ( const MandelbrotLUTColor &other ) = delete;

    private:
        bool CreateCommandBuffer ( android_vulkan::Renderer &renderer ) override;
        void DestroyCommandBuffer ( android_vulkan::Renderer &renderer ) override;

        bool CreateLUT ( android_vulkan::Renderer &renderer );
        void DestroyLUT ( android_vulkan::Renderer &renderer );

        void InitLUTSamples ( uint8_t* samples ) const;

        bool TryAllocateMemory ( VkDeviceMemory &memory,
            android_vulkan::Renderer &renderer,
            const VkMemoryRequirements &requirements,
            VkMemoryPropertyFlags memoryProperties,
            const char* where,
            const char* checkFailMessage
        ) const;
};

} // namespace mandelbrot


#endif // MANDELBROT_LUT_COLOR_H
