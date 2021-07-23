#ifndef PBR_MARIO_TARGET_H
#define PBR_MARIO_TARGET_H


#include <GXCommon/GXMath.h>


namespace pbr::mario {

class ITarget
{
    public:
        ITarget ( ITarget const & ) = delete;
        ITarget& operator = ( ITarget const & ) = delete;

        ITarget ( ITarget && ) = delete;
        ITarget& operator = ( ITarget && ) = delete;

        [[nodiscard]] virtual GXMat4 const& GetTransform () const noexcept = 0;

    protected:
        ITarget () = default;
        virtual ~ITarget () = default;
};

} // namespace pbr::mario


#endif // PBR_MARIO_TARGET_H
