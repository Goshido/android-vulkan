#ifndef OBJECT_DATA_HLSL
#define OBJECT_DATA_HLSL


#include "pbr/geometry_pass_binds.inc"
#include "pbr/gpgpu_limits.inc"
#include "tbn64.hlsl"


// In summary data per instance = 88 bytes
// Total instance data = 7392 bytes, less than minimum "Supported Limit" of maxUniformBufferRange
// see https://registry.khronos.org/vulkan/specs/1.3-extensions/html/chap47.html


[[vk::binding ( BIND_INSTANCE_POSITON_DATA, SET_INSTANCE_DATA )]]
cbuffer InstancePositionData:       register ( b0 )
{
    float32_t4x4    g_localViewProj[ PBR_OPAQUE_MAX_INSTANCE_COUNT ];
}

[[vk::binding ( BIND_INSTANCE_NORMAL_DATA, SET_INSTANCE_DATA )]]
cbuffer InstanceNormalData:         register ( b1 )
{
    TBN64           g_localView[ PBR_OPAQUE_MAX_INSTANCE_COUNT / 2U ];
}

struct ColorData
{
    uint32_t        _emiRcol0rgb;
    uint32_t        _emiGcol1rgb;
    uint32_t        _emiBcol2rgb;
    uint32_t        _col0aEmiIntens;
};

[[vk::binding ( BIND_INSTANCE_COLOR_DATA, SET_INSTANCE_DATA )]]
cbuffer InstanceColorData:          register ( b2 )
{
    ColorData       g_colorData[ PBR_OPAQUE_MAX_INSTANCE_COUNT ];
}


#endif // OBJECT_DATA_HLSL
