#ifndef ANDROID_VULKAN_MESH_VERTEX_INFO_H
#define ANDROID_VULKAN_MESH_VERTEX_INFO_H


#include <GXCommon/GXMath.h>


namespace android_vulkan {

#pragma pack ( push, 1 )

struct VertexInfo final
{
    GXVec3      _vertex;
    GXVec2      _uv;
    GXVec3      _normal;
    GXVec3      _tangent;
    GXVec3      _bitangent;

    VertexInfo () = default;

    constexpr VertexInfo ( const GXVec3 &vertex,
        const GXVec2 &uv,
        const GXVec3 &normal,
        const GXVec3 &tangent,
        const GXVec3 &bitangent
    ):
        _vertex ( vertex._data[ 0U ], vertex._data[ 1U ], vertex._data[ 2U ] ),
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


#endif // ANDROID_VULKAN_MESH_VERTEX_INFO_H
