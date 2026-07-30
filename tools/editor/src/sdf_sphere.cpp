#include <precompiled_headers.hpp>
#include <sdf_sphere.hpp>
#include <workspace.hpp>


namespace editor {

SDFSphere::SDFSphere ( GXVec3 &&location, float radius, eSDFPalette palette ) noexcept:
    _location ( std::move ( location ) ),
    _radius ( radius ),
    _palette ( palette )
{
    // NOTHING
}

void SDFSphere::SetColor ( eSDFPalette palette ) noexcept
{
    _palette = palette;
    _node.MarkUpdate ();
}

void SDFSphere::Show ( GXVec3 const &locationParent, GXQuat const &rotationParent ) noexcept
{
    _node = Workspace::Instance ().RegisterGizmo ( eSDFShape::Sphere,
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

void SDFSphere::Hide () noexcept
{
    _node = {};
}

void SDFSphere::OnParentUpdated ( GXVec3 const &location, GXQuat const &rotation ) noexcept
{
    if ( !_node.IsConnected () ) [[unlikely]]
        return;

    GXVec3 alpha {};
    rotation.TransformFast ( alpha, _location );
    _locationWorld.Sum ( location, alpha );
    _node.MarkUpdate ();
}

} // namespace editor
