struct VolumeData
{
    matrix          _transform;
};

[[vk::push_constant]]
const VolumeData    g_volumeData;

struct InputData
{
    [[vk::location ( 0 )]]
    float3          _vertex:    VERTEX;
};

//----------------------------------------------------------------------------------------------------------------------

linear float4 VS ( in InputData inputData ): SV_Position
{
    return mul ( g_volumeData._transform, float4 ( inputData._vertex, 1.0F ) );
}
