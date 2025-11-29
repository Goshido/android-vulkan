#ifndef EDITOR_MESH_NODE_HPP
#define EDITOR_MESH_NODE_HPP


#include "mesh_info.hpp"
#include "workspace_node.hpp"


namespace editor {

class Workspace;

class MeshNode final : public WorkspaceNode
{
    private:
        MeshInfo*       _internal = nullptr;
        MeshInfo        _meshInfo {};

    public:
        MeshNode () = default;

        MeshNode ( MeshNode const & ) = delete;
        MeshNode &operator = ( MeshNode const & ) = delete;

        MeshNode ( MeshNode &&other ) noexcept;
        MeshNode &operator = ( MeshNode &&other ) noexcept;

        explicit MeshNode ( Workspace &workspace, MeshInfo &internal ) noexcept;

        ~MeshNode () noexcept override;

        void Commit () noexcept;
        [[nodiscard]] MeshInfo const &GetInternalInfo () const noexcept;

        void SetColor ( GXColorUNORM color0,
            GXColorUNORM color1,
            GXColorUNORM color2,
            GXColorUNORM emission,
            float emissionIntensity
        ) noexcept;

        void SetLocal ( GXMat4 const &local, GXAABB const &localBounds ) noexcept;
        void SetMaterial ( PBRMaterial const &material ) noexcept;
};

} // namespace editor


#endif // EDITOR_MESH_NODE_HPP
