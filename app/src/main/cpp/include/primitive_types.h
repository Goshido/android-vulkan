#ifndef PBR_PRIMITIVE_TYPES_H
#define PBR_PRIMITIVE_TYPES_H


#include <GXCommon/GXWarning.h>

GX_DISABLE_COMMON_WARNINGS

#include <cinttypes>

GX_RESTORE_WARNING_STATE


namespace android_vulkan {

#pragma pack ( push, 1 )

using Mat4x4 = float[ 16U ];
using Vec2 = float[ 2U ];
using Vec3 = float[ 3U ];

using UTF8Offset = uint64_t;
constexpr UTF8Offset const NO_UTF8_OFFSET = 0U;

struct ColorUnorm final
{
    uint8_t     _red;
    uint8_t     _green;
    uint8_t     _blue;
    uint8_t     _alpha;
};

struct AABB final
{
    Vec3        _min;
    Vec3        _max;
};

#pragma pack ( pop )

} // namespace android_vulkan


#endif // PBR_PRIMITIVE_TYPES_H
