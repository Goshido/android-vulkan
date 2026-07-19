#include <precompiled_headers.hpp>
#include <outline_mesh_node.hpp>
#include <workspace.hpp>


namespace editor {

OutlineMeshNode::OutlineMeshNode ( OutlineMeshNode &&other ) noexcept
{
    bool const myLock = TryLock ();
    Disconnect ();

    bool const otherLock = other.TryLock ();

    _workspace = std::exchange ( other._workspace, nullptr );
    _hasChanges = std::exchange ( other._hasChanges, false );

    if ( _meshInfo = std::exchange ( other._meshInfo, nullptr ); _meshInfo )
        _meshInfo->_node = this;

    _rotation = std::exchange ( other._rotation, GXQuat::IDENTITY );
    _location = std::exchange ( other._location, GXVec3::ZERO );
    _scale = std::exchange ( other._scale, GXVec3::ONE );
    _boundLocal = std::exchange ( other._boundLocal, {} );

    if ( otherLock )
        other.Unlock ();

    if ( myLock )
    {
        Unlock ();
    }
}

OutlineMeshNode &OutlineMeshNode::operator = ( OutlineMeshNode &&other ) noexcept
{
    if ( this == &other ) [[unlikely]]
        return *this;

    bool const locked = TryLock ();
    Disconnect ();

    bool const otherLock = other.TryLock ();
    _workspace = std::exchange ( other._workspace, nullptr );
    _hasChanges = std::exchange ( other._hasChanges, false );

    if ( _meshInfo = std::exchange ( other._meshInfo, nullptr ); _meshInfo )
        _meshInfo->_node = this;

    _rotation = std::exchange ( other._rotation, GXQuat::IDENTITY );
    _location = std::exchange ( other._location, GXVec3::ZERO );
    _scale = std::exchange ( other._scale, GXVec3::ONE );
    _boundLocal = std::exchange ( other._boundLocal, {} );

    if ( otherLock )
        other.Unlock ();

    if ( locked )
        Unlock ();

    return *this;
}

OutlineMeshNode::OutlineMeshNode ( Workspace &workspace, OutlineMeshInfo &meshInfo ) noexcept:
    WorkspaceNode ( workspace ),
    _meshInfo ( &meshInfo )
{
    meshInfo._node = this;
}

OutlineMeshNode::~OutlineMeshNode () noexcept
{
    std::ignore = TryLock ();
    Disconnect ();
}

void OutlineMeshNode::Commit () noexcept
{
    if ( !_hasChanges || !TryLock () ) [[likely]]
        return;

    OutlineMeshInfo &meshInfo = *_meshInfo;

    GXMat4 local {};
    local.FromFast ( _rotation, _location );
    auto &x = *reinterpret_cast<GXVec3*> ( local._data[ 0U ] );
    GXVec3 const &s = _scale;

    auto &y = *reinterpret_cast<GXVec3*> ( local._data[ 1U ] );
    x.Multiply ( x, s._data[ 0U ] );

    auto &z = *reinterpret_cast<GXVec3*> ( local._data[ 2U ] );
    y.Multiply ( y, s._data[ 1U ] );
    z.Multiply ( z, s._data[ 2U ] );

    meshInfo._model =
    {
        ._x = x,
        ._y = y,
        ._z = z,
        ._w = *reinterpret_cast<GXVec3*> ( local._data[ 3U ] )
    };

    _boundLocal.Transform ( meshInfo._boundWorld, local );
    _hasChanges = false;
    Unlock ();
}

void OutlineMeshNode::SetRotation ( GXQuat const &rotation ) noexcept
{
    if ( !TryLock () ) [[unlikely]]
        return;

    _rotation = rotation;
    _hasChanges = true;

    Unlock ();
}

void OutlineMeshNode::SetRotation ( GXMat3 const &rotation ) noexcept
{
    GXQuat const r ( rotation );

    if ( !TryLock () ) [[unlikely]]
        return;

    _rotation = r;
    _hasChanges = true;

    Unlock ();
}

void OutlineMeshNode::SetRotation ( GXMat4 const &rotation ) noexcept
{
    GXQuat const r ( rotation );

    if ( !TryLock () ) [[unlikely]]
        return;

    _rotation = r;
    _hasChanges = true;

    Unlock ();
}

void OutlineMeshNode::SetLocation ( GXVec3 const &location ) noexcept
{
    if ( !TryLock () ) [[unlikely]]
        return;

    _location = location;
    _hasChanges = true;

    Unlock ();
}

void OutlineMeshNode::SetScale ( GXVec3 const &scale ) noexcept
{
    if ( !TryLock () ) [[unlikely]]
        return;

    _scale = scale;
    _hasChanges = true;

    Unlock ();
}

void OutlineMeshNode::SetLocal ( GXMat4 const &local ) noexcept
{
    GXQuat const r ( local );

    GXVec3 s {};
    local.ClearScale ( s );

    if ( !TryLock () ) [[unlikely]]
        return;

    _rotation = r;
    _location = *reinterpret_cast<GXVec3 const*> ( local._data[ 3U ] );
    _scale = s;
    _hasChanges = true;

    Unlock ();
}

void OutlineMeshNode::SetLocal ( GXQuat const &rotation, GXVec3 const &location ) noexcept
{
    if ( !TryLock () ) [[unlikely]]
        return;

    _rotation = rotation;
    _location = location;
    _hasChanges = true;

    Unlock ();
}

void OutlineMeshNode::SetLocal ( GXQuat const &rotation, GXVec3 const &location, GXVec3 const &scale ) noexcept
{
    if ( !TryLock () ) [[unlikely]]
        return;

    _rotation = rotation;
    _location = location;
    _scale = scale;
    _hasChanges = true;

    Unlock ();
}

void OutlineMeshNode::SetBounds ( GXAABB const &boundLocal ) noexcept
{
    _boundLocal = boundLocal;
    _hasChanges = true;
}

void OutlineMeshNode::Disconnect () noexcept
{
    if ( _workspace )
    {
        _workspace->UnregisterOutline ( *this );
    }
}

} // namespace editor
