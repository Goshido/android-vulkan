#include <precompiled_headers.hpp>
#include <av_assert.hpp>
#include <workspace_node.hpp>


namespace editor {

WorkspaceNode::WorkspaceNode ( Workspace &workspace ) noexcept:
    _workspace ( &workspace )
{
    // NOTHING
}

void WorkspaceNode::Lock () noexcept
{
    bool expected = false;

    while ( !_lock.compare_exchange_weak ( expected, true ) ) [[unlikely]]
        expected = false;

    AV_ASSERT ( !expected )
}

} // namespace editor
