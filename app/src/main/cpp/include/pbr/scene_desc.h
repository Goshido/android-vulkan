#ifndef PBR_SCENE_DESC_H
#define PBR_SCENE_DESC_H


#include "primitive_types.h"


namespace pbr {

#pragma pack ( push, 1 )

struct SceneDesc final
{
    uint32_t    _formatVersion;
    uint64_t    _componentCount;
    uint64_t    _textureCount;
    uint64_t    _meshCount;
    Mat4x4      _viewerLocal;
};

#pragma pack ( pop )

} // namespace pbr


#endif // PBR_SCENE_DESC_H
