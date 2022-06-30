#ifndef PBR_MARIO_RIDDLE_H
#define PBR_MARIO_RIDDLE_H


#include <pbr/scene.h>


namespace pbr::mario {

class Riddle final
{
    public:
        Riddle () = delete;

        Riddle ( Riddle const & ) = delete;
        Riddle& operator = ( Riddle const & ) = delete;

        Riddle ( Riddle && ) = delete;
        Riddle& operator = ( Riddle && ) = delete;

        ~Riddle () = delete;

        // Note "x", "y" and "z" coordinates must be in renderer units.
        static void Spawn ( Scene &scene, float x, float y, float z ) noexcept;
};

} // namespace pbr::mario


#endif // PBR_MARIO_RIDDLE_H
