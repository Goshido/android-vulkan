#ifndef EDITOR_WORKSPACE_NODE_HPP
#define EDITOR_WORKSPACE_NODE_HPP


#include <atomic>


namespace editor {

class Workspace;

class WorkspaceNode
{
    friend Workspace;

    protected:
        Workspace*          _workspace = nullptr;
        std::atomic_bool    _lock = false;
        bool                _hasChanges = true;

    public:
        WorkspaceNode () = default;

        WorkspaceNode ( WorkspaceNode const & ) = delete;
        WorkspaceNode &operator = ( WorkspaceNode const & ) = delete;

        WorkspaceNode ( WorkspaceNode && ) = delete;
        WorkspaceNode &operator = ( WorkspaceNode && ) = delete;

    protected:
        explicit WorkspaceNode ( Workspace &workspace ) noexcept;
        virtual ~WorkspaceNode () = default;

        [[nodiscard]] bool TryLock () noexcept;
        void Unlock () noexcept;
};

} // namespace editor


#endif // EDITOR_WORKSPACE_NODE_HPP
