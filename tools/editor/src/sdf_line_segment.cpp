#include <precompiled_headers.hpp>
#include <sdf_line_segment.hpp>
#include <workspace.hpp>


namespace editor {

SDFLineSegment::SDFLineSegment ( GXVec3 &&location, GXQuat &&rotation, GXVec3 &&scale, eSDFPalette palette ) noexcept:
    _location ( std::move ( location ) ),
    _rotation ( std::move ( rotation ) ),
    _scale ( std::move ( scale ) ),
    _palette ( palette )
{
    // NOTHING
}

void SDFLineSegment::SetColor ( eSDFPalette palette ) noexcept
{
    _node.SetColor ( palette );
}

void SDFLineSegment::Show ( GXVec3 const &locationParent, GXQuat const &rotationParent ) noexcept
{
    _node = Workspace::Instance ().RegisterGizmo ( eSDFShape::LineSegment );
    OnParentTransformUpdated ( locationParent, rotationParent );
}

void SDFLineSegment::Hide () noexcept
{
    _node = {};
}

void SDFLineSegment::OnParentTransformUpdated ( GXVec3 const &/*location*/, GXQuat const &/*rotation*/ ) noexcept
{
    // FUCK
}

} // namespace editor
