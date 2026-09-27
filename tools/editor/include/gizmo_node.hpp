#ifndef EDITOR_GIZMO_NODE_HPP
#define EDITOR_GIZMO_NODE_HPP


#include "gizmo_info.hpp"
#include "sdf_palette.hpp"
#include "workspace_node.hpp"

GX_DISABLE_COMMON_WARNINGS

#include <functional>

GX_RESTORE_WARNING_STATE


namespace editor {

class Workspace;

class GizmoNode final : public WorkspaceNode
{
    friend class Workspace;

    public:
        using UpdateHandler = std::move_only_function<
            void ( SDFVertex &, SDFPixel &, SDFShape &, GXVec3 const &, GXMat3 const &, GXVec3 const & )
        >;

    private:
        GizmoInfo*          _gizmoInfo = nullptr;
        UpdateHandler       _update = nullptr;

    public:
        GizmoNode () = default;

        GizmoNode ( GizmoNode const & ) = delete;
        GizmoNode &operator = ( GizmoNode const & ) = delete;

        GizmoNode ( GizmoNode &&other ) noexcept;
        GizmoNode &operator = ( GizmoNode &&other ) noexcept;

        explicit GizmoNode ( Workspace &workspace, GizmoInfo &gizmoInfo, UpdateHandler &&update ) noexcept;

        ~GizmoNode () noexcept override;

        void Commit ( GXVec3 const &cameraLocation, GXMat3 const &cameraBasis, GXVec3 const &viWorld ) noexcept;
        void MarkUpdate () noexcept;

    private:
        void Disconnect () noexcept;
};

} // namespace editor


#endif // EDITOR_GIZMO_NODE_HPP
