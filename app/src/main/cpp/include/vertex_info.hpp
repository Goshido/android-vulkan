#ifndef ANDROID_VULKAN_VERTEX_INFO_HPP
#define ANDROID_VULKAN_VERTEX_INFO_HPP


#include <GXCommon/GXMath.hpp>


namespace android_vulkan {

#pragma pack ( push, 1 )

struct PositionInfo final
{
    GXVec3      _position {};

    PositionInfo () = default;

    constexpr PositionInfo ( float x, float y, float z ) noexcept:
        _position ( x, y, z )
    {
        // NOTHING
    }

    constexpr explicit PositionInfo ( GXVec3 const &position ) noexcept:
        _position ( position )
    {
        // NOTHING
    }

    ~PositionInfo () = default;
};

struct VertexInfo final
{
    GXVec2      _uv {};
    GXVec3      _normal {};
    GXVec3      _tangent {};
    GXVec3      _bitangent {};

    VertexInfo () = default;

    constexpr VertexInfo ( GXVec2 const &uv,
        GXVec3 const &normal,
        GXVec3 const &tangent,
        GXVec3 const &bitangent
    ) noexcept:
        _uv ( uv._data[ 0U ], uv._data[ 1U ] ),
        _normal ( normal._data[ 0U ], normal._data[ 1U ], normal._data[ 2U ] ),
        _tangent ( tangent._data[ 0U ], tangent._data[ 1U ], tangent._data[ 2U ] ),
        _bitangent ( bitangent._data[ 0U ], bitangent._data[ 1U ], bitangent._data[ 2U ] )
    {
        // NOTHING
    }

    ~VertexInfo () = default;
};

#pragma pack ( pop )

} // namespace android_vulkan


#endif // ANDROID_VULKAN_VERTEX_INFO_HPP
