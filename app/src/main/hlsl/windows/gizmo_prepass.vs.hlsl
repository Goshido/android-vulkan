#include "windows/gizmo_prepass_common.hlsl"


#define SQRT_3              1.73205F


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
float32_t4 ComputeVertex ( in uint32_t vertexID )
{
    float32_t3 const v = g_cube[ vertexID ];
    float32_t2 const cases[ 2U ] = { float32_t2 ( 1.0F, 0.0F ), float32_t2 ( 0.5F, 0.5F ) };
    float32_t2 const params = cases[ g_pushConstants._shapeInfo._type & 0x00000001U ];
    return float32_t4 ( mad ( v.x, params.x, params.y ), v.yz, 1.0F );
}

//----------------------------------------------------------------------------------------------------------------------

Attributes VS ( in uint32_t vertexID: SV_VertexID )
{
    // See <repo>/docs/gizmo-rendering.md#shell-extend
    float32_t3 const v = mul ( g_pushConstants._toWorld, ComputeVertex ( vertexID ) ).xyz;
    float32_t const omega = SQRT_3 * dot ( v - g_pushConstants._cameraPositionWorld, g_pushConstants._viWorld );
    float32_t3x3 const toSDFOrientation = ToMatrix ( g_pushConstants._sdfOrientation );

    // SDF transformation contains shell expand transformation implicitly. Upper 3x3 matrix is orthogonal basis of
    // unit vectors. Taking inverse matrix from it will get shell expand basis. It's possible to use transpose trick to
    // compute inverse matrix in this case. Another thick is to swap multiplication order to achieve transpose effect.
    float32_t3 const expandDir = mul ( toSDFOrientation, g_expandDir[ vertexID ] );
    float32_t3 const shell = mad ( expandDir, omega, v );

    Attributes result;
    result._vertexH = mul ( g_pushConstants._toCVV, float32_t4 ( shell, 1.0F ) );
    result._canvas = mul ( shell, toSDFOrientation ) + g_pushConstants._sdfOffset;

    return result;
}
