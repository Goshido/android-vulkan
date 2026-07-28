#include <precompiled_headers.hpp>
#include <sdf_box.hpp>
#include <workspace.hpp>


namespace editor {

SDFBox::SDFBox ( GXVec3 &&location, GXQuat &&rotation, GXVec3 &&scale, eSDFPalette palette, float radius ) noexcept:
    _location ( std::move ( location ) ),
    _rotation ( std::move ( rotation ) ),
    _scale ( std::move ( scale ) ),
    _radius ( radius ),
    _palette ( palette )
{
    // NOTHING
}

void SDFBox::SetColor ( eSDFPalette palette ) noexcept
{
    _node.SetColor ( palette );
}

void SDFBox::Show ( GXVec3 const &locationParent, GXQuat const &rotationParent ) noexcept
{
    _node = Workspace::Instance ().RegisterGizmo ( eSDFShape::Box );
    OnParentTransformUpdated ( locationParent, rotationParent );
}

void SDFBox::Hide () noexcept
{
    _node = {};
}

void SDFBox::OnParentTransformUpdated ( GXVec3 const &/*location*/, GXQuat const &/*rotation*/ ) noexcept
{
    // FUCK
}

} // namespace editor
