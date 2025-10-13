#ifndef RAINBOW_HPP
#define RAINBOW_HPP


#include <game.hpp>


namespace rainbow {

class Rainbow final : public android_vulkan::Game
{
    private:
        struct SwapchainInfo final
        {
            VkSemaphore                 _renderEnd = VK_NULL_HANDLE;
            VkFramebuffer               _framebuffer = VK_NULL_HANDLE;
        };

        struct FrameInFlight final
        {
            VkSemaphore                 _acquire = VK_NULL_HANDLE;
            VkFence                     _fence = VK_NULL_HANDLE;
            VkCommandPool               _pool = VK_NULL_HANDLE;
            VkCommandBuffer             _buffer = VK_NULL_HANDLE;
        };

    private:
        constexpr static size_t         FRAMES_IN_FLIGHT = 2U;

        FrameInFlight                   _framesInFlight[ FRAMES_IN_FLIGHT ];
        size_t                          _writeFrameIndex = 0U;

        std::vector<SwapchainInfo>      _swapchainInfo{};

        VkRenderPassBeginInfo           _renderPassBeginInfo
        {
            .sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
            .pNext = nullptr,
            .renderPass = VK_NULL_HANDLE,
            .framebuffer = VK_NULL_HANDLE,

            .renderArea {
                .offset {
                    .x = 0,
                    .y = 0
                },

                .extent{}
            },

            .clearValueCount = 1U,
            .pClearValues = nullptr
        };

    public:
        Rainbow () = default;

        Rainbow ( Rainbow const & ) = delete;
        Rainbow &operator = ( Rainbow const & ) = delete;

        Rainbow ( Rainbow && ) = delete;
        Rainbow &operator = ( Rainbow && ) = delete;

        ~Rainbow () override = default;

    private:
        [[nodiscard]] bool IsReady () noexcept override;
        [[nodiscard]] bool OnFrame ( android_vulkan::Renderer &renderer, double deltaTime ) noexcept override;

        [[nodiscard]] bool OnInitDevice ( android_vulkan::Renderer &renderer ) noexcept override;
        void OnDestroyDevice ( android_vulkan::Renderer &renderer ) noexcept override;

        [[nodiscard]] bool OnSwapchainCreated ( android_vulkan::Renderer &renderer ) noexcept override;
        void OnSwapchainDestroyed ( android_vulkan::Renderer &renderer ) noexcept override;

        [[nodiscard]] bool BeginFrame ( android_vulkan::Renderer &renderer,
            uint32_t &swapchainImageIndex,
            VkSemaphore acquire
        ) noexcept;

        [[nodiscard]] bool EndFrame ( android_vulkan::Renderer &renderer,
            uint32_t swapchainImageIndex,
            VkSemaphore renderEnd
        ) noexcept;

        [[nodiscard]] bool CreateSwapchainInfo ( android_vulkan::Renderer &renderer ) noexcept;
        void DestroySwapchainInfo ( VkDevice device ) noexcept;

        [[nodiscard]] bool CreateRenderPass ( android_vulkan::Renderer &renderer ) noexcept;
        void DestroyRenderPass ( VkDevice device ) noexcept;
};

} // namespace rainbow


#endif // RAINBOW_HPP
