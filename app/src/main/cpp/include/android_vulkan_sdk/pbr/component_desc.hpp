#ifndef PBR_COMPONENT_DESC_HPP
#define PBR_COMPONENT_DESC_HPP


#include "component_classes.hpp"
#include <android_vulkan_sdk/primitive_types.hpp>


namespace pbr {

#pragma pack ( push, 1 )

struct ComponentDesc
{
    ClassID                         _classID;
    uint32_t                        _formatVersion;
    android_vulkan::UTF8Offset      _name;
};

#pragma pack ( pop )

} // namespace pbr


#endif // PBR_COMPONENT_DESC_HPP
