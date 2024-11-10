#ifndef OBJECT_DATA_INC
#define OBJECT_DATA_INC


#include "geometry_pass_bindings.inc"
#include "gpgpu_limits.inc"


// In summary data per instance = 144 bytes
// Total instance data = 8064 bytes, less than minimum "Supported Limit" of maxUniformBufferRange
// see https://registry.khronos.org/vulkan/specs/1.3-extensions/html/chap47.html


[[vk::binding ( BIND_INSTANCE_POSITON_DATA, SET_INSTANCE_DATA )]]
cbuffer InstancePositionData:       register ( b0 )
{
    float32_t4x4    g_localViewProj[ PBR_OPAQUE_MAX_INSTANCE_COUNT ];
}

[[vk::binding ( BIND_INSTANCE_NORMAL_DATA, SET_INSTANCE_DATA )]]
cbuffer InstanceNormalData:         register ( b1 )
{
    float32_t4      g_localView[ PBR_OPAQUE_MAX_INSTANCE_COUNT ];
}

struct ColorData
{
    float32_t4      _color0;
    float32_t4      _color1;
    float32_t4      _color2;
    float32_t4      _emission;
};

[[vk::binding ( BIND_INSTANCE_COLOR_DATA, SET_INSTANCE_DATA )]]
cbuffer InstanceColorData:          register ( b2 )
{
    ColorData       g_colorData[ PBR_OPAQUE_MAX_INSTANCE_COUNT ];
}


#endif // OBJECT_DATA_INC
