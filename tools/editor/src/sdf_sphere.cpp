#include <precompiled_headers.hpp>
#include <sdf_sphere.hpp>
#include <workspace.hpp>


namespace editor {

SDFSphere::SDFSphere ( GXVec3 &&location, float diameter, eSDFPalette palette ) noexcept:
    _location ( std::move ( location ) ),
    _diameter ( diameter ),
    _palette ( palette )
{
    // NOTHING
}

void SDFSphere::SetColor ( eSDFPalette palette ) noexcept
{
    _node.SetColor ( palette );
}

void SDFSphere::Show ( GXVec3 const &locationParent, GXQuat const &rotationParent ) noexcept
{
    _node = Workspace::Instance ().RegisterGizmo ( eSDFShape::Sphere );
    OnParentTransformUpdated ( locationParent, rotationParent );
}

void SDFSphere::Hide () noexcept
{
    _node = {};
}

void SDFSphere::OnParentTransformUpdated ( GXVec3 const &/*location*/, GXQuat const &/*rotation*/ ) noexcept
{
    // FUCK
}

} // namespace editor
