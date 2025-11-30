#include <precompiled_headers.hpp>
#include <av_assert.hpp>
#include <workspace_node.hpp>


namespace editor {

WorkspaceNode::WorkspaceNode ( Workspace &workspace ) noexcept:
    _workspace ( &workspace )
{
    // NOTHING
}

bool WorkspaceNode::TryLock () noexcept
{
    bool expected = false;

    while ( !_lock.compare_exchange_weak ( expected, true ) ) [[unlikely]]
        expected = false;

    AV_ASSERT ( !expected )

    if ( _workspace ) [[likely]]
        return true;

    _lock.store ( false );
    return false;
}

void WorkspaceNode::Unlock () noexcept
{
    _lock.store ( false );
}

} // namespace editor
