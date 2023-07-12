#ifndef ROTATING_MESH_GAME_ANALYTIC_HPP
#define ROTATING_MESH_GAME_ANALYTIC_HPP


#include "game.h"


namespace rotating_mesh {

class GameAnalytic final : public Game
{
    public:
        GameAnalytic () noexcept;
        ~GameAnalytic () override = default;

        GameAnalytic ( const GameAnalytic &other ) = delete;
        GameAnalytic &operator = ( const GameAnalytic &other ) = delete;

    private:
        bool CreateDescriptorSet ( android_vulkan::Renderer &renderer ) noexcept override;
        bool CreatePipelineLayout ( android_vulkan::Renderer &renderer ) noexcept override;
        bool LoadGPUContent ( android_vulkan::Renderer &renderer ) noexcept override;
};

} // namespace rotating_mesh


#endif // ROTATING_MESH_GAME_ANALYTIC_HPP
