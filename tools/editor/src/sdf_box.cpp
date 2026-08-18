#include <precompiled_headers.hpp>
#include <sdf_box.hpp>
#include <sdf_size.hpp>
#include <workspace.hpp>


namespace editor {

SDFBox::SDFBox ( GXVec3 &&location, GXQuat &&rotation, GXVec3 &&scale, eSDFPalette palette, float radius ) noexcept:
    _rotation ( std::move ( rotation ) ),
    _location ( std::move ( location ) ),
    _radius ( radius ),
    _scale ( std::move ( scale ) ),
    _palette ( palette )
{
    // NOTHING
}

GXQuat const &SDFBox::GetRotationWorld () const noexcept
{
    return _rotationWorld;
}

GXVec3 const &SDFBox::GetLocationWorld () const noexcept
{
    return _locationWorld;
}

void SDFBox::SetColor ( eSDFPalette palette ) noexcept
{
    _palette = palette;
}

void SDFBox::Show ( GXVec3 const &locationParent, GXQuat const &rotationParent ) noexcept
{
    _node = Workspace::Instance ().RegisterGizmo ( eSDFShape::Box,
        [ this ] ( SDFVertex &vertex,
            SDFPixel &pixel,
            SDFShape &shape,
            GXVec3 const &cameraLocation,
            GXVec3 const &/*cameraForward*/,
            GXVec3 const &viWorld
        ) noexcept {
            // See <repo>/docs/gizmo-rendering.md#pixel-coverage
            // FUCK - validate with Unity reference
            shape._palette = static_cast<uint32_t> ( _palette );

            GXVec3 alpha {};
            alpha.Subtract ( _parentLocation, cameraLocation );
            float const pixelSize = SDF_PIXEL_SIZE_SCALE * viWorld.DotProduct ( alpha );

            GXVec3 s {};
            s.Multiply ( _scale, pixelSize );
            float const r = _radius * pixelSize;
            reinterpret_cast<GXVec3*> ( &pixel._sdfParams )->Subtract ( GXVec3 ( r, r, r ), s );
            pixel._sdfParams._data[ 3U ] = -r;

            Model &toWorld = vertex._toWorld;
            reinterpret_cast<GXMat3*> ( &toWorld )->FromFast ( _rotationWorld );

            alpha.Subtract ( _locationWorld, _parentLocation );
            GXVec3 &p = toWorld._w;
            p.Sum ( _parentLocation, pixelSize, alpha );

            GXQuat &sdfOrientation = vertex._sdfOrientation;
            sdfOrientation.InverseFast ( _rotationWorld );

            toWorld._x.Multiply ( toWorld._x, s._data[ 0U ] );
            toWorld._y.Multiply ( toWorld._y, s._data[ 1U ] );
            toWorld._z.Multiply ( toWorld._z, s._data[ 2U ] );

            GXVec3 &sdfOffset = vertex._sdfOffset;
            sdfOrientation.TransformFast ( sdfOffset, p );
            sdfOffset.Reverse ();

            GXVec3 &cameraLocationSDF = pixel._cameraLocationSDF;
            sdfOrientation.TransformFast ( cameraLocationSDF, cameraLocation );
            cameraLocationSDF.Sum ( cameraLocationSDF, sdfOffset );

            sdfOrientation.TransformFast ( pixel._viSDF, viWorld );
        }
    );

    OnParentUpdated ( locationParent, rotationParent );
}

void SDFBox::Hide () noexcept
{
    _node = {};
}

void SDFBox::OnParentUpdated ( GXVec3 const &location, GXQuat const &rotation ) noexcept
{
    if ( !_node.IsConnected () ) [[unlikely]]
        return;

    GXVec3 alpha {};
    rotation.TransformFast ( alpha, _location );
    _locationWorld.Sum ( location, alpha );
    _rotationWorld.Multiply ( rotation, _rotation );
    _parentLocation = location;
}

} // namespace editor
