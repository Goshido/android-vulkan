#ifndef PBR_STATIC_MESH_COMPONENT_DESC_H
#define PBR_STATIC_MESH_COMPONENT_DESC_H


#include "component_desc.h"
#include "primitive_types.h"


namespace pbr {

#pragma pack ( push, 1 )

struct StaticMeshComponentDesc final : public ComponentDesc
{
    UTF8Offset      _material;
    UTF8Offset      _mesh;

    ColorUnorm      _color0;
    ColorUnorm      _color1;
    ColorUnorm      _color2;
    ColorUnorm      _color3;

    Mat4x4          _localMatrix;
};

#pragma pack ( pop )

} // namespace pbr


#endif // PBR_STATIC_MESH_COMPONENT_DESC_H
