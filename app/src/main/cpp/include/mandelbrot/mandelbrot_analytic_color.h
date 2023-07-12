#ifndef MANDELBROT_ANALYTIC_COLOR_HPP
#define MANDELBROT_ANALYTIC_COLOR_HPP


#include "mandelbrot_base.h"


namespace mandelbrot {

class MandelbrotAnalyticColor final : public MandelbrotBase
{
    public:
        MandelbrotAnalyticColor () noexcept;

        MandelbrotAnalyticColor ( MandelbrotAnalyticColor const & ) = delete;
        MandelbrotAnalyticColor &operator = ( MandelbrotAnalyticColor const & ) = delete;

        MandelbrotAnalyticColor ( MandelbrotAnalyticColor && ) = delete;
        MandelbrotAnalyticColor &operator = ( MandelbrotAnalyticColor && ) = delete;

        ~MandelbrotAnalyticColor () override = default;

    private:
        [[nodiscard]] bool OnSwapchainCreated ( android_vulkan::Renderer &renderer ) noexcept override;
        void OnSwapchainDestroyed ( android_vulkan::Renderer &renderer ) noexcept override;

        [[nodiscard]] bool CreatePipelineLayout ( android_vulkan::Renderer &renderer ) noexcept override;
        void DestroyPipelineLayout ( VkDevice device ) noexcept override;

        [[nodiscard]] bool CreateCommandBuffer ( android_vulkan::Renderer &renderer ) noexcept;
        void DestroyCommandBuffer ( VkDevice device ) noexcept;
};

} // namespace mandelbrot


#endif // MANDELBROT_ANALYTIC_COLOR_HPP
