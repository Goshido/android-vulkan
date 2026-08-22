#include <precompiled_headers.hpp>
#include <ring_math.hpp>
#include <sdf_ring.hpp>
#include <workspace.hpp>


namespace editor {

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
            GXMat3 const &cameraBasis,
            GXVec3 const &viWorld
        ) noexcept {
            RingMath::MakeGeneral ( *reinterpret_cast<GXMat3*> ( &vertex._toWorld ),
                *reinterpret_cast<GXVec2*> ( &pixel._sdfParams ),
                _rotationWorld,
                cameraBasis
            );

            ComputeParams ( vertex, pixel, shape, cameraLocation, viWorld );
        }
    );

    OnParentUpdated ( locationParent, rotationParent );
}

} // namespace editor
