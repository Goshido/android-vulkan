#ifndef PBR_GAME_HPP
#define PBR_GAME_HPP


#include "camera.hpp"
#include "render_session.hpp"
#include <game.hpp>


namespace pbr {

class PBRGame final : public android_vulkan::Game
{
    private:
        Camera                                              _camera {};
        VkCommandPool                                       _commandPool = VK_NULL_HANDLE;
        std::vector<VkCommandBuffer>                        _commandBuffers {};

        RenderSession                                       _renderSession {};
        std::list<ComponentRef>                             _allComponents {};
        std::list<std::reference_wrapper<ComponentRef>>     _renderableComponents {};

    public:
        PBRGame () = default;

        PBRGame ( PBRGame const & ) = delete;
        PBRGame &operator = ( PBRGame const & ) = delete;

        PBRGame ( PBRGame && ) = delete;
        PBRGame &operator = ( PBRGame && ) = delete;

        ~PBRGame () override = default;

    private:
        [[nodiscard]] bool IsReady () noexcept override;
        [[nodiscard]] bool OnFrame ( android_vulkan::Renderer &renderer, double deltaTime ) noexcept override;

        [[nodiscard]] bool OnInitDevice ( android_vulkan::Renderer &renderer ) noexcept override;
        void OnDestroyDevice ( android_vulkan::Renderer &renderer ) noexcept override;

        [[nodiscard]] bool OnSwapchainCreated ( android_vulkan::Renderer &renderer ) noexcept override;
        void OnSwapchainDestroyed ( android_vulkan::Renderer &renderer ) noexcept override;

        void DestroyCommandPool ( VkDevice device ) noexcept;
        [[nodiscard]] bool UploadGPUContent ( android_vulkan::Renderer &renderer ) noexcept;
};

} // namespace pbr


#endif // PBR_GAME_HPP
