#ifndef PBR_MATERIAL_INFO_H
#define PBR_MATERIAL_INFO_H


#include <primitive_types.h>


namespace pbr {

#pragma pack ( push, 1 )

struct MaterialHeader final
{
    android_vulkan::UTF8Offset      _diffuseOffset;
    android_vulkan::UTF8Offset      _emissionOffset;
    android_vulkan::UTF8Offset      _maskOffset;
    android_vulkan::UTF8Offset      _normalOffset;
    android_vulkan::UTF8Offset      _paramOffset;

    float                           _roughnessScale;
};

#pragma pack ( pop )

} // namespace pbr


#endif // PBR_MATERIAL_INFO_H
