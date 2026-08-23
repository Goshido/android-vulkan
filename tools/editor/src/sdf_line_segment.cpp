#include <precompiled_headers.hpp>
#include <sdf_line_segment.hpp>
#include <sdf_size.hpp>
#include <workspace.hpp>


namespace editor {

SDFLineSegment::SDFLineSegment ( GXVec3 &&location, GXQuat &&rotation, GXVec3 &&scale, eSDFPalette palette ) noexcept:
    SDF ( std::move ( location ), std::move ( scale ), palette ),
    _rotation ( std::move ( rotation ) )
{
    // NOTHING
}

void SDFLineSegment::Show ( GXVec3 const &locationParent, GXQuat const &rotationParent ) noexcept
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
            alpha.Subtract ( _locationWorld, _parentLocation );
            GXVec3 &p = toWorld._w;
            p.Sum ( _parentLocation, pixelSize, alpha );

            GXQuat &sdfOrientation = vertex._sdfOrientation;
            sdfOrientation.InverseFast ( _rotationWorld );

            reinterpret_cast<GXMat3*> ( &toWorld )->FromFast ( _rotationWorld );
            alpha.Sum ( p, side, toWorld._x );
            alpha.Reverse ();

            GXVec3 &sdfOffset = vertex._sdfOffset;
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

void SDFLineSegment::OnParentUpdated ( GXVec3 const &location, GXQuat const &rotation ) noexcept
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
