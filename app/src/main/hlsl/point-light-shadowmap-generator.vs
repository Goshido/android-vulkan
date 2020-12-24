struct InputData
{
    [[vk::location ( 0 )]]
    float3                  _vertex:            VERTEX;

    uint                    _instanceIndex:     SV_InstanceID;
};

struct OutputData
{
    linear float4           _vertex:            SV_Position;

    [[vk::location ( 0 )]]
    nointerpolation uint    _instanceIndex:     INSTANCE;
};

OutputData VS ( in InputData inputData )
{
    OutputData result;

    result._vertex = float4 ( inputData._vertex, 1.0F );
    result._instanceIndex = inputData._instanceIndex;

    return result;
}
