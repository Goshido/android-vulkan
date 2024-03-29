#ifndef OBJECT_DATA_INC
#define OBJECT_DATA_INC


#include "geometry_pass_bindings.inc"
#include "gpgpu_limits.inc"


struct ObjectData
{
    float32_t4x4    _localView;
    float32_t4x4    _localViewProjection;
    float32_t4      _color0;
    float32_t4      _color1;
    float32_t4      _color2;
    float32_t4      _emission;
};

[[vk::binding ( BIND_INSTANCE_DATA, SET_INSTANCE_DATA )]]
cbuffer InstanceData:       register ( b0 )
{
    // sizeof ( ObjectData ) = 192 bytes
    // sizeof ( InstanceData ) = 8064 bytes, less than minimum "Supported Limit"
    // see https://vulkan.lunarg.com/doc/view/1.1.108.0/mac/chunked_spec/chap36.html#limits-minmax
    ObjectData      g_instanceData[ PBR_OPAQUE_MAX_INSTANCE_COUNT ];
}


#endif // OBJECT_DATA_INC
