#include <precompiled_headers.hpp>
#include <sdf_ring_billboard.hpp>
#include <workspace.hpp>


namespace editor {

SDFRingBillboard::SDFRingBillboard ( GXVec3 &&location,
    GXQuat &&rotation,
    GXVec3 &&scale,
    eSDFPalette palette
) noexcept:
    SDFRingBase ( std::move ( location ), std::move ( rotation ), std::move ( scale ), palette )
{
    // NOTHING
}

void SDFRingBillboard::Show ( GXVec3 const &locationParent, GXQuat const &rotationParent ) noexcept
{
    _node = Workspace::Instance ().RegisterGizmo ( eSDFShape::Ring,
        [ this ] ( SDFVertex &vertex,
            SDFPixel &pixel,
            SDFShape &shape,
            GXVec3 const &cameraLocation,
            GXVec3 const &cameraForward,
            GXVec3 const &viWorld
        ) noexcept {
            Ring ( *reinterpret_cast<GXMat3*> ( &vertex._toWorld ),
                *reinterpret_cast<GXVec2*> ( &pixel._sdfParams ),
                cameraForward
            );

            ComputeParams ( vertex, pixel, shape, cameraLocation, viWorld );
        }
    );

    OnParentUpdated ( locationParent, rotationParent );
}

} // namespace editor
