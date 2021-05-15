[[vk::binding ( 0, 1 )]]
cbuffer VolumeData:         register ( b1 )
{
    matrix      _transform;
};

struct InputData
{
    [[vk::location ( 0 )]]
    float3      _vertex:    VERTEX;
};

//----------------------------------------------------------------------------------------------------------------------

linear float4 VS ( in InputData inputData ): SV_Position
{
    return mul ( _transform, float4 ( inputData._vertex, 1.0F ) );
}
