#ifndef ANDROID_VULKAN_GAME_H
#define ANDROID_VULKAN_GAME_H


#include "renderer.h"


namespace android_vulkan {

class Game
{
    public:
        Game ( Game const & ) = delete;
        Game& operator = ( Game const & ) = delete;

        Game ( Game && ) = delete;
        Game& operator = ( Game && ) = delete;

        virtual ~Game () = default;

        [[nodiscard]] virtual bool IsReady () = 0;

        [[nodiscard]] virtual bool OnInit ( Renderer &renderer ) = 0;
        [[nodiscard]] virtual bool OnFrame ( Renderer &renderer, double deltaTime ) = 0;
        [[nodiscard]] virtual bool OnDestroy ( Renderer &renderer ) = 0;

    protected:
        Game () noexcept = default;
};

} // namespace android_vulkan


#endif // ANDROID_VULKAN_GAME_H
