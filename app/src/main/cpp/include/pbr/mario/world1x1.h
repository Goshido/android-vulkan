#ifndef PBR_MARIO_WORLD1x1_H
#define PBR_MARIO_WORLD1x1_H


#include <pbr/scene.h>
#include <game.h>


namespace pbr::mario {

class World1x1 final : public android_vulkan::Game
{
    private:
        VkCommandPool                   _commandPool = VK_NULL_HANDLE;
        bool                            _isReady = false;
        android_vulkan::Physics         _physics {};
        RenderSession                   _renderSession {};
        Scene                           _scene {};

    public:
        World1x1 () = default;

        World1x1 ( World1x1 const & ) = delete;
        World1x1& operator = ( World1x1 const & ) = delete;

        World1x1 ( World1x1 && ) = delete;
        World1x1& operator = ( World1x1 && ) = delete;

        ~World1x1 () override = default;

    private:
        [[nodiscard]] bool IsReady () noexcept override;
        [[nodiscard]] bool OnFrame ( android_vulkan::Renderer &renderer, double deltaTime ) noexcept override;

        [[nodiscard]] bool OnInitDevice ( android_vulkan::Renderer &renderer ) noexcept override;
        void OnDestroyDevice ( android_vulkan::Renderer &renderer ) noexcept override;

        [[nodiscard]] bool OnSwapchainCreated ( android_vulkan::Renderer &renderer ) noexcept override;
        void OnSwapchainDestroyed ( android_vulkan::Renderer &renderer ) noexcept override;

        [[nodiscard]] bool CreatePhysics () noexcept;
        void DestroyCommandPool ( VkDevice device ) noexcept;
};

} // namespace pbr::mario


#endif // PBR_MARIO_WORLD1x1_H
