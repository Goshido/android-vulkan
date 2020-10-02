#ifndef PBR_PRIMITIVE_TYPES_H
#define PBR_PRIMITIVE_TYPES_H


#include <GXCommon/GXWarning.h>

GX_DISABLE_COMMON_WARNINGS

#include <cinttypes>

GX_RESTORE_WARNING_STATE


namespace pbr {

#pragma pack ( push, 1 )

struct ColorUnorm final
{
    uint8_t     _red;
    uint8_t     _green;
    uint8_t     _blue;
    uint8_t     _alpha;
};

struct Mat4x4 final
{
    float       _data[ 16U ];
};

using UTF8Offset = uint64_t;
constexpr UTF8Offset const NO_UTF8_OFFSET = 0U;

#pragma pack ( pop )

} // namespace pbr


#endif // PBR_PRIMITIVE_TYPES_H
