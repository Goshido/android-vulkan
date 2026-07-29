#include <precompiled_headers.hpp>
#include <sdf_cone.hpp>
#include <workspace.hpp>


namespace editor {

SDFCone::SDFCone ( GXVec3 &&location, GXQuat &&rotation, GXVec3 &&scale, eSDFPalette palette, float radius ) noexcept:
    _location ( std::move ( location ) ),
    _rotation ( std::move ( rotation ) ),
    _scale ( std::move ( scale ) ),
    _radius ( radius ),
    _palette ( palette )
{
    // NOTHING
}

void SDFCone::SetColor ( eSDFPalette palette ) noexcept
{
    _node.SetColor ( palette );
}

void SDFCone::Show ( GXVec3 const &locationParent, GXQuat const &rotationParent ) noexcept
{
    _node = Workspace::Instance ().RegisterGizmo ( eSDFShape::Cone );
    OnParentTransformUpdated ( locationParent, rotationParent );
}

void SDFCone::Hide () noexcept
{
    _node = {};
}

void SDFCone::OnParentTransformUpdated ( GXVec3 const &/*location*/, GXQuat const &/*rotation*/ ) noexcept
{
    // FUCK
}

} // namespace editor
