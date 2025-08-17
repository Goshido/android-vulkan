#ifndef OBJECT_DATA_HLSL
#define OBJECT_DATA_HLSL


#include "pbr/geometry_pass_binds.inc"
#include "pbr/gpgpu_limits.inc"
#include "tbn64.hlsl"


// In summary data per instance = 88 bytes
// Total instance data = 7392 bytes, less than minimum "Supported Limit" of maxUniformBufferRange
// see https://registry.khronos.org/vulkan/specs/1.3-extensions/html/chap47.html


[[vk::binding ( BIND_INSTANCE_POSITION_DATA, SET_INSTANCE_DATA )]]
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
    uint32_t        _emiR: 8;

    uint32_t        _col0R: 8;
    uint32_t        _col0G: 8;
    uint32_t        _col0B: 8;

    uint32_t        _emiG: 8;

    uint32_t        _col1R: 8;
    uint32_t        _col1G: 8;
    uint32_t        _col1B: 8;

    uint32_t        _emiB: 8;

    uint32_t        _col2R: 8;
    uint32_t        _col2G: 8;
    uint32_t        _col2B: 8;

    uint32_t        _col0A: 8;

    uint32_t        _emiIntens: 24;
};

[[vk::binding ( BIND_INSTANCE_COLOR_DATA, SET_INSTANCE_DATA )]]
cbuffer InstanceColorData:          register ( b2 )
{
    ColorData       g_colorData[ PBR_OPAQUE_MAX_INSTANCE_COUNT ];
}


#endif // OBJECT_DATA_HLSL
