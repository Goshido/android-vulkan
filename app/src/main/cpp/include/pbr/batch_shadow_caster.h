#ifndef PBR_BATCH_SHADOW_CASTER_H
#define PBR_BATCH_SHADOW_CASTER_H


#include "types.h"


namespace pbr {

struct BatchShadowCaster final
{
    [[maybe_unused]] MeshRef                _mesh;
    [[maybe_unused]] std::vector<GXMat4>    _transforms;
};

} // namespace pbr


#endif // PBR_BATCH_SHADOW_CASTER_H
