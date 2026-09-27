#ifndef GEOMETRY_PASS_ATTRIBUTES_HLSL
#define GEOMETRY_PASS_ATTRIBUTES_HLSL


#include "platform/android/pbr/geometry_pass_binds.inc"


struct Attributes
{
    [[vk::location ( ATT_SLOT_UV )]]
    linear float32_t2           _uv:                UV;

    [[vk::location ( ATT_SLOT_TANGENT_VIEW )]]
    linear float32_t3           _tangentView:       TANGENT;

    [[vk::location ( ATT_SLOT_BITANGENT_VIEW )]]
    linear float32_t3           _bitangentView:     BITANGENT;

    [[vk::location ( ATT_SLOT_NORMAL_VIEW )]]
    linear float32_t3           _normalView:        NORMAL;

    [[vk::location ( ATT_SLOT_INSTANCE_INDEX )]]
    nointerpolation uint32_t    _instanceIndex:     INSTANCE_INDEX;

    linear float4               _vertexH:           SV_Position;
};


#endif // GEOMETRY_PASS_ATTRIBUTES_HLSL
