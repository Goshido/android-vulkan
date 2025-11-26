#ifndef EDITOR_OPAQUE_MESH_NODE_HPP
#define EDITOR_OPAQUE_MESH_NODE_HPP


#include <GXCommon/GXMath.hpp>


namespace editor {

class Workspace;

class OpaqueMeshNode final
{
    private:
        GXAABB              _bounds {};
        GXMat4              _local {};

        Workspace*          _workspace = nullptr;
        std::atomic_bool    _lock = false;
        bool                _hasChanges = true;

    public:
        OpaqueMeshNode () = default;

        OpaqueMeshNode ( OpaqueMeshNode const & ) = delete;
        OpaqueMeshNode &operator = ( OpaqueMeshNode const & ) = delete;

        OpaqueMeshNode ( OpaqueMeshNode &&other ) noexcept;
        OpaqueMeshNode &operator = ( OpaqueMeshNode &&other ) noexcept;

        explicit OpaqueMeshNode ( Workspace &workspace ) noexcept;

        ~OpaqueMeshNode () noexcept;

        void Commit ( GXMat4 &local, GXAABB &bounds ) noexcept;

        void SetColor ( GXColorUNORM color0,
            GXColorUNORM color1,
            GXColorUNORM color2,
            GXColorUNORM emission,
            float emissionIntensity
        ) noexcept;

        void SetLocal ( GXMat4 const &local, GXAABB const &localBounds ) noexcept;
        void SetMaterial () noexcept;

    private:
        void Lock () noexcept;
};

} // namespace editor


#endif // EDITOR_OPAQUE_MESH_NODE_HPP
