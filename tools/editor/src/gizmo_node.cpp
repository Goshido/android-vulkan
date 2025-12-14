#include <precompiled_headers.hpp>
#include <gizmo_node.hpp>
#include <workspace.hpp>


namespace editor {

GizmoNode::GizmoNode ( GizmoNode &&other ) noexcept
{
    std::ignore = other.TryLock ();

    _workspace = std::exchange ( other._workspace, nullptr );
    _hasChanges = std::move ( other._hasChanges );
    _internal = std::exchange ( other._internal, nullptr );
    _info = std::move ( other._info );

    other.Unlock ();
}

GizmoNode &GizmoNode::operator = ( GizmoNode &&other ) noexcept
{
    if ( this == &other || !other.TryLock () ) [[unlikely]]
        return *this;

    _workspace = std::exchange ( other._workspace, nullptr );
    _hasChanges = std::move ( other._hasChanges );
    _internal = std::exchange ( other._internal, nullptr );
    _info = std::move ( other._info );

    other.Unlock ();
    return *this;
}

GizmoNode::GizmoNode ( Workspace &workspace, GizmoInfo &internal ) noexcept:
    WorkspaceNode ( workspace ),
    _internal ( &internal )
{
    // NOTHING
}

GizmoNode::~GizmoNode () noexcept
{
    if ( !TryLock () ) [[unlikely]]
        return;

    _workspace->Unregister ( *this );
    _workspace = nullptr;
    Unlock ();
}

void GizmoNode::Commit () noexcept
{
    if ( !_hasChanges || !TryLock () ) [[likely]]
        return;

    *_internal = _info;
    _hasChanges = false;

    Unlock ();
}

GizmoInfo const &GizmoNode::GetInternalInfo () const noexcept
{
    AV_ASSERT ( _internal )
    return *_internal;
}

} // namespace editor
