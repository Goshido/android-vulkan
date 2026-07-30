#include <precompiled_headers.hpp>
#include <gizmo_node.hpp>
#include <workspace.hpp>


namespace editor {

GizmoNode::GizmoNode ( GizmoNode &&other ) noexcept
{
    bool const myLock = TryLock ();
    Disconnect ();

    bool const otherLock = other.TryLock ();

    _workspace = std::exchange ( other._workspace, nullptr );
    _hasChanges = std::exchange ( other._hasChanges, false );

    if ( _gizmoInfo = std::exchange ( other._gizmoInfo, nullptr ); _gizmoInfo )
        _gizmoInfo->_node = this;

    _update = std::exchange ( other._update, nullptr );

    if ( otherLock )
        other.Unlock ();

    if ( myLock )
    {
        Unlock ();
    }
}

GizmoNode &GizmoNode::operator = ( GizmoNode &&other ) noexcept
{
    if ( this == &other ) [[unlikely]]
        return *this;

    bool const locked = TryLock ();
    Disconnect ();

    bool const otherLock = other.TryLock ();
    _workspace = std::exchange ( other._workspace, nullptr );
    _hasChanges = std::exchange ( other._hasChanges, false );

    if ( _gizmoInfo = std::exchange ( other._gizmoInfo, nullptr ); _gizmoInfo )
        _gizmoInfo->_node = this;

    _update = std::exchange ( other._update, nullptr );

    if ( otherLock )
        other.Unlock ();

    if ( locked )
        Unlock ();

    return *this;
}

GizmoNode::GizmoNode ( Workspace &workspace, GizmoInfo &gizmoInfo, UpdateHandler &&update ) noexcept:
    WorkspaceNode ( workspace ),
    _gizmoInfo ( &gizmoInfo ),
    _update ( std::move ( update ) )
{
    _gizmoInfo->_node = this;
}

GizmoNode::~GizmoNode () noexcept
{
    if ( !TryLock () ) [[unlikely]]
        return;

    _workspace->Unregister ( *this );
    _workspace = nullptr;
    Unlock ();
}

void GizmoNode::Commit ( GXVec3 const &viewerLocation, GXVec3 const &viewerForward, GXVec3 const &viWorld ) noexcept
{
    if ( !_hasChanges || !TryLock () ) [[likely]]
        return;

    GizmoInfo &gizmoInfo = *_gizmoInfo;
    _update ( gizmoInfo._vertex, gizmoInfo._pixel, viewerLocation, viewerForward, viWorld );
    _hasChanges = false;
    Unlock ();
}

void GizmoNode::MarkUpdate () noexcept
{
    if ( TryLock () ) [[likely]]
    {
        _hasChanges = true;
        Unlock ();
    }
}

void GizmoNode::Disconnect () noexcept
{
    if ( _workspace )
    {
        _workspace->Unregister ( *this );
    }
}

} // namespace editor
