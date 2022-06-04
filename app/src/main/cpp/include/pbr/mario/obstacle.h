#ifndef PBR_MARIO_OBSTACLE_H
#define PBR_MARIO_OBSTACLE_H


#include <pbr/scene.h>

GX_DISABLE_COMMON_WARNINGS

#include <span>

GX_RESTORE_WARNING_STATE


namespace pbr {

class Obstacle final
{
    public:
        struct Item final
        {
            GXVec3      _location;
            GXVec3      _size;
            float       _friction;
        };

        using Items = std::span<Item const>;

    public:
        Obstacle () = delete;

        Obstacle ( Obstacle const & ) = delete;
        Obstacle& operator = ( Obstacle const & ) = delete;

        Obstacle ( Obstacle && ) = delete;
        Obstacle& operator = ( Obstacle && ) = delete;

        ~Obstacle () = delete;

        static void Spawn ( Items const &items, Scene &scene, std::string &&name ) noexcept;
};

} // namespace pbr


#endif // PBR_MARIO_OBSTACLE_H
