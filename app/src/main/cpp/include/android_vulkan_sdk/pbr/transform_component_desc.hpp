#ifndef PBR_TRANSFORM_COMPONENT_DESC_HPP
#define PBR_TRANSFORM_COMPONENT_DESC_HPP


#include "component_desc.hpp"
#include <android_vulkan_sdk/primitive_types.hpp>


namespace pbr {

#pragma pack ( push, 1 )

struct TransformComponentDesc final : public ComponentDesc
{
    android_vulkan::Mat4x4      _localMatrix;
};

#pragma pack ( pop )

} // namespace pbr


#endif // PBR_TRANSFORM_COMPONENT_DESC_HPP
