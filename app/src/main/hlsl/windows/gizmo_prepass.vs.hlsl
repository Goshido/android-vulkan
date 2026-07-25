#include "windows/gizmo_prepass_common.hlsl"


#define SQRT_3              1.73205F


struct InputData
{
    uint32_t                _vertexID:      SV_VertexID;
    uint32_t                _instanceID:    SV_InstanceID;
};

static float32_t3 const     g_cube[ 8U ] =
{
    float32_t3 ( -1.0F, -1.0F, -1.0F ),
    float32_t3 ( -1.0F, -1.0F, 1.0F ),
    float32_t3 ( -1.0F, 1.0F, -1.0F ),
    float32_t3 ( -1.0F, 1.0F, 1.0F ),
    float32_t3 ( 1.0F, -1.0F, -1.0F ),
    float32_t3 ( 1.0F, -1.0F, 1.0F ),
    float32_t3 ( 1.0F, 1.0F, -1.0F ),
    float32_t3 ( 1.0F, 1.0F, 1.0F )
};

static float32_t3 const     g_expandDir[ 8U ] =
{
    float32_t3 ( -5.773503e-1F, -5.773503e-1F, -5.773503e-1F ),
    float32_t3 ( -5.773503e-1F, -5.773503e-1F, 5.773503e-1F ),
    float32_t3 ( -5.773503e-1F, 5.773503e-1F, -5.773503e-1F ),
    float32_t3 ( -5.773503e-1F, 5.773503e-1F, 5.773503e-1F ),
    float32_t3 ( 5.773503e-1F, -5.773503e-1F, -5.773503e-1F ),
    float32_t3 ( 5.773503e-1F, -5.773503e-1F, 5.773503e-1F ),
    float32_t3 ( 5.773503e-1F, 5.773503e-1F, -5.773503e-1F ),
    float32_t3 ( 5.773503e-1F, 5.773503e-1F, 5.773503e-1F )
};

//----------------------------------------------------------------------------------------------------------------------

// All SDF shapes use two shells:
// - cube 2x2x2 with center in origin
// - x axis aligned box 1x2x2 with center in (0.5, 0, 0)
// The g_cube describes the cube geometry. In case of SHAPE_LINE_SEGMENT_X and SHAPE_CONE the cube geometry must be
// transformed into box geometry. Shape type are assigned such a way that SHAPE_LINE_SEGMENT_X and SHAPE_CONE have odd
// value but SHAPE_SPHERE, SHAPE_BOX and CAPPED_TORUS have even value.
float32_t4 ComputeVertex ( in InputData inputData )
{
    SDFShape const sdfShape = vk::RawBufferLoad<SDFShape> (
        g_pushConstants._shapeStream + inputData._instanceID * sizeof ( SDFShape ),
        4U
    );

    float32_t3 const v = g_cube[ inputData._vertexID ];
    float32_t2 const cases[ 2U ] = { float32_t2 ( 1.0F, 0.0F ), float32_t2 ( 0.5F, 0.5F ) };
    float32_t2 const params = cases[ sdfShape._type & 0x00000001U ];
    return float32_t4 ( mad ( v.x, params.x, params.y ), v.yz, 1.0F );
}

//----------------------------------------------------------------------------------------------------------------------

Attributes VS ( in InputData inputData )
{
    // See <repo>/docs/gizmo-rendering.md#shell-extend

    SDFVertex const sdfVertex = vk::RawBufferLoad<SDFVertex> (
        g_pushConstants._vertexStream + inputData._instanceID * sizeof ( SDFVertex ),
        8U
    );

    float32_t3 const v = mul ( sdfVertex._toWorld, ComputeVertex ( inputData ) ).xyz;
    float32_t const omega = SQRT_3 * dot ( v - g_pushConstants._cameraPositionWorld, g_pushConstants._viWorld );
    float32_t3x3 const toSDFOrientation = ToMatrix ( sdfVertex._sdfOrientation );

    // SDF transformation contains shell expand transformation implicitly. Upper 3x3 matrix is orthogonal basis of
    // unit vectors. Taking inverse matrix from it will get shell expand basis. It's possible to use transpose trick to
    // compute inverse matrix in this case. Another thick is to swap multiplication order to achieve transpose effect.
    float32_t3 const expandDir = mul ( toSDFOrientation, g_expandDir[ inputData._vertexID ] );
    float32_t3 const shell = mad ( expandDir, omega, v );

    Attributes result;
    result._vertexH = mul ( g_pushConstants._toCVV, float32_t4 ( shell, 1.0F ) );
    result._canvas = mul ( shell, toSDFOrientation ) + sdfVertex._sdfOffset;
    result._instanceID = inputData._instanceID;

    return result;
}
