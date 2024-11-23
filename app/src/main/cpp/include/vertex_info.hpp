#ifndef ANDROID_VULKAN_VERTEX_INFO_HPP
#define ANDROID_VULKAN_VERTEX_INFO_HPP


#include <GXCommon/GXMath.hpp>


namespace android_vulkan {

#pragma pack ( push, 1 )

struct VertexInfo final
{
    // Non-mirrored identity quaternion.
    constexpr static uint32_t IDENTITY_TBN = 0xC000'0000U;

    GXVec2      _uv {};
    uint32_t    _tbn = 0U;
};

#pragma pack ( pop )

} // namespace android_vulkan


#endif // ANDROID_VULKAN_VERTEX_INFO_HPP
