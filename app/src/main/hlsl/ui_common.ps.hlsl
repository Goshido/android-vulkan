#ifndef UI_COMMON_PS_HLSL
#define UI_COMMON_PS_HLSL


#include "pbr/ui_primitive_type.inc"
#include "pbr/ui_program.inc"


#define COLOR_ALPHA_GUESS_1         3.72545e-1H
#define COLOR_ALPHA_GUESS_2         7.5557e-1H

// P means Bezier control point
static float16_t2 const             COLOR_ALPHA_P_X = float16_t2 ( 7.48e-1H, 3.99e-1H );
static float16_t2 const             COLOR_ALPHA_P_Y = float16_t2 ( 8.04e-1H, 1.0H );

static float16_t2 const             LUMA_BOTTOM_P1 = float16_t2 ( 5.4e-1H, 2.7e-1H );
static float16_t2 const             LUMA_BOTTOM_P2 = float16_t2 ( 9.9e-1H, 6.18e-1H );

static float16_t2 const             LUMA_TOP_P1 = float16_t2 ( 3.0e-2H, 2.12e-1H );
static float16_t2 const             LUMA_TOP_P2 = float16_t2 ( 3.42e-1H, 7.18e-1H );

[[vk::binding ( BIND_ATLAS_SAMPLER, SET_COMMON )]]
SamplerState                        g_atlasSampler:     register ( s0 );

[[vk::binding ( BIND_ATLAS_TEXTURE, SET_COMMON )]]
Texture2DArray<float32_t>           g_atlasTexture:     register ( t0 );

[[vk::binding ( BIND_IMAGE_SAMPLER, SET_COMMON )]]
SamplerState                        g_imageSampler:     register ( s1 );

[[vk::binding ( BIND_IMAGE_TEXTURE, SET_IMAGE_TEXTURE )]]
Texture2D<float32_t4>               g_imageTexture:     register ( t1 );

struct InputData
{
    [[vk::location ( ATT_SLOT_IMAGE_UV )]]
    noperspective float32_t2        _imageUV:               IMAGE_UV;

    [[vk::location ( ATT_SLOT_ATLAS_UV )]]
    noperspective float32_t2        _atlasUV:               ATLAS_UV;

    [[vk::location ( ATT_SLOT_ATLAS_LAYER )]]
    nointerpolation float32_t       _atlasLayer:            ATLAS_LAYER;

    [[vk::location ( ATT_SLOT_UI_PRIMITIVE_TYPE )]]
    nointerpolation uint32_t        _uiPrimitiveType:       UI_PRIMITIVE_TYPE;

    [[vk::location ( ATT_SLOT_COLOR )]]
    nointerpolation float32_t4      _color:                 COLOR;
};

//----------------------------------------------------------------------------------------------------------------------

// FUCK - make a reference to documentation.

float16_t SolveNewton ( in float16_t p1, in float16_t p2, in float16_t p, in float16_t guess )
{
    float16_t2 const vv = 3.0H * float16_t2 ( p1, p2 );
    float16_t const a = vv.x - vv.y + 1.0H;
    float16_t const b = mad ( -2.0H, vv.x, vv.y );
    float16_t const xi = 3.0H * a;
    float16_t const zeta = 2.0H * b;
    p = -p;

    [unroll ( 3 )]
    for ( uint16_t i = 0U; i < 3U; ++i )
    {
        float16_t const gamma = guess * guess;
        float16_t const t0 = mad ( a, guess, b );
        float16_t const t1 = mad ( vv.x, guess, p );
        float16_t const t2 = mad ( zeta, guess, vv.x );
        guess = guess - mad ( gamma, t0, t1 ) / mad ( xi, gamma, t2 );
    }

    return guess;
}

float16_t CubicBezier ( in float16_t p1, in float16_t p2, in float16_t t )
{
    float16_t const t2 = t * t;
    float16_t const w0 = t * p1;
    float16_t const w1 = mad ( -2.0H, p1, p2 );
    float16_t const w2 = ( p1 - p2 + 3.33333e-1H ) * t2;
    float16_t const w3 = mad ( w1, t2, w0 );
    return 3.0H * mad ( w2, t, w3 );
}

float16_t ResolveTextAlpha ( float16_t colorAlpha, float16_t glyphLuma )
{
    float16_t const guesses[ 3U ] =
    {
        mad ( 1.6H, colorAlpha, -3.8e-1H ),
        mad ( 7.0e-1H, colorAlpha, 3.0e-1H ),
        5.8e-1H * colorAlpha
    };

    uint16_t const idx = ( (uint16_t)( colorAlpha < COLOR_ALPHA_GUESS_1 ) << 1U ) +
        (uint16_t)( colorAlpha >= COLOR_ALPHA_GUESS_2 );

    float16_t const t = SolveNewton ( COLOR_ALPHA_P_X.x, COLOR_ALPHA_P_X.y, colorAlpha, guesses[ idx ] );
    float16_t const lumaLerp = CubicBezier ( COLOR_ALPHA_P_Y.x, COLOR_ALPHA_P_Y.y, t );

    float16_t2 const p1 = lerp ( LUMA_BOTTOM_P1, LUMA_TOP_P1, lumaLerp );
    float16_t2 const p2 = lerp ( LUMA_BOTTOM_P2, LUMA_TOP_P2, lumaLerp );

    return CubicBezier ( p1.y, p2.y, SolveNewton ( p1.x, p2.x, glyphLuma, glyphLuma ) );
}

float16_t4 HandleImage ( in float16_t4 color, in InputData inputData )
{
    return color * (float16_t4)g_imageTexture.SampleLevel ( g_imageSampler, inputData._imageUV, 0.0F );
}

float16_t4 HandleText ( in float16_t4 color, in InputData inputData )
{
    float32_t3 const atlasCoordinate = float32_t3 ( inputData._atlasUV, inputData._atlasLayer );
    float16_t const glyphLuma = (float16_t)g_atlasTexture.SampleLevel ( g_atlasSampler, atlasCoordinate, 0.0F ).x;
    return float16_t4 ( color.xyz, ResolveTextAlpha ( color.a, glyphLuma ) );
}

float16_t4 Compute ( in InputData inputData )
{
    float16_t4 const color = (float16_t4)inputData._color;

    switch ( inputData._uiPrimitiveType )
    {
        case PBR_UI_PRIMITIVE_TYPE_TEXT:
        return HandleText ( color, inputData );

        case PBR_UI_PRIMITIVE_TYPE_IMAGE:
        return HandleImage ( color, inputData );

        case PBR_UI_PRIMITIVE_TYPE_GEOMETRY:
        default:
        return color;
    }
}


#endif // UI_COMMON_PS_HLSL
