#include <precompiled_headers.hpp>
#include <ring_math.hpp>


namespace editor {

namespace {

constexpr float AXIS_THRESHOLD = 1.0e-3F;

// Half ring - angle is pi / 2
constexpr float HALF_RING_SIN_ANGLE = 1.0F;
constexpr float HALF_RING_COS_ANGLE = 0.0F;

// Full ring - angle is pi
constexpr float FULL_RING_SIN_ANGLE = 0.0F;
constexpr float FULL_RING_COS_ANGLE = -1.0F;

} // end of anonymous namespace

//----------------------------------------------------------------------------------------------------------------------

void RingMath::MakeBillboard ( GXMat3 &basis, GXVec2 &sinCosAngle, GXMat3 const &cameraBasis ) noexcept
{
    basis = cameraBasis;
    sinCosAngle = GXVec2 ( FULL_RING_SIN_ANGLE, FULL_RING_COS_ANGLE );
}

void RingMath::MakeGeneral ( GXMat3 &basis,
    GXVec2 &sinCosAngle,
    GXQuat const &ringOrientation,
    GXMat3 const &cameraBasis
) noexcept
{
    GXVec3 forward {};
    ringOrientation.GetForward ( forward );
    GXVec3 const &cameraForward = cameraBasis.Forward ();

    if ( 1.0F - std::abs ( forward.DotProduct ( cameraForward ) ) < AXIS_THRESHOLD ) [[unlikely]]
    {
        MakeBillboard ( basis, sinCosAngle, cameraBasis );
        return;
    }

    Arc ( basis, sinCosAngle, cameraForward, forward );
}

void RingMath::Arc ( GXMat3 &basis, GXVec2 &sinCosAngle, GXVec3 const &cameraForward, GXVec3 const &axis ) noexcept
{
    GXVec3 &x = basis.Right ();
    x.CrossProduct ( axis, cameraForward );
    x.Normalize ();

    GXVec3 &y = basis.Up ();
    y.CrossProduct ( axis, x );

    GXVec3 &z = basis.Forward ();
    z = axis;

    sinCosAngle = GXVec2 ( HALF_RING_SIN_ANGLE, HALF_RING_COS_ANGLE );
}

} // namespace editor
