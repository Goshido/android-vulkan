#include <precompiled_headers.hpp>
#include <sdf_sphere.hpp>
#include <sdf_size.hpp>
#include <workspace.hpp>


namespace editor {

SDFSphere::SDFSphere ( GXVec3 &&location, float radius, eSDFPalette palette ) noexcept:
    SDF ( std::move ( location ), GXVec3 ( radius, radius, radius ), palette )
{
    // NOTHING
}

void SDFSphere::Show ( GXVec3 const &locationParent, GXQuat const &rotationParent ) noexcept
{
    _node = Workspace::Instance ().RegisterGizmo ( eSDFShape::Sphere,
        [ this ] ( SDFVertex &vertex,
            SDFPixel &pixel,
            SDFShape &shape,
            GXVec3 const &cameraLocation,
            GXMat3 const &/*cameraBasis*/,
            GXVec3 const &viWorld
        ) noexcept {
            // See <repo>/docs/gizmo-rendering.md#pixel-coverage
            shape._palette = static_cast<uint32_t> ( _palette );

            GXVec3 alpha {};
            alpha.Subtract ( _parentLocation, cameraLocation );
            float const r = _scale._data[ 0U ] * SDF_PIXEL_SIZE_SCALE * viWorld.DotProduct ( alpha );

            Model &toWorld = vertex._toWorld;
            toWorld._x = GXVec3 ( r, 0.0F, 0.0F );
            toWorld._y = GXVec3 ( 0.0F, r, 0.0F );
            toWorld._z = GXVec3 ( 0.0F, 0.0F, r );
            toWorld._w = _locationWorld;

            vertex._sdfOrientation = GXQuat::IDENTITY;

            GXVec3 &sdfOffset = vertex._sdfOffset;
            sdfOffset = _locationWorld;
            sdfOffset.Reverse ();

            pixel._sdfParams._data[ 0U ] = -r;
            pixel._cameraLocationSDF.Sum ( cameraLocation, sdfOffset );
            pixel._viSDF = viWorld;
        }
    );

    OnParentUpdated ( locationParent, rotationParent );
}

void SDFSphere::OnParentUpdated ( GXVec3 const &location, GXQuat const &rotation ) noexcept
{
    if ( !_node.IsConnected () ) [[unlikely]]
        return;

    GXVec3 alpha {};
    rotation.TransformFast ( alpha, _location );
    _locationWorld.Sum ( location, alpha );
    _parentLocation = location;
}

} // namespace editor
