#ifndef EDITOR_OPAQUE_MESH_NODE_HPP
#define EDITOR_OPAQUE_MESH_NODE_HPP


#include <GXCommon/GXMath.hpp>
#include "pbr_material.hpp"
#include <vulkan_utils.hpp>


namespace editor {

class Workspace;

AV_DX_ALIGNMENT_BEGIN

struct ColorData final
{
    uint32_t    _emiRcol0rgb = 0U;
    uint32_t    _emiGcol1rgb = 0U;
    uint32_t    _emiBcol2rgb = 0U;
    uint32_t    _col0aEmiIntens = 0U;
};

AV_DX_ALIGNMENT_END

class OpaqueMeshNode final
{
    private:
        GXAABB              _bounds {};
        ColorData           _color {};
        GXMat4              _local {};
        PBRMaterial         _material {};

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


#endif // EDITOR_OPAQUE_MESH_NODE_HPP
