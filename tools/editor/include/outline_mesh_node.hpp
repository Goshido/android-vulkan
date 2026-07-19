#ifndef EDITOR_OUTLINE_MESH_NODE_HPP
#define EDITOR_OUTLINE_MESH_NODE_HPP


#include "outline_mesh_info.hpp"
#include "workspace_node.hpp"


namespace editor {

class Workspace;

class OutlineMeshNode final : public WorkspaceNode
{
    friend class Workspace;

    private:
        OutlineMeshInfo*    _meshInfo = nullptr;
        GXQuat              _rotation {};
        GXVec3              _location {};
        GXVec3              _scale {};
        GXAABB              _boundLocal {};

    public:
        OutlineMeshNode () = default;

        OutlineMeshNode ( OutlineMeshNode const & ) = delete;
        OutlineMeshNode &operator = ( OutlineMeshNode const & ) = delete;

        OutlineMeshNode ( OutlineMeshNode &&other ) noexcept;
        OutlineMeshNode &operator = ( OutlineMeshNode &&other ) noexcept;

        explicit OutlineMeshNode ( Workspace &workspace, OutlineMeshInfo &meshInfo ) noexcept;

        ~OutlineMeshNode () noexcept override;

        void Commit () noexcept;

        void SetRotation ( GXQuat const &rotation ) noexcept;
        void SetRotation ( GXMat3 const &rotation ) noexcept;
        void SetRotation ( GXMat4 const &rotation ) noexcept;

        void SetLocation ( GXVec3 const &location ) noexcept;
        void SetScale ( GXVec3 const &scale ) noexcept;

        void SetLocal ( GXMat4 const &local ) noexcept;
        void SetLocal ( GXQuat const &rotation, GXVec3 const &location ) noexcept;
        void SetLocal ( GXQuat const &rotation, GXVec3 const &location, GXVec3 const &scale ) noexcept;

        void SetBounds ( GXAABB const &boundLocal ) noexcept;

    private:
        void Disconnect () noexcept;
};

} // namespace editor


#endif // EDITOR_OUTLINE_MESH_NODE_HPP
