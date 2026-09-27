#include <precompiled_headers.hpp>
#include <reflection_probe_local_node.hpp>
#include <workspace.hpp>


namespace editor {

ReflectionProbeLocalNode::ReflectionProbeLocalNode ( ReflectionProbeLocalNode &&other ) noexcept
{
    std::ignore = other.TryLock ();

    _workspace = std::exchange ( other._workspace, nullptr );
    _hasChanges = std::move ( other._hasChanges );
    _internal = std::exchange ( other._internal, nullptr );
    _info = std::move ( other._info );

    other.Unlock ();
}

ReflectionProbeLocalNode &ReflectionProbeLocalNode::operator = ( ReflectionProbeLocalNode &&other ) noexcept
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

ReflectionProbeLocalNode::ReflectionProbeLocalNode ( Workspace &workspace,
    ReflectionProbeLocalInfo &internal
) noexcept:
    WorkspaceNode ( workspace ),
    _internal ( &internal )
{
    // NOTHING
}

ReflectionProbeLocalNode::~ReflectionProbeLocalNode () noexcept
{
    if ( !TryLock () ) [[unlikely]]
        return;

    _workspace->Unregister ( *this );
    _workspace = nullptr;
    Unlock ();
}

void ReflectionProbeLocalNode::Commit () noexcept
{
    if ( !_hasChanges || !TryLock () ) [[likely]]
        return;

    *_internal = _info;
    _hasChanges = false;

    Unlock ();
}

ReflectionProbeLocalInfo &ReflectionProbeLocalNode::GetInternalInfo () noexcept
{
    AV_ASSERT ( _internal )
    return *_internal;
}

void ReflectionProbeLocalNode::SetIntensity ( float intensity ) noexcept
{
    if ( !TryLock () ) [[unlikely]]
        return;

    _info._intensity = intensity;
    _hasChanges = true;

    Unlock ();
}

void ReflectionProbeLocalNode::SetRadius ( float radius ) noexcept
{
    if ( !TryLock () ) [[unlikely]]
        return;

    _info._radius = radius;
    _hasChanges = true;

    Unlock ();
}

} // namespace editor
