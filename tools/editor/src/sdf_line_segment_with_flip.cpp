#include <precompiled_headers.hpp>
#include <sdf_line_segment_with_flip.hpp>
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
        [ this ] ( SDFVertex &/*vertex*/,
            SDFPixel &/*pixel*/,
            GXVec3 const &/*viewerLocation*/,
            GXVec3 const &/*viewerForward*/,
            GXVec3 const &/*viWorld*/
        ) noexcept {
            // TODO
            std::printf ( "%p", this );
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
    _node.MarkUpdate ();
}

} // namespace editor
