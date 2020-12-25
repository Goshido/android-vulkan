#include "gpgpu_limits.inc"


struct ObjectData
{
    matrix                  _transform[ PBR_POINT_LIGHT_SHADOW_CASTER_PROJECTION_COUNT ];
};

[[vk::binding ( 0, 0 )]]
cbuffer InstanceData:                       register ( b0 )
{
    // sizeof ( ObjectData ) = 384 bytes
    // sizeof ( InstanceData ) = 16128 bytes, less than minimum "Supported Limit"
    // see https://vulkan.lunarg.com/doc/view/1.1.108.0/mac/chunked_spec/chap36.html#limits-minmax
    ObjectData              g_instanceData[ PBR_POINT_LIGHT_MAX_SHADOW_CASTER_INSTANCE_COUNT ];
};

struct InputData
{
    [[vk::location ( 0 )]]
    float3                  _vertex:        VERTEX;

    uint                    _instanceID:    SV_InstanceID;
    uint                    _viewID:        SV_ViewID;
};

linear float4 VS ( in InputData inputData ): SV_Position
{
    return mul ( g_instanceData[ inputData._instanceID ]._transform[ inputData._viewID ],
        float4 ( inputData._vertex, 1.0F )
    );
}
