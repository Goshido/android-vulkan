#include "color_packing.hlsl"
#include "color_space.hlsl"
#include "platform/windows/pbr/gizmo_binds.inc"
#include "platform/windows/pbr/gizmo_shapes.inc"
#include "platform/windows/pbr/resource_heap.inc"
#include "windows/gizmo_pack.hlsl"
#include "windows/gizmo_prepass_common.hlsl"
#include "windows/gizmo_tile.hlsl"


// 0.5F * ALPHA_8_BIT_UNORM
#define MIN_ALPHA_THRESHOLD     1.96078e-3F

// 2.0F * ALPHA_8_BIT_UNORM
#define INSIDE_TEST_FACTOR      7.84314e-3F

// 1.0F - 0.5F * ALPHA_8_BIT_UNORM
#define MAX_ALPHA_THRESHOLD     9.98039e-1F

#define BAD_SDF                 42.0e+20F
#define MAX_STEPS               30U


struct Pixel
{
    float32_t4      _color;
    float32_t       _depth;
};

struct OutputData
{
    float32_t       _depth:     SV_Depth;

    [[vk::location ( OUT_COLOR )]]
    float32_t4      _color:     SV_Target0;
};

//----------------------------------------------------------------------------------------------------------------------

// Idea is taken from https://iquilezles.org/articles/distfunctions/
float32_t SDFLineSegment ( in float32_t3 p, in float32_t h, in float32_t r )
{
    return r + length ( float32_t3 ( p.x - clamp ( p.x, 0.0F, h ), p.yz ) );
}

// Idea is taken from https://iquilezles.org/articles/distfunctions/
// [2026/02/23] In original paper the cone tip is touching (0, 0, 0). The cone base is below the origin.
float32_t SDFCone ( in float32_t3 p, in float32_t2 q, in float32_t qDotFactor, in float32_t r )
{
    float32_t2 const nQ = -q;

    float32_t2 const w = float32_t2 ( length ( p.yz ), p.x );
    float32_t2 const a = mad ( q, -saturate ( dot ( w, q ) * qDotFactor ), w );
    float32_t2 const b = mad ( q, float32_t2 ( saturate ( w.x / nQ.x ), -1.0F ), w );

    float32_t const d = min ( dot ( a, a ), dot ( b, b ) );
    float32_t2 const v = sign ( q.y ) * float32_t2 ( w.y + nQ.y, dot ( w, float32_t2 ( q.y, nQ.x ) ) );

    return mad ( sqrt ( d ), sign ( max ( v.x, v.y ) ), r );
}

// Idea is taken from https://iquilezles.org/articles/distfunctions/
float32_t SDFSphere ( in float32_t3 p, in float32_t r )
{
    return r + length ( p );
}

// Idea is taken from https://iquilezles.org/articles/distfunctions/
float32_t SDFBox ( in float32_t3 p, in float32_t3 size, in float32_t r )
{
    float32_t3 const q = abs ( p ) + size;
    return r + length ( max ( 0.0F, q ) ) + min ( 0.0F, max ( q.x, max ( q.y, q.z ) ) );
}

// Idea is taken from https://iquilezles.org/articles/distfunctions/
// [2026/03/16] 'sinCosAngle' accepts angles in range from 0 to pi. 0 - torus is single point. pi - complete torus.
// Torus is aligned to XY plane.
// Torus center is located in origin.
// The grow point is located on Y axis, positive direction: (0, radius, 0)
float32_t SDFCappedTorus ( in float32_t3 p, in float32_t2 sinCosAngle, in float32_t radius, in float32_t thickness )
{
    p.x = abs ( p.x );
    float32_t2 const a = (float32_t2)radius * float32_t2 ( radius, -2.0F );
    float32_t2 const b = sinCosAngle * p.yx;
    float32_t const c = dot ( p, p );
    float const d = b.x < b.y ? dot ( p.xy, sinCosAngle ) : length ( p.xy );
    return thickness + sqrt ( c + mad ( d, a.y, a.x ) );
}

float32_t SDF ( in float32_t3 p, in uint32_t shapeType, in float32_t4 sdfParams )
{
    switch ( shapeType )
    {
        case SHAPE_LINE_SEGMENT: return SDFLineSegment ( p, sdfParams.x, sdfParams.y );
        case SHAPE_CONE: return SDFCone ( p, sdfParams.xy, sdfParams.z, sdfParams.w );
        case SHAPE_SPHERE: return SDFSphere ( p, sdfParams.x );
        case SHAPE_BOX: return SDFBox ( p, sdfParams.xyz, sdfParams.w );
        case SHAPE_CAPPED_TORUS: return SDFCappedTorus ( p, sdfParams.xy, sdfParams.z, sdfParams.w );

        default:
            // IMPOSSIBLE
        break;
    }

    return BAD_SDF;
}

float32_t LinearStep ( in float32_t step, in float32_t x )
{
    float32_t const s = -step;
    return saturate ( ( x + s ) / s );
}

Pixel ComputeColor ( in Attributes inputData, in SDFShape sdfShape )
{
    SDFPixel const sdfPixel = vk::RawBufferLoad<SDFPixel> (
        g_pushConstants._pixelStream + inputData._instanceID * sizeof ( SDFPixel ),
        8U
    );

    float32_t3 const ray = normalize ( inputData._canvas - sdfPixel._cameraLocationSDF );

    // precomputing part of dot product due to dot product property: dot(S * a, b) = S * dot(a, b)
    float32_t const pixelScale = dot ( ray, sdfPixel._viSDF );

    // x - current distance from SDF
    // y - maximum allowed distance (camera far plane)
    float32_t2 alpha = float32_t2 ( 0.0F, g_pushConstants._maxRayDistance );

    // x - adjustable minimal distance to consider ray vs SDF hit
    // y - ray distance has traveled
    float32_t2 beta = (float32_t2)0.0F;

    // x - closest distance detected
    // y - closest ray length corresponding closest distance detected
    float32_t2 closest = (float32_t2)g_pushConstants._maxRayDistance;

    float32_t const dynamicThresholdFactor = pixelScale * ALPHA_8_BIT_UNORM;

    for ( uint32_t steps = 0U; steps < MAX_STEPS; ++steps )
    {
        alpha.x = SDF ( mad ( ray, beta.y, sdfPixel._cameraLocationSDF ), sdfShape._type, sdfPixel._sdfParams );
        closest = lerp ( closest, float32_t2 ( alpha.x, beta.y ), closest.x > alpha.x );
        beta.y += alpha.x;
        beta.x = beta.y * dynamicThresholdFactor;

        if ( any ( alpha < beta ) )
        {
            break;
        }
    }

    Pixel result;
    result._depth = closest.y * g_pushConstants._invMaxRayDistance;

    float32_t const insideProbe = SDF (
        mad ( ray, mad ( pixelScale * INSIDE_TEST_FACTOR, beta.y, closest.y ), sdfPixel._cameraLocationSDF ),
        sdfShape._type,
        sdfPixel._sdfParams
    );

    float32_t4 const color = g_gizmoPalette[ sdfShape._palette ];

    float32_t2 const cases = float32_t2 (
        // inside SDF shape
        color.w,

        // AA loop or outside SDF shape
        color.w * LinearStep ( closest.y * pixelScale, closest.x )
    );

    result._color = float32_t4 ( color.xyz, cases[ (uint32_t)( insideProbe >= 0.0F ) ] );
    return result;
}

void AddSample ( in uint32_t2 pix, in float32_t alpha, in float32_t depth, in uint32_t palette )
{
    uint32_t2 const tile = pix >> uint32_t2 ( TILE_WIDTH_SHIFT, TILE_HEIGHT_SHIFT );
    uint32_t2 const item = pix & uint32_t2 ( TILE_LOCAL_X_MASK, TILE_LOCAL_Y_MASK );
    uint32_t const idx = tile.y * g_pushConstants._tileCountWidth + tile.x;

    RWStructuredBuffer<GizmoCounters> tileCounters = ResourceDescriptorHeap[ g_pushConstants._tileCounters ];
    uint32_t layer;
    InterlockedAdd ( tileCounters[ idx ]._counters[ item.y ], MakeSampleCounterIncrement ( item.x ), layer );
    layer = UnpackSampleCounter ( layer, item.x );

    if ( layer >= TILE_LAYERS )
        return;

    // See <repo>/docs/gizmo-rendering.md#samples
    uint32_t2 const beta = (uint32_t2)layer & uint32_t2 ( 0xFFFFFFFEU, 0x00000001U );
    uint32_t2 const omega = pix & 0x00000001U;
    uint32_t2 const phi = item >> 1U;

    uint32_t4 zeta = uint32_t4 ( idx,
        beta.x,
        uint32_t2 ( phi.x, omega.x ) + ( uint32_t2 ( phi.y, omega.y ) << uint32_t2 ( 2U, 1U ) )
    );

    zeta <<= uint32_t4 ( 9U, 6U, 3U, 1U );
    uint64_t const offset = (uint64_t)( ( zeta.x + zeta.y + zeta.z + zeta.w + beta.y ) << 2U );
    TileSamples ( (uint64_t)g_pushConstants._tileSamples + offset ).Get () = PackSample ( palette, alpha, depth );
}

//----------------------------------------------------------------------------------------------------------------------

OutputData PS ( in Attributes inputData )
{
    SDFShape const sdfShape = vk::RawBufferLoad<SDFShape> (
        g_pushConstants._shapeStream + inputData._instanceID * sizeof ( SDFShape ),
        4U
    );

    Pixel pixel = ComputeColor ( inputData, sdfShape );
    bool2 const check = pixel._color.ww > float32_t2 ( MAX_ALPHA_THRESHOLD, MIN_ALPHA_THRESHOLD );

    if ( check.x )
    {
        OutputData result;

        result._color = float32_t4 (
            (float32_t3)LinearToSRGB ( pow ( (float16_t3)pixel._color.xyz, (float16_t)g_pushConstants._brightness ) ),
            1.0F
        );

        result._depth = pixel._depth;
        return result;
    }

    if ( check.y )
        AddSample ( (uint32_t2)inputData._vertexH.xy, pixel._color.w, pixel._depth, sdfShape._palette );

    discard;
    OutputData result;
    result._color = (float32_t4)0.0F;
    result._depth = 1.0F;
    return result;
}
