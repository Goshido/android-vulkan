#ifndef EDITOR_MESH_NODE_HPP
#define EDITOR_MESH_NODE_HPP


#include "mesh_info.hpp"


namespace editor {

class Workspace;

class MeshNode final
{
    private:
        MeshInfo const*     _internal = nullptr;
        Workspace*          _workspace = nullptr;
        MeshInfo            _meshInfo {};
        std::atomic_bool    _lock = false;
        bool                _hasChanges = true;

    public:
        MeshNode () = default;

        MeshNode ( MeshNode const & ) = delete;
        MeshNode &operator = ( MeshNode const & ) = delete;

        MeshNode ( MeshNode &&other ) noexcept;
        MeshNode &operator = ( MeshNode &&other ) noexcept;

        explicit MeshNode ( Workspace &workspace, MeshInfo const &internal ) noexcept;

        ~MeshNode () noexcept;

        [[nodiscard]] MeshInfo const &GetInternalMeshInfo () const noexcept;
        void Commit ( GXMat4 &local, GXAABB &bounds, ColorData &color, PBRMaterial &material ) noexcept;

        void SetColor ( GXColorUNORM color0,
            GXColorUNORM color1,
            GXColorUNORM color2,
            GXColorUNORM emission,
            float emissionIntensity
        ) noexcept;

        void SetLocal ( GXMat4 const &local, GXAABB const &localBounds ) noexcept;
        void SetMaterial ( PBRMaterial const &material ) noexcept;

    private:
        void Lock () noexcept;
};

} // namespace editor


#endif // EDITOR_MESH_NODE_HPP
