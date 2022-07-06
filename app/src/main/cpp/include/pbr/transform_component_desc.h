#ifndef PBR_TRANSFORM_COMPONENT_DESC_H
#define PBR_TRANSFORM_COMPONENT_DESC_H


#include "component_desc.h"
#include "primitive_types.h"


namespace pbr {

#pragma pack ( push, 1 )

struct TransformComponentDesc final : public ComponentDesc
{
    android_vulkan::Mat4x4      _localMatrix;
};

#pragma pack ( pop )

} // namespace pbr


#endif // PBR_TRANSFORM_COMPONENT_DESC_H
