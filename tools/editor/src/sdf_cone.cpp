#include <precompiled_headers.hpp>
#include <sdf_cone.hpp>
#include <workspace.hpp>


namespace editor {

SDFCone::SDFCone ( GXVec3 &&location, GXQuat &&rotation, GXVec3 &&scale, eSDFPalette palette, float radius ) noexcept:
    _rotation ( std::move ( rotation ) ),
    _location ( std::move ( location ) ),
    _radius ( radius ),
    _scale ( std::move ( scale ) ),
    _palette ( palette )
{
    // NOTHING
}

void SDFCone::SetColor ( eSDFPalette palette ) noexcept
{
    _palette = palette;
    _node.MarkUpdate ();
}

void SDFCone::Show ( GXVec3 const &locationParent, GXQuat const &rotationParent ) noexcept
{
    _node = Workspace::Instance ().RegisterGizmo ( eSDFShape::Cone,
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

void SDFCone::Hide () noexcept
{
    _node = {};
}

void SDFCone::OnParentUpdated ( GXVec3 const &location, GXQuat const &rotation ) noexcept
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
