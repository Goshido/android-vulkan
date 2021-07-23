#ifndef PBR_MARIO_PIPE_X2_H
#define PBR_MARIO_PIPE_X2_H


#include "pipe_base.h"


namespace pbr::mario {

class PipeX2 final : public PipeBase
{
    public:
        PipeX2 () = default;

        PipeX2 ( PipeX2 const & ) = delete;
        PipeX2& operator = ( PipeX2 const & ) = delete;

        PipeX2 ( PipeX2 && ) = delete;
        PipeX2& operator = ( PipeX2 && ) = delete;

        ~PipeX2 () override = default;

    private:
        [[nodiscard]] GXVec3 const& GetColliderOffset () const noexcept override;
        [[nodiscard]] GXVec3 const& GetColliderSize () const noexcept override;
        [[nodiscard]] char const* GetMesh () const noexcept override;
};

} // namespace pbr::mario


#endif // PBR_MARIO_PIPE_X2_H
