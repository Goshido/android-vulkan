#ifndef PBR_MARIO_PIPE_X1_H
#define PBR_MARIO_PIPE_X1_H


#include "pipe_base.h"


namespace pbr::mario {

class PipeX1 final : public PipeBase
{
    public:
        PipeX1 () = delete;

        PipeX1 ( PipeX1 const & ) = delete;
        PipeX1& operator = ( PipeX1 const & ) = delete;

        PipeX1 ( PipeX1 && ) = delete;
        PipeX1& operator = ( PipeX1 && ) = delete;

        ~PipeX1 () = delete;

        // Note "x", "y" and "z" coordinates must be in renderer units.
        static void Spawn ( Scene &scene, float x, float y, float z ) noexcept;
};

} // namespace pbr::mario


#endif // PBR_MARIO_PIPE_X1_H
