#include <precompiled_headers.hpp>
#include <opaque_mesh_node.hpp>
#include <workspace.hpp>


namespace editor {

OpaqueMeshNode::OpaqueMeshNode ( OpaqueMeshNode &&other ) noexcept:
    _bounds ( std::move ( other._bounds ) ),
    _local ( std::move ( other._local ) ),
    _workspace ( std::move ( other._workspace ) ),
    _hasChanges ( std::move ( other._hasChanges ) )
{
    _lock.store ( other._lock.load () );
}

OpaqueMeshNode &OpaqueMeshNode::operator = ( OpaqueMeshNode &&other ) noexcept
{
    if ( this == &other ) [[unlikely]]
        return *this;

    _bounds = std::move ( other._bounds );
    _local = std::move ( other._local );
    _workspace = std::move ( other._workspace );
    _hasChanges = std::move ( other._hasChanges );
    _lock.store ( other._lock.load () );

    return *this;
}

OpaqueMeshNode::OpaqueMeshNode ( Workspace &workspace ) noexcept:
    _workspace ( &workspace )
{
    // NOTHING
}

OpaqueMeshNode::~OpaqueMeshNode () noexcept
{
    if ( _workspace ) [[likely]]
    {
        _workspace->Unregister ( *this );
    }
}

void OpaqueMeshNode::Commit ( GXMat4 &local, GXAABB &bounds ) noexcept
{
    if ( !_hasChanges ) [[likely]]
        return;

    Lock ();

    local = _local;
    bounds = _bounds;
    _hasChanges = false;

    _lock.store ( false );
}

void OpaqueMeshNode::SetColor ( GXColorUNORM /*color0*/,
    GXColorUNORM /*color1*/,
    GXColorUNORM /*color2*/,
    GXColorUNORM /*emission*/,
    float /*emissionIntensity*/
) noexcept
{
    // FUCK
}

void OpaqueMeshNode::SetLocal ( GXMat4 const &local, GXAABB const &localBounds ) noexcept
{
    Lock ();

    _local = local;
    localBounds.Transform ( _bounds, local );
    _hasChanges = true;

    _lock.store ( false );
}

void OpaqueMeshNode::SetMaterial () noexcept
{
    // FUCK
}

void OpaqueMeshNode::Lock () noexcept
{
    bool expected = false;

    while ( !_lock.compare_exchange_weak ( expected, true ) ) [[unlikely]]
        expected = false;

    AV_ASSERT ( !expected )
}

} // namespace editor
