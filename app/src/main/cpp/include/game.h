#ifndef ANDROID_VULKAN_GAME_HPP
#define ANDROID_VULKAN_GAME_HPP


#include "renderer.h"


namespace android_vulkan {

class Game
{
    public:
        Game ( Game const & ) = delete;
        Game &operator = ( Game const & ) = delete;

        Game ( Game && ) = delete;
        Game &operator = ( Game && ) = delete;

        virtual ~Game () = default;

        [[nodiscard]] virtual bool IsReady () noexcept = 0;
        [[nodiscard]] virtual bool OnFrame ( Renderer &renderer, double deltaTime ) noexcept = 0;

        [[nodiscard]] virtual bool OnInitDevice ( Renderer &renderer ) noexcept = 0;
        virtual void OnDestroyDevice ( Renderer &renderer ) noexcept = 0;

        [[nodiscard]] virtual bool OnSwapchainCreated ( Renderer &renderer ) noexcept = 0;
        virtual void OnSwapchainDestroyed ( Renderer &renderer ) noexcept = 0;

    protected:
        Game () = default;
};

} // namespace android_vulkan


#endif // ANDROID_VULKAN_GAME_HPP
