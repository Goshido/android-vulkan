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
        bool OnDestroy ( android_vulkan::Renderer &renderer ) override;

        bool CreateCommandBuffer ( android_vulkan::Renderer &renderer ) override;
        void DestroyCommandBuffer ( android_vulkan::Renderer &renderer ) override;
};

} // namespace mandelbrot


#endif // MANDELBROT_ANALYTIC_COLOR_H
