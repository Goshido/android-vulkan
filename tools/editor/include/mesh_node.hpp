#ifndef EDITOR_MESH_NODE_HPP
#define EDITOR_MESH_NODE_HPP


#include "mesh_info.hpp"
#include "pbr_material.hpp"
#include "workspace_node.hpp"


namespace editor {

class Workspace;

class MeshNode final : public WorkspaceNode
{
    friend class Workspace;

    private:
        MeshInfo*       _meshInfo = nullptr;
        PBRMaterial     _material {};
        ColorData       _colors {};
        GXQuat          _rotation {};
        GXVec3          _location {};
        GXVec3          _scale {};
        GXAABB          _boundLocal {};

    public:
        MeshNode () = default;

        MeshNode ( MeshNode const & ) = delete;
        MeshNode &operator = ( MeshNode const & ) = delete;

        MeshNode ( MeshNode &&other ) noexcept;
        MeshNode &operator = ( MeshNode &&other ) noexcept;

        explicit MeshNode ( Workspace &workspace, MeshInfo &meshInfo ) noexcept;

        ~MeshNode () noexcept override;

        void Commit ( uint32_t defaultAlbedo,
            uint32_t defaultEmission,
            uint32_t defaultMask,
            uint32_t defaultParam,
            uint32_t defaultNormal
        ) noexcept;

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
