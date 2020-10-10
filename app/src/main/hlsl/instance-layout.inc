#include "gpgpu_limits.inc"


struct ObjectData
{
    matrix          _localView;
    matrix          _localViewProjection;
    float4          _color0;
    float4          _color1;
    float4          _color2;
    float4          _color3;
};

[[vk::binding ( 0, 1 )]]
cbuffer InstanceData:       register ( b0 )
{
    // sizeof ( ObjectData ) = 192 bytes
    // sizeof ( InstanceData ) = 8064 bytes
    ObjectData      g_instanceData[ PBR_OPAQUE_MAX_INSTANCE_COUNT ];
}
