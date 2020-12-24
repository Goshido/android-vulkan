#include "gpgpu_limits.inc"


#define VERTICES_PER_TRIANGLE                   3U

struct ObjectData
{
    matrix                  _transform[ PBR_POINT_LIGHT_SHADOW_CASTER_PROJECTION_COUNT ];
};

[[vk::binding ( 0, 0 )]]
cbuffer InstanceData:                           register ( b0 )
{
    // sizeof ( ObjectData ) = 384 bytes
    // sizeof ( InstanceData ) = 16128 bytes, less than minimum "Supported Limit"
    // see https://vulkan.lunarg.com/doc/view/1.1.108.0/mac/chunked_spec/chap36.html#limits-minmax
    ObjectData              g_instanceData[ PBR_POINT_LIGHT_MAX_SHADOW_CASTER_INSTANCE_COUNT ];
};

struct InputData
{
    linear float4           _vertex:            SV_Position;

    [[vk::location ( 0 )]]
    nointerpolation uint    _instanceIndex:     INSTANCE;
};

struct OutputData
{
    linear float4           _vertex:            SV_Position;
    nointerpolation uint    _renderTarget:      SV_RenderTargetArrayIndex;
};

[maxvertexcount ( VERTICES_PER_TRIANGLE * PBR_POINT_LIGHT_SHADOW_CASTER_PROJECTION_COUNT )]
void GS ( triangle InputData inputData[ VERTICES_PER_TRIANGLE ], inout TriangleStream<OutputData> outputData )
{
    // All input triangle vertices have same "_instanceIndex" by design.
    const uint instanceIndex = inputData[ 0U ]._instanceIndex;

    [unroll]
    for ( uint target = 0U; target < PBR_POINT_LIGHT_SHADOW_CASTER_PROJECTION_COUNT; ++target )
    {
        OutputData vertexInfo;
        vertexInfo._renderTarget = target;
        const matrix transform = g_instanceData[ instanceIndex ]._transform[ target ];

        [unroll]
        for ( uint ind = 0U; ind < VERTICES_PER_TRIANGLE; ++ind )
        {
            vertexInfo._vertex = mul ( transform, inputData[ ind ]._vertex );
            outputData.Append ( vertexInfo );
        }

        outputData.RestartStrip ();
    }
}
