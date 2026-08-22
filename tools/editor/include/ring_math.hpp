#ifndef EDITOR_RING_MATH_HPP
#define EDITOR_RING_MATH_HPP


#include <GXCommon/GXMath.hpp>


namespace editor {

class RingMath final
{
    public:
        RingMath () = delete;

        RingMath ( RingMath const & ) = delete;
        RingMath &operator = ( RingMath const & ) = delete;

        RingMath ( RingMath && ) = delete;
        RingMath &operator = ( RingMath && ) = delete;

        ~RingMath () = delete;

        static void MakeBillboard ( GXMat3 &basis, GXVec2 &sinCosAngle, GXMat3 const &cameraBasis ) noexcept;

        static void MakeGeneral ( GXMat3 &basis,
            GXVec2 &sinCosAngle,
            GXQuat const &ringOrientation,
            GXMat3 const &cameraBasis
        ) noexcept;

    private:
        static void Arc ( GXMat3 &basis,
            GXVec2 &sinCosAngle,
            GXVec3 const &cameraForward,
            GXVec3 const &axis
        ) noexcept;
};

} // namespace editor


#endif // EDITOR_RING_MATH_HPP
