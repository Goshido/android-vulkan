#include "color_space.hlsl"
#include "platform/windows/pbr/gizmo_compose.inc"
#include "platform/windows/pbr/resource_heap.inc"
#include "windows/gizmo_pack.hlsl"
#include "windows/gizmo_tile.hlsl"


typedef vk::BufferPointer<GizmoCounters>    TileCounters;
typedef vk::BufferPointer<uint32_t>         TileSamples;

struct PushConstants
{
    TileCounters                            _tileCounters;
    TileSamples                             _tileSamples;
    uint32_t2                               _resolution;
    float32_t                               _brightness;
    uint32_t                                _tileCountWidth;
    uint32_t                                _color;
    uint32_t                                _depth;
};

[[vk::push_constant]]
PushConstants                               g_pushConstants;

// [2026/08/16] DXC has no syntax explicit image format and 'ResourceDescriptorHeap'.
// So the workaround is used.
// [2026/08/16] Never mix ResourceDescriptorHeap|SamplerDescriptorHeap and explicit descriptor indexing in same shader.
// The DXC will produce unexpected 'Binding' locations in SPIR-V. Be consistent.

[[vk::binding ( BIND_RESOURCES, SET_RESOURCE_HEAP )]]
[[vk::image_format ( "rgba8" )]]
RWTexture2D<float32_t4>                     g_images[]:     register ( u0 );

[[vk::binding ( BIND_RESOURCES, SET_RESOURCE_HEAP )]]
Texture2D<float32_t>                        g_depth[]:      register ( t0 );

struct History
{
    uint32_t                                _count;
    uint32_t                                _samples[ TILE_LAYERS ];
};

static History                              g_history;

//----------------------------------------------------------------------------------------------------------------------

bool DepthTestHistory ( in uint32_t2 pix, in uint32_t sampleCount )
{
    Texture2D<float32_t> depthImage = g_depth[ g_pushConstants._depth ];
    uint32_t const depth = PackDepth ( depthImage[ pix ] );
    g_history._count = 0U;

    // See <repo>/docs/gizmo-rendering.md#samples
    uint32_t4 const alpha = pix.xxyy & uint32_t4 ( 0x00000007U, 0x00000001U, 0x00000007U, 0x00000001U );
    uint32_t4 const beta = uint32_t4 ( alpha.xz, pix ) >> uint32_t4 ( 1U, 1U, 3U, 3U );

    uint32_t3 zeta = uint32_t3 ( beta.w * g_pushConstants._tileCountWidth,
        uint32_t2 ( beta.y, alpha.w ) << uint32_t2 ( 2U, 1U )
    );

    zeta += uint32_t3 ( beta.zx, alpha.y );
    zeta <<= uint32_t3 ( 11U, 5U, 3U );
    uint32_t offset = zeta.x + zeta.y + zeta.z;
    uint32_t const cases[] = { 4U, 508U };

    for ( uint32_t i = 0U; i < sampleCount; ++i )
    {
        // That's the reason why bit fields approach is not used.
        uint32_t const s = TileSamples ( (uint64_t)g_pushConstants._tileSamples + (uint64_t)offset ).Get ();
        offset += cases[ i & 0x00000001U ];

        if ( s < depth )
        {
            g_history._samples[ g_history._count++ ] = s;
        }
    }

    return g_history._count > 0U;
}

void BubbleSort ()
{
    uint32_t const outerLimit = g_history._count - 1U;

    for ( uint32_t i = 0U; i < outerLimit; ++i )
    {
        uint32_t const innerLimit = outerLimit - i;

        for ( uint32_t prevIdx = 0U; prevIdx < innerLimit; ++prevIdx )
        {
            uint32_t const nextIdx = prevIdx + 1U;
            uint32_t const prev = g_history._samples[ prevIdx ];
            uint32_t const next = g_history._samples[ nextIdx ];

            // One more reason why bit fields approach is not used.
            if ( prev >= next )
                continue;

            g_history._samples[ prevIdx ] = next;
            g_history._samples[ nextIdx ] = prev;
        }
    }
}

void Compose ( in uint32_t2 pix )
{
    // Using premultiplied alpha technique to properly blend data as overlay.
    float16_t4 c = (float16_t4)0.0H;

    for ( uint32_t i = 0U; i < g_history._count; ++i )
    {
        float16_t4 const overlay = UnpackSamplePremultipliedAlpha ( g_history._samples[ i ] );
        c = mad ( c, 1.0H - overlay.w, overlay );
    }

    c.xyz = pow ( c.xyz, (float16_t)g_pushConstants._brightness );

    RWTexture2D<float32_t4> color = g_images[ g_pushConstants._color ];
    float16_t3 const dst = SRGBToLinear ( (float16_t3)color[ pix ].xyz );
    color[ pix ] = float32_t4 ( (float32_t3)LinearToSRGB ( mad ( dst, 1.0H - c.w, c.xyz ) ), 1.0F );
}

//----------------------------------------------------------------------------------------------------------------------

[numthreads ( THREADS_X, THREADS_Y, 1 )]
void CS ( in uint32_t3 tile: SV_GroupID, in uint32_t3 pix: SV_DispatchThreadID, in uint32_t3 item: SV_GroupThreadID )
{
    if ( any ( pix.xy >= g_pushConstants._resolution ) )
        return;

    uint32_t const tileIdx = tile.y * g_pushConstants._tileCountWidth + tile.x;
    uint64_t const address = (uint64_t)g_pushConstants._tileCounters + (uint64_t)( tileIdx * sizeof ( GizmoCounters ) );
    uint32_t const tileLineSampleCount = TileCounters ( address ).Get ()._counters[ item.y ];

    if ( tileLineSampleCount == 0U )
        return;

    uint32_t const c = UnpackSampleCounter ( tileLineSampleCount, item.x );
    uint32_t const cases[ 2U ] = { c, TILE_LAYERS };

    if ( !DepthTestHistory ( pix.xy, cases[ (uint32_t)( c > TILE_LAYERS ) ] ) )
        return;

    BubbleSort ();
    Compose ( pix.xy );
}
