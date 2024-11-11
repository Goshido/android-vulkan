#ifndef ANDROID_VULKAN_VERTEX_INFO_HPP
#define ANDROID_VULKAN_VERTEX_INFO_HPP


#include <GXCommon/GXMath.hpp>


namespace android_vulkan {

#pragma pack ( push, 1 )

struct VertexInfo final
{
    GXVec2      _uv {};
    GXVec3      _normal {};
    GXVec3      _tangent {};
    GXVec3      _bitangent {};
};

#pragma pack ( pop )

} // namespace android_vulkan


#endif // ANDROID_VULKAN_VERTEX_INFO_HPP
