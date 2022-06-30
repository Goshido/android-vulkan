#ifndef PBR_MARIO_BRICK_H
#define PBR_MARIO_BRICK_H


#include <pbr/scene.h>


namespace pbr::mario {

class Brick final
{
    public:
        Brick () = delete;

        Brick ( Brick const & ) = delete;
        Brick& operator = ( Brick const & ) = delete;

        Brick ( Brick && ) = delete;
        Brick& operator = ( Brick && ) = delete;

        ~Brick () = delete;

        // Note "x", "y" and "z" coordinates must be in renderer units.
        static void Spawn ( Scene &scene, float x, float y, float z ) noexcept;
};

} // namespace pbr::mario


#endif // PBR_MARIO_BRICK_H
