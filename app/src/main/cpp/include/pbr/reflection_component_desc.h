#ifndef PBR_REFLECTION_COMPONENT_DESC_H
#define PBR_REFLECTION_COMPONENT_DESC_H


#include "component_desc.h"
#include "primitive_types.h"


namespace pbr {

#pragma pack ( push, 1 )

struct ReflectionComponentDesc final : public ComponentDesc
{
    android_vulkan::UTF8Offset      _sideXPlus;
    android_vulkan::UTF8Offset      _sideXMinus;
    android_vulkan::UTF8Offset      _sideYPlus;
    android_vulkan::UTF8Offset      _sideYMinus;
    android_vulkan::UTF8Offset      _sideZPlus;
    android_vulkan::UTF8Offset      _sideZMinus;

    android_vulkan::Vec3            _location;

    // size equals FLT_MAX means global reflection probe.
    float                           _size;
};

#pragma pack ( pop )


} // namespace pbr 


#endif // PBR_REFLECTION_COMPONENT_DESC_H
