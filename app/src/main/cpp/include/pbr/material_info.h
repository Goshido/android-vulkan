#ifndef PBR_MATERIAL_INFO_H
#define PBR_MATERIAL_INFO_H


#include "primitive_types.h"


namespace pbr {

#pragma pack ( push, 1 )

struct MaterialHeader final
{
    UTF8Offset      _diffuseOffset;
    UTF8Offset      _emissionOffset;
    UTF8Offset      _maskOffset;
    UTF8Offset      _normalOffset;
    UTF8Offset      _paramOffset;

    float           _roughnessScale;
};

#pragma pack ( pop )

} // namespace pbr


#endif // PBR_MATERIAL_INFO_H
