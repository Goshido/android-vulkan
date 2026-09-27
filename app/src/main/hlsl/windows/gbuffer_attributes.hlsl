#ifndef GBUFFER_ATTRIBUTES_HLSL
#define GBUFFER_ATTRIBUTES_HLSL


#include "windows/gbuffer_attribute_slots.hlsl"


struct Attributes
{
    linear float32_t4           _vertexH:           SV_Position;

    [[vk::location ( ATT_SLOT_UV )]]
    linear float32_t2           _uv:                UV;

    [[vk::location ( ATT_SLOT_TANGENT_VIEW )]]
    linear float32_t3           _tangentView:       TANGENT;

    [[vk::location ( ATT_SLOT_BITANGENT_VIEW )]]
    linear float32_t3           _bitangentView:     BITANGENT;

    [[vk::location ( ATT_SLOT_NORMAL_VIEW )]]
    linear float32_t3           _normalView:        NORMAL;

    [[vk::location ( ATT_SLOT_INSTANCE_ID )]]
    nointerpolation uint32_t    _instanceID:        INSTANCE_ID;
};


#endif // GBUFFER_ATTRIBUTES_HLSL
