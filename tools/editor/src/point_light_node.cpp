#include <precompiled_headers.hpp>
#include <point_light_node.hpp>
#include <workspace.hpp>


namespace editor {

PointLightNode::PointLightNode ( PointLightNode &&other ) noexcept
{
    std::ignore = other.TryLock ();

    _workspace = std::exchange ( other._workspace, nullptr );
    _hasChanges = std::move ( other._hasChanges );
    _internal = std::exchange ( other._internal, nullptr );
    _info = std::move ( other._info );

    other.Unlock ();
}

PointLightNode &PointLightNode::operator = ( PointLightNode &&other ) noexcept
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

PointLightNode::PointLightNode ( Workspace &workspace, PointLightInfo &internal ) noexcept:
    WorkspaceNode ( workspace ),
    _internal ( &internal )
{
    // NOTHING
}

PointLightNode::~PointLightNode () noexcept
{
    if ( !TryLock () ) [[unlikely]]
        return;

    _workspace->Unregister ( *this );
    _workspace = nullptr;
    Unlock ();
}

void PointLightNode::Commit () noexcept
{
    if ( !_hasChanges || !TryLock () ) [[likely]]
        return;

    *_internal = _info;
    _hasChanges = false;

    Unlock ();
}

PointLightInfo &PointLightNode::GetInternalInfo () noexcept
{
    AV_ASSERT ( _internal )
    return *_internal;
}

void PointLightNode::SetColor ( GXColorUNORM color ) noexcept
{
    if ( !TryLock () ) [[unlikely]]
        return;

    _info._color = color;
    _hasChanges = true;

    Unlock ();
}

void PointLightNode::SetIntensity ( float intensity ) noexcept
{
    if ( !TryLock () ) [[unlikely]]
        return;

    _info._intensity = intensity;
    _hasChanges = true;

    Unlock ();
}

void PointLightNode::SetLocation ( GXVec3 const &location ) noexcept
{
    if ( !TryLock () ) [[unlikely]]
        return;

    _info._location = location;
    _hasChanges = true;

    Unlock ();
}

void PointLightNode::SetRadius ( float radius ) noexcept
{
    if ( !TryLock () ) [[unlikely]]
        return;

    _info._radius = radius;
    _hasChanges = true;

    Unlock ();
}

} // namespace editor
