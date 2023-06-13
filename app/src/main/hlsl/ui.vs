[[vk::binding ( 0, 0 )]]
cbuffer Transform:                                  register ( b0 )
{
    float32_t4x4                    _transform;
};

struct InputData
{
    [[vk::location ( 0 )]]
    float32_t2                      _vertex:        VERTEX;

    [[vk::location ( 1 )]]
    float32_t4                      _color:         COLOR;

    [[vk::location ( 2 )]]
    float32_t2                      _atlasUV:       ATLAS;

    [[vk::location ( 3 )]]
    float32_t2                      _imageUV:       IMAGE;
};

struct OutputData
{
    linear float32_t4               _vertexH:       SV_Position;

    [[vk::location ( 0 )]]
    nointerpolation float32_t4      _color:         COLOR;

    [[vk::location ( 1 )]]
    noperspective float32_t2        _atlasUV:       ATLAS;

    [[vk::location ( 2 )]]
    noperspective float32_t2        _imageUV:       IMAGE;
};

//----------------------------------------------------------------------------------------------------------------------

OutputData VS ( in InputData inputData )
{
    OutputData result;

    result._vertexH = mul ( _transform, float32_t4 ( inputData._vertex, 0.5F, 1.0F ) );

    result._color = inputData._color;
    result._atlasUV = inputData._atlasUV;
    result._imageUV = inputData._imageUV;

    return result;
}
