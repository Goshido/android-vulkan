#include <precompiled_headers.hpp>
#include <sdf_line_segment_with_flip.hpp>
#include <sdf_size.hpp>
#include <workspace.hpp>


namespace editor {

SDFLineSegmentWithFlip::SDFLineSegmentWithFlip ( GXVec3 &&location,
    GXQuat &&rotation,
    GXVec3 &&scale,
    eSDFPalette palette,
    GXQuat const &aFlipRotation,
    GXVec3 const &aFlipLocation,
    float aFlipOffset,
    GXQuat const &bFlipRotation,
    GXVec3 const &bFlipLocation,
    float bFlipOffset
) noexcept:
    SDF ( std::move ( location ), std::move ( scale ), palette ),
    _rotation ( std::move ( rotation ) ),
    _aFlipRotation ( aFlipRotation ),
    _aFlipLocation ( aFlipLocation ),
    _bFlipRotation ( bFlipRotation ),
    _bFlipLocation ( bFlipLocation ),
    _aFlipOffset ( aFlipOffset ),
    _bFlipOffset ( bFlipOffset )
{
    // NOTHING
}

void SDFLineSegmentWithFlip::Show ( GXVec3 const &locationParent, GXQuat const &rotationParent ) noexcept
{
    _node = Workspace::Instance ().RegisterGizmo ( eSDFShape::LineSegment,
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
            float const side = std::min ( s._data[ 1U ], s._data[ 2U ] );
            float const negativeR = -side;

            Model &toWorld = vertex._toWorld;
            reinterpret_cast<GXMat3*> ( &toWorld )->FromFast ( _rotationWorld );

            GXVec3 &p = toWorld._w;
            p = _locationWorld;

            ReadSensors ( cameraLocation );

            if ( _aSensorResult )
                p.Sum ( p, _aFlipOffset, toWorld._x );

            if ( _bSensorResult )
                p.Sum ( p, _bFlipOffset, toWorld._y );

            alpha.Subtract ( p, _parentLocation );
            p.Sum ( _parentLocation, pixelSize, alpha );

            GXQuat &sdfOrientation = vertex._sdfOrientation;
            sdfOrientation.InverseFast ( _rotationWorld );

            GXVec3 &sdfOffset = vertex._sdfOffset;
            alpha.Sum ( p, side, toWorld._x );
            alpha.Reverse ();
            sdfOrientation.TransformFast ( sdfOffset, alpha );

            toWorld._x.Multiply ( toWorld._x, s._data[ 0U ] );
            toWorld._y.Multiply ( toWorld._y, side );
            toWorld._z.Multiply ( toWorld._z, side );

            GXVec4 &sdfParams = pixel._sdfParams;
            sdfParams._data[ 0U ] = s._data[ 0U ] + negativeR + negativeR;
            sdfParams._data[ 1U ] = negativeR;

            GXVec3 &cameraLocationSDF = pixel._cameraLocationSDF;
            sdfOrientation.TransformFast ( cameraLocationSDF, cameraLocation );
            cameraLocationSDF.Sum ( cameraLocationSDF, sdfOffset );

            sdfOrientation.TransformFast ( pixel._viSDF, viWorld );
        }
    );

    OnParentUpdated ( locationParent, rotationParent );
}

void SDFLineSegmentWithFlip::OnParentUpdated ( GXVec3 const &location, GXQuat const &rotation ) noexcept
{
    GXVec3 alpha {};
    rotation.TransformFast ( alpha, _location );
    _locationWorld.Sum ( location, alpha );
    _rotationWorld.Multiply ( rotation, _rotation );
    _parentLocation = location;
}

void SDFLineSegmentWithFlip::LockSensors ( GXVec3 const &cameraLocation ) noexcept
{
    _lockSensors = true;
    UpdateSensors ( cameraLocation );
}

void SDFLineSegmentWithFlip::UnlockSensors () noexcept
{
    _lockSensors = false;
}

void SDFLineSegmentWithFlip::ReadSensors ( GXVec3 const &cameraLocation ) noexcept
{
    if ( !_lockSensors )
    {
        UpdateSensors ( cameraLocation );
    }
}

void SDFLineSegmentWithFlip::UpdateSensors ( GXVec3 const &cameraLocation ) noexcept
{
    GXVec3 alpha {};
    alpha.Subtract ( _aFlipLocation, cameraLocation );

    GXVec3 right {};
    _aFlipRotation.GetRight ( right );
    _aSensorResult = alpha.DotProduct ( right ) > 0.0F;

    alpha.Subtract ( _bFlipLocation, cameraLocation );
    _bFlipRotation.GetRight ( right );
    _bSensorResult = alpha.DotProduct ( right ) > 0.0F;
}

} // namespace editor
