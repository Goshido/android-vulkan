#ifndef EDITOR_GIZMO_NODE_HPP
#define EDITOR_GIZMO_NODE_HPP


#include "gizmo_info.hpp"
#include "workspace_node.hpp"


namespace editor {

class Workspace;

class GizmoNode final : public WorkspaceNode
{
    friend class Workspace;

    private:
        GizmoInfo*      _gizmoInfo = nullptr;
        GXQuat          _rotation {};
        GXVec3          _location {};
        GXVec3          _scale {};
        uint8_t         _palette;

    public:
        GizmoNode () = delete;

        GizmoNode ( GizmoNode const & ) = delete;
        GizmoNode &operator = ( GizmoNode const & ) = delete;

        GizmoNode ( GizmoNode &&other ) noexcept;
        GizmoNode &operator = ( GizmoNode &&other ) noexcept;

        explicit GizmoNode ( Workspace &workspace, GizmoInfo &gizmoInfo ) noexcept;

        ~GizmoNode () noexcept override;

        void Commit ( GXVec3 const &cameraLocation, GXVec3 const &viWorld ) noexcept;

        void SetColor ( uint8_t palette ) noexcept;
        void SetRotation ( GXQuat const &rotation ) noexcept;
        void SetLocation ( GXVec3 const &location ) noexcept;
        void SetScale ( GXVec3 const &scale ) noexcept;

    private:
        void Disconnect () noexcept;
};

} // namespace editor


#endif // EDITOR_GIZMO_NODE_HPP
