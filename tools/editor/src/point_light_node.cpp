#include <precompiled_headers.hpp>
#include <point_light_node.hpp>
#include <workspace.hpp>


namespace editor {

PointLightNode::PointLightNode ( PointLightNode &&other ) noexcept
{
    other.Lock ();

    _workspace = std::exchange ( other._workspace, nullptr );
    _hasChanges = std::move ( other._hasChanges );
    _internal = std::exchange ( other._internal, nullptr );
    _lightInfo = std::move ( other._lightInfo );

    other._lock.store ( false );
}

PointLightNode &PointLightNode::operator = ( PointLightNode &&other ) noexcept
{
    if ( this == &other ) [[unlikely]]
        return *this;

    other.Lock ();

    _workspace = std::exchange ( other._workspace, nullptr );
    _hasChanges = std::move ( other._hasChanges );
    _internal = std::exchange ( other._internal, nullptr );
    _lightInfo = std::move ( other._lightInfo );

    other._lock.store ( false );
    return *this;
}

PointLightNode::PointLightNode ( Workspace &workspace, PointLightInfo &internal ) noexcept:
    WorkspaceNode ( workspace ),
    _internal ( &internal )
{
    // NOTHING
}

PointLightNode::~PointLightNode () noexcept
{
    if ( _workspace ) [[likely]]
    {
        _workspace->Unregister ( *this );
    }
}

PointLightInfo &PointLightNode::GetInternalInfo () noexcept
{
    AV_ASSERT ( _internal )
    return *_internal;
}

void PointLightNode::Commit () noexcept
{
    if ( !_hasChanges ) [[likely]]
        return;

    Lock ();

    *_internal = _lightInfo;
    _hasChanges = false;

    _lock.store ( false );
}

} // namespace editor
