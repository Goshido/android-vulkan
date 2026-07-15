#ifndef EDITOR_MESH_INFO_HPP
#define EDITOR_MESH_INFO_HPP


#include "shading.hpp"
#include "transform.hpp"

GX_DISABLE_COMMON_WARNINGS

#include <bit>

GX_RESTORE_WARNING_STATE


namespace editor {

class MeshNode;

enum class eMaterial : uint8_t
{
    Opaque,
    Outline,
    Stipple
};

struct MeshInfo final
{
    MeshNode*       _node = nullptr;
    Transform       _transform {};
    GXAABB          _boundWorld {};
    Shading         _shading {};
    uint64_t        _id = std::bit_cast<uint64_t> ( nullptr );
    eMaterial       _material = eMaterial::Opaque;
};

} // namespace editor


#endif // EDITOR_MESH_INFO_HPP
