#ifndef ROTATING_MESH_GAME_ANALYTIC_H
#define ROTATING_MESH_GAME_ANALYTIC_H


#include "game.h"


namespace rotating_mesh {

class GameAnalytic final : public Game
{
    public:
        GameAnalytic ();
        ~GameAnalytic () override = default;

        GameAnalytic ( const GameAnalytic &other ) = delete;
        GameAnalytic& operator = ( const GameAnalytic &other ) = delete;

    private:
        bool CreateDescriptorSet ( android_vulkan::Renderer &renderer ) override;
        bool CreatePipelineLayout ( android_vulkan::Renderer &renderer ) override;
        bool LoadGPUContent ( android_vulkan::Renderer &renderer ) override;
};

} // namespace rotating_mesh


#endif // ROTATING_MESH_GAME_ANALYTIC_H
