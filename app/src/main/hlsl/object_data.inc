#ifndef OBJECT_DATA_INC
#define OBJECT_DATA_INC


#include "geometry_pass_bindings.inc"
#include "gpgpu_limits.inc"


struct ObjectData
{
    float32_t4      _localViewQuat;
    float32_t4x4    _localViewProjection;
    float32_t4      _color0;
    float32_t4      _color1;
    float32_t4      _color2;
    float32_t4      _emission;
};

[[vk::binding ( BIND_INSTANCE_DATA, SET_INSTANCE_DATA )]]
cbuffer InstanceData:       register ( b0 )
{
    // sizeof ( ObjectData ) = 144 bytes
    // sizeof ( InstanceData ) = 8064 bytes, less than minimum "Supported Limit" of maxUniformBufferRange
    // see https://registry.khronos.org/vulkan/specs/1.3-extensions/html/chap47.html
    ObjectData      g_instanceData[ PBR_OPAQUE_MAX_INSTANCE_COUNT ];
}


#endif // OBJECT_DATA_INC
