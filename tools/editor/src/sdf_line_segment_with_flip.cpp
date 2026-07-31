#include <precompiled_headers.hpp>
#include <sdf_line_segment_with_flip.hpp>
#include <sdf_size.hpp>
#include <workspace.hpp>


namespace editor {

SDFLineSegmentWithFlip::SDFLineSegmentWithFlip ( GXVec3 &&location,
    GXQuat &&rotation,
    GXVec3 &&scale,
    eSDFPalette palette,
    GXQuat const &xFlipRotation,
    GXVec3 const &xFlipLocation,
    float xFlipOffset,
    GXQuat const &yFlipRotation,
    GXVec3 const &yFlipLocation,
    float yFlipOffset
) noexcept:
    _rotation ( std::move ( rotation ) ),
    _location ( std::move ( location ) ),
    _scale ( std::move ( scale ) ),
    _xFlipOffset ( xFlipOffset ),
    _xFlipRotation ( xFlipRotation ),
    _xFlipLocation ( xFlipLocation ),
    _yFlipRotation ( yFlipRotation ),
    _yFlipLocation ( yFlipLocation ),
    _yFlipOffset ( yFlipOffset ),
    _palette ( palette )
{
    // NOTHING
}

void SDFLineSegmentWithFlip::SetColor ( eSDFPalette palette ) noexcept
{
    _palette = palette;
    _node.MarkUpdate ();
}

void SDFLineSegmentWithFlip::Show ( GXVec3 const &locationParent, GXQuat const &rotationParent ) noexcept
{
    _node = Workspace::Instance ().RegisterGizmo ( eSDFShape::LineSegment,
        [ this ] ( SDFVertex &vertex,
            SDFPixel &pixel,
            SDFShape &shape,
            GXVec3 const &cameraLocation,
            GXVec3 const &/*cameraForward*/,
            GXVec3 const &viWorld
        ) noexcept {
            shape._palette = static_cast<uint32_t> ( _palette );

            GXVec3 alpha{};
            alpha.Subtract ( _parentLocation, cameraLocation );
            float const pixelSize = SDF_PIXEL_SIZE_SCALE * viWorld.DotProduct ( alpha );

            GXVec3 s {};
            s.Multiply ( _scale, pixelSize );
            float const side = std::min ( s._data[ 1U ], s._data[ 2U ] );
            float const negativeR = -side;

            Model &toWorld = vertex._toWorld;
            GXMat3 &basis = *reinterpret_cast<GXMat3*> ( &toWorld );
            basis.FromFast ( _rotationWorld );

            GXVec3 &p = toWorld._w;
            GXVec3 flipRight {};
            _xFlipRotation.GetRight ( flipRight );
            alpha.Subtract ( _xFlipLocation, cameraLocation );

            if ( flipRight.DotProduct ( alpha ) > 0.0F )
                p.Sum ( p, _xFlipOffset, toWorld._x );

            _yFlipRotation.GetRight ( flipRight );
            alpha.Subtract ( _yFlipLocation, cameraLocation );

            if ( flipRight.DotProduct ( alpha ) > 0.0F )
                p.Sum ( p, _yFlipOffset, toWorld._y );

            alpha.Subtract ( p, _parentLocation );
            p.Sum ( _parentLocation, pixelSize, alpha );

            GXQuat sdfOrientation {};
            sdfOrientation.InverseFast ( _rotationWorld );

            GXVec3 &sdfOffset = vertex._sdfOffset;
            alpha.Sum ( _locationWorld, side, toWorld._x );
            alpha.Reverse ();
            sdfOrientation.TransformFast ( sdfOffset, alpha );

            toWorld._x.Multiply ( toWorld._x, s._data[ 0U ] );
            toWorld._y.Multiply ( toWorld._y, side );
            toWorld._z.Multiply ( toWorld._z, side );

            GXVec4 &sdfParams = pixel._sdfParams;
            sdfParams._data[ 0U ] = s._data[ 0U ] + negativeR + negativeR;
            sdfParams._data[ 1U ] = negativeR;

            vertex._sdfOrientation = sdfOrientation.ToTBN64 ();

            GXVec3 &cameraLocationSDF = pixel._cameraLocationSDF;
            sdfOrientation.TransformFast ( cameraLocationSDF, cameraLocation );
            cameraLocationSDF.Sum ( cameraLocationSDF, sdfOffset );

            sdfOrientation.TransformFast ( pixel._viSDF, viWorld );
        }
    );

    OnParentUpdated ( locationParent, rotationParent );
}

void SDFLineSegmentWithFlip::Hide () noexcept
{
    _node = {};
}

void SDFLineSegmentWithFlip::OnParentUpdated ( GXVec3 const &location, GXQuat const &rotation ) noexcept
{
    if ( !_node.IsConnected () ) [[unlikely]]
        return;

    GXVec3 alpha {};
    rotation.TransformFast ( alpha, _location );
    _locationWorld.Sum ( location, alpha );
    _rotationWorld.Multiply ( rotation, _rotation );
    _parentLocation = location;
    _node.MarkUpdate ();
}

} // namespace editor
