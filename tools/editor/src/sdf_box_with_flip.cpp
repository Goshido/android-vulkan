#include <precompiled_headers.hpp>
#include <sdf_box_with_flip.hpp>
#include <sdf_size.hpp>
#include <workspace.hpp>


namespace editor {

SDFBoxWithFlip::SDFBoxWithFlip ( GXVec3 &&location,
    GXQuat &&rotation,
    GXVec3 &&scale,
    eSDFPalette palette,
    float radius,
    float flipOffset
) noexcept:
    SDF ( std::move ( location ), std::move ( scale ), palette ),
    _rotation ( std::move ( rotation ) ),
    _radius ( radius ),
    _flipOffset ( flipOffset )
{
    // NOTHING
}

void SDFBoxWithFlip::Show ( GXVec3 const &locationParent, GXQuat const &rotationParent ) noexcept
{
    _node = Workspace::Instance ().RegisterGizmo ( eSDFShape::Box,
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
            float const pixelSize = SDF_PIXEL_SIZE_SCALE * viWorld.DotProduct ( alpha );

            GXVec3 s {};
            s.Multiply ( _scale, pixelSize );
            float const r = _radius * pixelSize;
            reinterpret_cast<GXVec3*> ( &pixel._sdfParams )->Subtract ( GXVec3 ( r, r, r ), s );
            pixel._sdfParams._data[ 3U ] = -r;

            Model &toWorld = vertex._toWorld;
            reinterpret_cast<GXMat3*> ( &toWorld )->FromFast ( _rotationWorld );
            GXVec3 &p = toWorld._w;
            p = _locationWorld;

            GXVec3 flipRight {};
            _aFlipRotation->GetRight ( flipRight );
            alpha.Subtract ( *_aFlipLocation, cameraLocation );

            if ( flipRight.DotProduct ( alpha ) > 0.0F )
                p.Sum ( p, _flipOffset, toWorld._y );

            _bFlipRotation->GetRight ( flipRight );
            alpha.Subtract ( *_bFlipLocation, cameraLocation );

            if ( flipRight.DotProduct ( alpha ) > 0.0F )
                p.Sum ( p, _flipOffset, toWorld._z );

            alpha.Subtract ( p, _parentLocation );
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

void SDFBoxWithFlip::OnParentUpdated ( GXVec3 const &location, GXQuat const &rotation ) noexcept
{
    GXVec3 alpha {};
    rotation.TransformFast ( alpha, _location );
    _locationWorld.Sum ( location, alpha );
    _rotationWorld.Multiply ( rotation, _rotation );
    _parentLocation = location;
}

void SDFBoxWithFlip::SetFlipSensors ( GXQuat const &aFlipRotation,
    GXVec3 const &aFlipLocation,
    GXQuat const &bFlipRotation,
    GXVec3 const &bFlipLocation
) noexcept
{
    _aFlipRotation = &aFlipRotation;
    _aFlipLocation = &aFlipLocation;
    _bFlipRotation = &bFlipRotation;
    _bFlipLocation = &bFlipLocation;
}

} // namespace editor
