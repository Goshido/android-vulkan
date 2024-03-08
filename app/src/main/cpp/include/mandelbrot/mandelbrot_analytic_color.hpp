#ifndef MANDELBROT_ANALYTIC_COLOR_HPP
#define MANDELBROT_ANALYTIC_COLOR_HPP


#include "mandelbrot_base.hpp"


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
        [[nodiscard]] bool CreatePipelineLayout ( VkDevice device ) noexcept override;
        void DestroyPipelineLayout ( VkDevice device ) noexcept override;

        [[nodiscard]] bool RecordCommandBuffer ( android_vulkan::Renderer &renderer,
            VkCommandBuffer commandBuffer,
            VkFramebuffer framebuffer
        ) noexcept override;
};

} // namespace mandelbrot


#endif // MANDELBROT_ANALYTIC_COLOR_HPP
