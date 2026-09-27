#include <precompiled_headers.hpp>
#include <reflection_probe_global_node.hpp>
#include <workspace.hpp>


namespace editor {

ReflectionProbeGlobalNode::ReflectionProbeGlobalNode ( ReflectionProbeGlobalNode &&other ) noexcept
{
    std::ignore = other.TryLock ();

    _workspace = std::exchange ( other._workspace, nullptr );
    _hasChanges = std::move ( other._hasChanges );
    _internal = std::exchange ( other._internal, nullptr );
    _info = std::move ( other._info );

    other.Unlock ();
}

ReflectionProbeGlobalNode &ReflectionProbeGlobalNode::operator = ( ReflectionProbeGlobalNode &&other ) noexcept
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

ReflectionProbeGlobalNode::ReflectionProbeGlobalNode ( Workspace &workspace,
    ReflectionProbeGlobalInfo &internal
) noexcept:
    WorkspaceNode ( workspace ),
    _internal ( &internal )
{
    // NOTHING
}

ReflectionProbeGlobalNode::~ReflectionProbeGlobalNode () noexcept
{
    if ( !TryLock () ) [[unlikely]]
        return;

    _workspace->Unregister ( *this );
    _workspace = nullptr;
    Unlock ();
}

void ReflectionProbeGlobalNode::Commit () noexcept
{
    if ( !_hasChanges || !TryLock () ) [[likely]]
        return;

    *_internal = _info;
    _hasChanges = false;

    Unlock ();
}

ReflectionProbeGlobalInfo &ReflectionProbeGlobalNode::GetInternalInfo () noexcept
{
    AV_ASSERT ( _internal )
    return *_internal;
}

void ReflectionProbeGlobalNode::SetIntensity ( float intensity ) noexcept
{
    if ( !TryLock () ) [[unlikely]]
        return;

    _info._intensity = intensity;
    _hasChanges = true;

    Unlock ();
}

} // namespace editor
