#ifndef PBR_ACTOR_DESC_H
#define PBR_ACTOR_DESC_H


#include "primitive_types.h"


namespace pbr {

#pragma pack ( push, 1 )

struct ActorDesc final
{
    uint32_t                        _formatVersion;
    android_vulkan::UTF8Offset      _name;
    uint32_t                        _components;
    android_vulkan::Mat4x4          _localMatrix;
};

#pragma pack ( pop )

} // namespace pbr


#endif // PBR_ACTOR_DESC_H
