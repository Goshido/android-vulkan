#ifndef EDITOR_GBUFFER_MESH_INFO_HPP
#define EDITOR_GBUFFER_MESH_INFO_HPP


#include "shading.hpp"
#include "transform.hpp"

GX_DISABLE_COMMON_WARNINGS

#include <bit>

GX_RESTORE_WARNING_STATE


namespace editor {

class GBufferMeshNode;

enum class eMaterial : uint8_t
{
    Opaque,
    Stipple
};

struct GBufferMeshInfo final
{
    GBufferMeshNode*    _node = nullptr;
    Transform           _transform {};
    GXAABB              _boundWorld {};
    Shading             _shading {};
    uint64_t            _id = std::bit_cast<uint64_t> ( nullptr );
    eMaterial           _material = eMaterial::Opaque;
};

} // namespace editor


#endif // EDITOR_GBUFFER_MESH_INFO_HPP
