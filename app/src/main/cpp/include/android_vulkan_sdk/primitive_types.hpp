#ifndef PBR_PRIMITIVE_TYPES_HPP
#define PBR_PRIMITIVE_TYPES_HPP


#include <GXCommon/GXWarning.hpp>

GX_DISABLE_COMMON_WARNINGS

#include <cstdint>

GX_RESTORE_WARNING_STATE


namespace android_vulkan {

#pragma pack ( push, 1 )

using Boolean = uint8_t;
[[maybe_unused]] constexpr static Boolean AV_TRUE = 1U;
[[maybe_unused]] constexpr static Boolean AV_FALSE = 0U;

using Mat4x4 = float[ 16U ];
using HVec2 = uint16_t[ 2U ];
using Vec3 = float[ 3U ];

// Mapping for indices:
//      0 - r
//      1 - a
//      2 - b
//      3 - c
//
// q = r + ai + bj + ck
using Quat = float[ 4U ];

using UTF8Offset = uint64_t;
constexpr static UTF8Offset NO_UTF8_OFFSET = 0U;

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

enum class eSoundChannel : uint8_t
{
    Music [[maybe_unused]] = 0U,
    Ambient [[maybe_unused]] = 1U,
    SFX [[maybe_unused]] = 2U,
    Speech [[maybe_unused]] = 3U
};

#pragma pack ( pop )

} // namespace android_vulkan


#endif // PBR_PRIMITIVE_TYPES_HPP
