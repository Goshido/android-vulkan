#include "gpgpu_limits.inc"


struct ObjectData
{
    matrix      _localView;
    matrix      _localViewProjection;
    half4       _color0;
    half4       _color1;
    half4       _color2;
    half4       _color3;
};

[[vk::binding ( 0, 1 )]]
cbuffer InstanceData:       register ( b0 )
{
    // sizeof ( ObjectData ) = 160 bytes
    // sizeof ( InstanceData ) = 6720 bytes
    ObjectData      g_instanceData[ PBR_OPAQUE_MAX_INSTANCE_COUNT ];
}
