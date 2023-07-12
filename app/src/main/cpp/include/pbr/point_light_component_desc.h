#ifndef PBR_POINT_LIGHT_COMPONENT_DESC_HPP
#define PBR_POINT_LIGHT_COMPONENT_DESC_HPP


#include "component_desc.h"
#include "primitive_types.h"


namespace pbr {

#pragma pack ( push, 1 )

struct PointLightComponentDesc final : public ComponentDesc
{
    android_vulkan::AABB            _bounds;
    android_vulkan::ColorUnorm      _hue;
    float                           _intensity;
    android_vulkan::Vec3            _location;
};

#pragma pack ( pop )

} // namespace pbr


#endif // PBR_POINT_LIGHT_COMPONENT_DESC_HPP
