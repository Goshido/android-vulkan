#ifndef OBJECT_DATA_HLSL
#define OBJECT_DATA_HLSL


#include "color_packing.hlsl"
#include "platform/android/pbr/geometry_pass_binds.inc"
#include "platform/android/pbr/gpgpu_limits.inc"
#include "quat64.hlsl"


// In summary data per instance = 88 bytes
// Total instance data = 7392 bytes, less than minimum "Supported Limit" of maxUniformBufferRange
// see https://registry.khronos.org/vulkan/specs/1.3-extensions/html/chap47.html


[[vk::binding ( BIND_INSTANCE_POSITION_DATA, SET_INSTANCE_DATA )]]
cbuffer InstancePositionData:       register ( b0 )
{
    float32_t4x4    g_localViewProj[ PBR_OPAQUE_MAX_INSTANCE_COUNT ];
};

[[vk::binding ( BIND_INSTANCE_NORMAL_DATA, SET_INSTANCE_DATA )]]
cbuffer InstanceNormalData:         register ( b1 )
{
    Quat64x2        g_localView[ PBR_OPAQUE_MAX_INSTANCE_COUNT / 2U ];
};

[[vk::binding ( BIND_INSTANCE_COLOR_DATA, SET_INSTANCE_DATA )]]
cbuffer InstanceColorData:          register ( b2 )
{
    ColorData       g_colorData[ PBR_OPAQUE_MAX_INSTANCE_COUNT ];
}


#endif // OBJECT_DATA_HLSL
