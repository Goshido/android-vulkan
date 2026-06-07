#ifndef EDITOR_MESH_NODE_HPP
#define EDITOR_MESH_NODE_HPP


#include "mesh_info.hpp"
#include "workspace_node.hpp"


namespace editor {

class Workspace;

class MeshNode final : public WorkspaceNode
{
    public:
        // FUCK - use this instead MeshInfo
        struct RenderInfo final
        {
            PBRMaterial     _material {};
            GXMat4          _local {};
            GXAABB          _boundWorld {};
            ColorData       _color {};
        };

    private:
        MeshInfo*           _meshInfo = nullptr;
        RenderInfo          _renderInfo {};

    public:
        MeshNode () = default;

        MeshNode ( MeshNode const & ) = delete;
        MeshNode &operator = ( MeshNode const & ) = delete;

        MeshNode ( MeshNode &&other ) noexcept;
        MeshNode &operator = ( MeshNode &&other ) noexcept;

        explicit MeshNode ( Workspace &workspace, MeshInfo &meshInfo ) noexcept;

        ~MeshNode () noexcept override;

        void Commit () noexcept;
        [[nodiscard]] MeshInfo &GetMeshInfo () const noexcept;
        [[nodiscard]] RenderInfo const &GetRenderInfo () const noexcept;

        void SetColor ( GXColorUNORM color0,
            GXColorUNORM color1,
            GXColorUNORM color2,
            GXColorUNORM emission,
            float emissionIntensity
        ) noexcept;

        void SetRotation ( GXQuat const &rotation ) noexcept;
        void SetRotation ( GXMat3 const &rotation ) noexcept;
        void SetRotation ( GXMat4 const &rotation ) noexcept;

        void SetLocation ( GXVec3 const &location ) noexcept;
        void SetScale ( GXVec3 const &scale ) noexcept;

        void SetLocal ( GXMat4 const &local ) noexcept;
        void SetLocal ( GXQuat const &rotation, GXVec3 const &location ) noexcept;
        void SetLocal ( GXQuat const &rotation, GXVec3 const &location, GXVec3 const &scale ) noexcept;

        void SetBounds ( GXAABB const &boundLocal ) noexcept;
        void SetMaterial ( PBRMaterial const &material ) noexcept;
};

} // namespace editor


#endif // EDITOR_MESH_NODE_HPP
