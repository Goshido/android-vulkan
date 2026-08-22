#include <precompiled_headers.hpp>
#include <sdf_ring_base.hpp>
#include <sdf_size.hpp>


namespace editor {

SDFRingBase::SDFRingBase ( GXVec3 &&location, GXQuat &&rotation, GXVec3 &&scale, eSDFPalette palette ) noexcept:
    _rotation ( std::move ( rotation ) ),
    _location ( std::move ( location ) ),
    _scale ( std::move ( scale ) ),
    _palette ( palette )
{
    // NOTHING
}

GXQuat const &SDFRingBase::GetRotationWorld () const noexcept
{
    return _rotationWorld;
}

GXVec3 const &SDFRingBase::GetLocationWorld () const noexcept
{
    return _locationWorld;
}

void SDFRingBase::SetColor ( eSDFPalette palette ) noexcept
{
    _palette = palette;
}

void SDFRingBase::Hide () noexcept
{
    _node = {};
}

void SDFRingBase::OnParentUpdated ( GXVec3 const &location, GXQuat const &rotation ) noexcept
{
    if ( !_node.IsConnected () ) [[unlikely]]
        return;

    GXVec3 alpha {};
    rotation.TransformFast ( alpha, _location );
    _locationWorld.Sum ( location, alpha );
    _rotationWorld.Multiply ( rotation, _rotation );
    _parentLocation = location;
}

void SDFRingBase::ComputeParams ( SDFVertex &vertex,
    SDFPixel &pixel,
    SDFShape &shape,
    GXVec3 const &cameraLocation,
    GXVec3 const &viWorld
) noexcept
{
    // See <repo>/docs/gizmo-rendering.md#pixel-coverage
    shape._palette = static_cast<uint32_t> ( _palette );

    GXVec3 alpha {};
    alpha.Subtract ( _parentLocation, cameraLocation );
    float const pixelSize = SDF_PIXEL_SIZE_SCALE * viWorld.DotProduct ( alpha );

    Model &toWorld = vertex._toWorld;
    GXMat3 &basis = *reinterpret_cast<GXMat3*> ( &toWorld );
    GXQuat q {};
    q.FromFast ( basis );

    GXQuat &sdfOrientation = vertex._sdfOrientation;
    sdfOrientation.InverseFast ( q );

    GXVec3 s {};
    s.Multiply ( _scale, pixelSize );

    float const thickness = -s._data[ 2U ];
    float const radius = std::min ( s._data[ 0U ], s._data[ 1U ] );

    toWorld._w = _locationWorld;
    toWorld._x.Multiply ( toWorld._x, radius );
    toWorld._y.Multiply ( toWorld._y, radius );
    toWorld._z.Multiply ( toWorld._z, s._data[ 2U ] );

    GXVec4 &sdfParams = pixel._sdfParams;
    sdfParams._data[ 2U ] = radius + thickness;
    sdfParams._data[ 3U ] = thickness;

    alpha = _locationWorld;
    alpha.Reverse ();
    GXVec3 &sdfOffset = vertex._sdfOffset;
    sdfOrientation.TransformFast ( sdfOffset, alpha );

    GXVec3 &cameraLocationSDF = pixel._cameraLocationSDF;
    sdfOrientation.TransformFast ( cameraLocationSDF, cameraLocation );
    cameraLocationSDF.Sum ( cameraLocationSDF, sdfOffset );

    sdfOrientation.TransformFast ( pixel._viSDF, viWorld );
}

} // namespace editor
