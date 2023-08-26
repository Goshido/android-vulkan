[[vk::binding ( 0 )]]
cbuffer Transform:                              register ( b0 )
{
    float32_t4x4            _transform;
    float32_t4x4            _normalTransform;
};

struct InputData
{
    [[vk::location ( 0 )]]
    float32_t3              _vertex:            VERTEX;

    [[vk::location ( 1 )]]
    float32_t2              _uv:                UV;

    [[vk::location ( 2 )]]
    float32_t3              _normal:            NORMAL;

    [[vk::location ( 3 )]]
    float32_t3              _tangent:           TANGENT;

    [[vk::location ( 4 )]]
    float32_t3              _bitangent:         BITANGENT;
};

struct OutputData
{
    linear float32_t4       _vertexH:           SV_Position;

    [[vk::location ( 0 )]]
    linear float32_t3       _fragmentView:      FRAGMENT;

    [[vk::location ( 1 )]]
    linear float32_t2       _uv:                UV;

    [[vk::location ( 2 )]]
    linear float32_t3       _normalView:        NORMAL;

    [[vk::location ( 3 )]]
    linear float32_t3       _tangentView:       TANGENT;

    [[vk::location ( 4 )]]
    linear float32_t3       _bitangentView:     BITANGENT;
};

//----------------------------------------------------------------------------------------------------------------------

OutputData VS ( in InputData inputData )
{
    float32_t4 const vertex = float32_t4 ( inputData._vertex, 1.0F );

    OutputData result;
    result._vertexH = mul ( _transform, vertex );

    float32_t3x3 const normalTransform = (float32_t3x3)_normalTransform;
    result._fragmentView = ( mul ( _normalTransform, vertex ) ).xyz;
    result._uv = inputData._uv;

    result._normalView = mul ( normalTransform, inputData._normal );
    result._tangentView = mul ( normalTransform, inputData._tangent );
    result._bitangentView = mul ( normalTransform, inputData._bitangent );

    return result;
}
