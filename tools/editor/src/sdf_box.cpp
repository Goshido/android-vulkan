#include <precompiled_headers.hpp>
#include <sdf_box.hpp>
#include <workspace.hpp>


namespace editor {

SDFBox::SDFBox ( GXVec3 &&location, GXQuat &&rotation, GXVec3 &&scale, eSDFPalette palette, float radius ) noexcept:
    _rotation ( std::move ( rotation ) ),
    _location ( std::move ( location ) ),
    _radius ( radius ),
    _scale ( std::move ( scale ) ),
    _palette ( palette )
{
    // NOTHING
}

GXQuat const &SDFBox::GetRotationWorld () const noexcept
{
    return _rotationWorld;
}

GXVec3 const &SDFBox::GetLocationWorld () const noexcept
{
    return _locationWorld;
}

void SDFBox::SetColor ( eSDFPalette palette ) noexcept
{
    _palette = palette;
    _node.MarkUpdate ();
}

void SDFBox::Show ( GXVec3 const &locationParent, GXQuat const &rotationParent ) noexcept
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

void SDFBox::Hide () noexcept
{
    _node = {};
}

void SDFBox::OnParentUpdated ( GXVec3 const &location, GXQuat const &rotation ) noexcept
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
