[[vk::binding ( 0, 1 )]]
cbuffer VolumeData:             register ( b1 )
{
    float32_t4x4    _transform;
};

struct InputData
{
    [[vk::location ( 0 )]]
    float32_t3      _vertex:    VERTEX;
};

//----------------------------------------------------------------------------------------------------------------------

linear float32_t4 VS ( in InputData inputData ): SV_Position
{
    return mul ( _transform, float32_t4 ( inputData._vertex, 1.0F ) );
}
