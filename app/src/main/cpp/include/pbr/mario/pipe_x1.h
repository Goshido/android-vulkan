#ifndef PBR_MARIO_PIPE_X1_H
#define PBR_MARIO_PIPE_X1_H


#include "pipe_base.h"


namespace pbr::mario {

class PipeX1 final : public PipeBase
{
    public:
        PipeX1 () = default;

        PipeX1 ( PipeX1 const & ) = delete;
        PipeX1& operator = ( PipeX1 const & ) = delete;

        PipeX1 ( PipeX1 && ) = delete;
        PipeX1& operator = ( PipeX1 && ) = delete;

        ~PipeX1 () override = default;

    private:
        [[nodiscard]] GXVec3 const& GetColliderOffset () const noexcept override;
        [[nodiscard]] GXVec3 const& GetColliderSize () const noexcept override;
        [[nodiscard]] char const* GetMesh () const noexcept override;
};

} // namespace pbr::mario


#endif // PBR_MARIO_PIPE_X1_H
