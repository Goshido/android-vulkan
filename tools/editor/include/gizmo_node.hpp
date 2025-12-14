#ifndef EDITOR_GIZMO_NODE_HPP
#define EDITOR_GIZMO_NODE_HPP


#include "gizmo_info.hpp"
#include "workspace_node.hpp"


namespace editor {

class Workspace;

class GizmoNode final : public WorkspaceNode
{
    private:
        GizmoInfo*      _internal = nullptr;
        GizmoInfo       _info {};

    public:
        GizmoNode () = default;

        GizmoNode ( GizmoNode const & ) = delete;
        GizmoNode &operator = ( GizmoNode const & ) = delete;

        GizmoNode ( GizmoNode &&other ) noexcept;
        GizmoNode &operator = ( GizmoNode &&other ) noexcept;

        explicit GizmoNode ( Workspace &workspace, GizmoInfo &internal ) noexcept;

        ~GizmoNode () noexcept override;

        void Commit () noexcept;
        [[nodiscard]] GizmoInfo const &GetInternalInfo () const noexcept;
};

} // namespace editor


#endif // EDITOR_GIZMO_NODE_HPP
