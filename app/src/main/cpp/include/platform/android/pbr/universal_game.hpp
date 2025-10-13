#ifndef PBR_UNIVERSAL_GAME_HPP
#define PBR_UNIVERSAL_GAME_HPP


#include "scene.hpp"
#include <game.hpp>


namespace pbr {

class UniversalGame final : public android_vulkan::Game
{
    private:
        VkCommandPool               _commandPool = VK_NULL_HANDLE;
        bool                        _isReady = false;
        android_vulkan::Physics     _physics {};
        RenderSession               _renderSession {};
        Scene                       _scene {};
        std::string const           _sceneFile {};

    public:
        UniversalGame () = delete;

        UniversalGame ( UniversalGame const & ) = delete;
        UniversalGame &operator = ( UniversalGame const & ) = delete;

        UniversalGame ( UniversalGame && ) = delete;
        UniversalGame &operator = ( UniversalGame && ) = delete;

        explicit UniversalGame ( std::string &&sceneFile ) noexcept;

        ~UniversalGame () override = default;

    private:
        [[nodiscard]] bool IsReady () noexcept override;
        [[nodiscard]] bool OnFrame ( android_vulkan::Renderer &renderer, double deltaTime ) noexcept override;

        [[nodiscard]] bool OnInitDevice ( android_vulkan::Renderer &renderer ) noexcept override;
        void OnDestroyDevice ( android_vulkan::Renderer &renderer ) noexcept override;

        [[nodiscard]] bool OnSwapchainCreated ( android_vulkan::Renderer &renderer ) noexcept override;
        void OnSwapchainDestroyed ( android_vulkan::Renderer &renderer ) noexcept override;

        [[nodiscard]] bool OnInitSoundSystem () noexcept override;
        void OnDestroySoundSystem () noexcept override;

        [[nodiscard]] bool CreatePhysics () noexcept;
        void DestroyCommandPool ( VkDevice device ) noexcept;
};

} // namespace pbr


#endif // PBR_UNIVERSAL_GAME_HPP
