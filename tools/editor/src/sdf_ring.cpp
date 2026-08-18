#include <precompiled_headers.hpp>
#include <sdf_ring.hpp>
#include <workspace.hpp>


namespace editor {

namespace {

constexpr float AXIS_THRESHOLD = 1.0e-3F;

// Half ring - angle is pi / 2
constexpr float HALF_RING_SIN_ANGLE = 1.0F;
constexpr float HALF_RING_COS_ANGLE = 0.0F;

} // end of anonymous namespace

SDFRing::SDFRing ( GXVec3 &&location, GXQuat &&rotation, GXVec3 &&scale, eSDFPalette palette ) noexcept:
    SDFRingBase ( std::move ( location ), std::move ( rotation ), std::move ( scale ), palette )
{
    // NOTHING
}

void SDFRing::Show ( GXVec3 const &locationParent, GXQuat const &rotationParent ) noexcept
{
    _node = Workspace::Instance ().RegisterGizmo ( eSDFShape::Ring,
        [ this ] ( SDFVertex &vertex,
            SDFPixel &pixel,
            SDFShape &shape,
            GXVec3 const &cameraLocation,
            GXVec3 const &cameraForward,
            GXVec3 const &viWorld
        ) noexcept {
            GXMat3 &basis = *reinterpret_cast<GXMat3*> ( &vertex._toWorld );
            GXVec2 &sinCosAngle = *reinterpret_cast<GXVec2*> ( &pixel._sdfParams );

            GXVec3 forward {};
            _rotationWorld.GetForward ( forward );

            if ( 1.0F - std::abs ( forward.DotProduct ( cameraForward ) ) < AXIS_THRESHOLD ) [[unlikely]]
                Ring ( basis, sinCosAngle, cameraForward );
            else
                Arc ( basis, sinCosAngle, cameraForward, forward );

            ComputeParams ( vertex, pixel, shape, cameraLocation, viWorld );
        }
    );

    OnParentUpdated ( locationParent, rotationParent );
}

void SDFRing::Arc ( GXMat3 &basis, GXVec2 &sinCosAngle, GXVec3 const &cameraForward, GXVec3 const &axis ) noexcept
{
    GXVec3 &x = *reinterpret_cast<GXVec3*> ( basis._data[ 0U ] );
    x.CrossProduct ( axis, cameraForward );
    x.Normalize ();

    GXVec3 &y = *reinterpret_cast<GXVec3*> ( basis._data[ 1U ] );
    y.CrossProduct ( axis, x );

    GXVec3 &z = *reinterpret_cast<GXVec3*> ( basis._data[ 2U ] );
    z = axis;

    sinCosAngle = GXVec2 ( HALF_RING_SIN_ANGLE, HALF_RING_COS_ANGLE );
}

} // namespace editor
