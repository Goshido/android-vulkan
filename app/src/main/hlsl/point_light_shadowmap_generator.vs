#include "pbr/gpgpu_limits.inc"
#include "pbr/point_light_shadowmap_generator.inc"


struct ObjectData
{
    float32_t4x4    _transform[ PBR_POINT_LIGHT_SHADOW_CASTER_PROJECTION_COUNT ];
};

[[vk::binding ( BIND_INSTANCE_DATA, SET_INSTANCE_DATA )]]
cbuffer InstanceData:               register ( b0 )
{
    // sizeof ( ObjectData ) = 384 bytes
    // sizeof ( InstanceData ) = 16128 bytes, less than minimum "Supported Limit"
    // see https://vulkan.lunarg.com/doc/view/1.1.108.0/mac/chunked_spec/chap36.html#limits-minmax
    ObjectData      g_instanceData[ PBR_POINT_LIGHT_MAX_SHADOW_CASTER_INSTANCE_COUNT ];
};

struct InputData
{
    [[vk::location ( IN_SLOT_VERTEX )]]
    float32_t3      _vertex:        VERTEX;

    uint32_t        _instanceID:    SV_InstanceID;
    uint32_t        _viewID:        SV_ViewID;
};

linear float32_t4 VS ( in InputData inputData ): SV_Position
{
    return mul ( g_instanceData[ inputData._instanceID ]._transform[ inputData._viewID ],
        float32_t4 ( inputData._vertex, 1.0F )
    );
}
