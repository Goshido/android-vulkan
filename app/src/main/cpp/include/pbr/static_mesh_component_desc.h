#ifndef PBR_STATIC_MESH_COMPONENT_DESC_H
#define PBR_STATIC_MESH_COMPONENT_DESC_H


#include "component_desc.h"
#include "primitive_types.h"


namespace pbr {

#pragma pack ( push, 1 )

struct StaticMeshComponentDesc final : public ComponentDesc
{
    android_vulkan::UTF8Offset      _material;
    android_vulkan::UTF8Offset      _mesh;

    android_vulkan::ColorUnorm      _color0;
    android_vulkan::ColorUnorm      _color1;
    android_vulkan::ColorUnorm      _color2;
    android_vulkan::ColorUnorm      _color3;

    android_vulkan::Mat4x4          _localMatrix;
};

#pragma pack ( pop )

} // namespace pbr


#endif // PBR_STATIC_MESH_COMPONENT_DESC_H
