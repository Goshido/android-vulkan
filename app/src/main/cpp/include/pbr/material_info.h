#ifndef PBR_MATERIAL_INFO_H
#define PBR_MATERIAL_INFO_H


#include <GXCommon/GXWarning.h>

GX_DISABLE_COMMON_WARNINGS

#include <cinttypes>

GX_RESTORE_WARNING_STATE


namespace pbr {

constexpr uint64_t const MATERIAL_NO_TEXTURE = 0U;

#pragma pack ( push, 1 )

struct MaterialHeader final
{
    uint64_t        _diffuseOffset;
    uint64_t        _emissionOffset;
    uint64_t        _normalOffset;
    uint64_t        _paramOffset;

    float           _roughnessScale;
};

#pragma pack ( pop )

} // namespace pbr


#endif // PBR_MATERIAL_INFO_H
