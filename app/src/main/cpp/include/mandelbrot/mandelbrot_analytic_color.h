#ifndef MANDELBROT_ANALYTIC_COLOR_H
#define MANDELBROT_ANALYTIC_COLOR_H


#include "mandelbrot_base.h"


namespace mandelbrot {

class MandelbrotAnalyticColor final : public MandelbrotBase
{
    public:
        MandelbrotAnalyticColor ();
        ~MandelbrotAnalyticColor () override = default;

        MandelbrotAnalyticColor ( const MandelbrotAnalyticColor &other ) = delete;
        MandelbrotAnalyticColor& operator = ( const MandelbrotAnalyticColor &other ) = delete;

    private:
        [[nodiscard]] bool OnSwapchainCreated ( android_vulkan::Renderer &renderer ) override;
        void OnSwapchainDestroyed ( VkDevice device ) override;

        [[nodiscard]] bool CreatePipelineLayout ( android_vulkan::Renderer &renderer ) override;
        void DestroyPipelineLayout ( VkDevice device ) override;

        [[nodiscard]] bool CreateCommandBuffer ( android_vulkan::Renderer &renderer );
        void DestroyCommandBuffer ( VkDevice device );
};

} // namespace mandelbrot


#endif // MANDELBROT_ANALYTIC_COLOR_H
