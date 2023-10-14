#ifndef PBR_UNIVERSAL_HPP
#define PBR_UNIVERSAL_HPP


#include <pbr/scene.hpp>
#include <game.hpp>


namespace pbr::universal {

class Universal final : public android_vulkan::Game
{
    private:
        VkCommandPool                   _commandPool = VK_NULL_HANDLE;
        bool                            _isReady = false;
        android_vulkan::Physics         _physics {};
        RenderSession                   _renderSession {};
        Scene                           _scene {};
        std::string const               _sceneFile {};

    public:
        Universal () = delete;

        Universal ( Universal const & ) = delete;
        Universal &operator = ( Universal const & ) = delete;

        Universal ( Universal && ) = delete;
        Universal &operator = ( Universal && ) = delete;

        explicit Universal ( std::string &&sceneFile ) noexcept;

        ~Universal () override = default;

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

} // namespace pbr::universal


#endif // PBR_UNIVERSAL_HPP
