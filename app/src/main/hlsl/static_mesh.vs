#include "rotating_mesh/bindings.inc"


[[vk::binding ( BIND_TRANSFORM, SET_ONCE )]]
cbuffer Transform:                              register ( b0 )
{
    float32_t4x4            _transform;
    float32_t4x4            _normalTransform;
};

struct InputData
{
    [[vk::location ( IN_SLOT_POSITION )]]
    float32_t3              _position:          POSITION;

    [[vk::location ( IN_SLOT_UV )]]
    float32_t2              _uv:                UV;

    [[vk::location ( IN_SLOT_NORMAL )]]
    float32_t3              _normal:            NORMAL;

    [[vk::location ( IN_SLOT_TANGENT )]]
    float32_t3              _tangent:           TANGENT;
};

struct OutputData
{
    linear float32_t4       _vertexH:           SV_Position;

    [[vk::location ( ATT_SLOT_FRAGMENT_VIEW )]]
    linear float32_t3       _fragmentView:      FRAGMENT;

    [[vk::location ( ATT_SLOT_UV )]]
    linear float32_t2       _uv:                UV;

    [[vk::location ( ATT_SLOT_NORMAL_VIEW )]]
    linear float32_t3       _normalView:        NORMAL;

    [[vk::location ( ATT_SLOT_TANGENT_VIEW )]]
    linear float32_t3       _tangentView:       TANGENT;

    [[vk::location ( ATT_SLOT_BITANGENT_VIEW )]]
    linear float32_t3       _bitangentView:     BITANGENT;
};

//----------------------------------------------------------------------------------------------------------------------

OutputData VS ( in InputData inputData )
{
    float32_t4 const vertex = float32_t4 ( inputData._position, 1.0F );

    OutputData result;
    result._vertexH = mul ( _transform, vertex );

    float32_t3x3 const normalTransform = (float32_t3x3)_normalTransform;
    result._fragmentView = ( mul ( _normalTransform, vertex ) ).xyz;
    result._uv = inputData._uv;

    float32_t3 const normalView = mul ( normalTransform, inputData._normal );
    float32_t3 const tangentView = mul ( normalTransform, inputData._tangent );

    result._normalView = normalView;
    result._tangentView = tangentView;
    result._bitangentView = cross ( normalView, tangentView );

    return result;
}
