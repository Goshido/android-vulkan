#ifndef GAME_H
#define GAME_H


#include "renderer.h"


namespace android_vulkan {

class Game
{
    public:
        Game ( const Game &other ) = delete;
        Game& operator = ( const Game &other ) = delete;
        virtual ~Game () = default;

        [[nodiscard]] virtual bool IsReady () = 0;

        [[nodiscard]] virtual bool OnInit ( Renderer &renderer ) = 0;
        [[nodiscard]] virtual bool OnFrame ( Renderer &renderer, double deltaTime ) = 0;
        [[nodiscard]] virtual bool OnDestroy ( Renderer &renderer ) = 0;

    protected:
        Game () = default;
};

} // namespace android_vulkan


#endif // GAME_H
