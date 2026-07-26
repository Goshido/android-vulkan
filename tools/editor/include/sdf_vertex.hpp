#ifndef EDITOR_SDF_VERTEX_HPP
#define EDITOR_SDF_VERTEX_HPP


#include "model.hpp"


namespace editor {

AV_DX_ALIGNMENT_BEGIN

struct SDFVertex final
{
    uint64_t    _sdfOrientation;
    Model       _toWorld;
    GXVec3      _sdfOffset;
    uint32_t    _pad0;
};

AV_DX_ALIGNMENT_END

} // namespace editor


#endif // EDITOR_SDF_VERTEX_HPP
