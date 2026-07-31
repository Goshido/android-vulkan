#include <precompiled_headers.hpp>
#include <sdf_box_with_flip.hpp>
#include <workspace.hpp>


namespace editor {

SDFBoxWithFlip::SDFBoxWithFlip ( GXVec3 &&location,
    GXQuat &&rotation,
    GXVec3 &&scale,
    eSDFPalette palette,
    float radius,
    float flipOffset
) noexcept:
    _rotation ( std::move ( rotation ) ),
    _location ( std::move ( location ) ),
    _radius ( radius ),
    _scale ( std::move ( scale ) ),
    _flipOffset ( flipOffset ),
    _palette ( palette )
{
    // NOTHING
}

GXQuat const &SDFBoxWithFlip::GetRotationWorld () const noexcept
{
    return _rotationWorld;
}

GXVec3 const &SDFBoxWithFlip::GetLocationWorld () const noexcept
{
    return _locationWorld;
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

void SDFBoxWithFlip::SetColor ( eSDFPalette palette ) noexcept
{
    _palette = palette;
    _node.MarkUpdate ();
}

void SDFBoxWithFlip::Show ( GXVec3 const &locationParent, GXQuat const &rotationParent ) noexcept
{
    _node = Workspace::Instance ().RegisterGizmo ( eSDFShape::Box,
        [ this ] ( SDFVertex &/*vertex*/,
            SDFPixel &/*pixel*/,
            SDFShape &/*shape*/,
            GXVec3 const &/*cameraLocation*/,
            GXVec3 const &/*cameraForward*/,
            GXVec3 const &/*viWorld*/
        ) noexcept {
            // TODO
            std::printf ( "%p", this );
        }
    );

    OnParentUpdated ( locationParent, rotationParent );
}

void SDFBoxWithFlip::Hide () noexcept
{
    _node = {};
}

void SDFBoxWithFlip::OnParentUpdated ( GXVec3 const &location, GXQuat const &rotation ) noexcept
{
    if ( !_node.IsConnected () ) [[unlikely]]
        return;

    GXVec3 alpha {};
    rotation.TransformFast ( alpha, _location );
    _locationWorld.Sum ( location, alpha );
    _rotationWorld.Multiply ( rotation, _rotation );
    _node.MarkUpdate ();
}

} // namespace editor
