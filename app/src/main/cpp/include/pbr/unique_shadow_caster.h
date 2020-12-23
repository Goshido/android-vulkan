#ifndef PBR_UNIQUE_SHADOW_CASTER_H
#define PBR_UNIQUE_SHADOW_CASTER_H


#include "types.h"


namespace pbr {

struct UniqueShadowCaster final
{
    [[maybe_unused]] MeshRef    _mesh;
    [[maybe_unused]] GXMat4     _transform;
};

} // namespace pbr


#endif // PBR_UNIQUE_SHADOW_CASTER_H
