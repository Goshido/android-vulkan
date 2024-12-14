#include "lightup_common.ps.hlsl"
#include "pbr/point_light.inc"


// The implementation is based on ideas from following resources:
// see https://www.trentreed.net/blog/physically-based-shading-and-image-based-lighting/
// see https://learnopengl.com/PBR/Theory
// see https://learnopengl.com/PBR/Lighting

#define EPSILON             1.0e-3H

#define PI                  3.141592F
#define INVERSE_PI          0.31830H
#define SQRT_PI             1.77245H

// It was set by empirical test.
#define SHADOW_MAP_BIAS     9.9e-1F

[[vk::binding ( BIND_SHADOWMAP_TEXTURE, SET_LIGHT_DATA )]]
TextureCube<float32_t>      g_shadowmap:    register ( t0 );

[[vk::binding ( BIND_SHADOWMAP_SAMPLER, SET_LIGHT_DATA )]]
SamplerComparisonState      g_sampler:      register ( s0 );

[[vk::binding ( BIND_LIGHT_DATA, SET_LIGHT_DATA )]]
cbuffer LightData:                          register ( b2 )
{
    float32_t4x4            _lightProjection;
    float32_t4x4            _viewToLight;

    float32_t3              _lightLocationView;

    // Must be equal: 1.0F / units-per-meter
    float32_t               _sceneScaleFactor;

    float32_t3              _hue;
    float32_t               _intensity;
};

//----------------------------------------------------------------------------------------------------------------------

float16_t3 GetDiffuseK ( in float16_t3 fresnelFactor, in float16_t metallic )
{
    return ( 1.0H - fresnelFactor ) * ( 1.0H - metallic );
}

float16_t GGXSchlick ( in float16_t dotFactor, in float16_t k )
{
    return dotFactor / mad ( dotFactor, 1.0H - k, k );
}

float16_t GetAttenuation ( in float32_t3 fragToLight )
{
    float16_t const beta = (float16_t)max ( 1.0F, _sceneScaleFactor * length ( fragToLight ) );
    return (float16_t)_intensity / ( beta * beta );
}

float16_t GetDistribution ( in float16_t nH, in float16_t aa )
{
    // float16_t precision is not enough here. We need more precision.
    float32_t const nHF = (float32_t)nH;
    float32_t const aaF = (float32_t)aa;

    float32_t const alpha = mad ( nHF * nHF, aaF - 1.0F, 1.0F );
    return (float16_t)( aaF / ( PI * alpha * alpha ) );
}

float16_t GetGeometryAttenuation ( in float16_t nL, in float16_t nV, in float16_t aa )
{
    float16_t const k = aa * SQRT_PI;
    return GGXSchlick ( nL, k ) * GGXSchlick ( nV, k );
}

float16_t GetShadowFactor ( in float32_t3 locationView )
{
    float32_t3 const dir = mul ( _viewToLight, float32_t4 ( locationView, 1.0F ) ).xyz * SHADOW_MAP_BIAS;
    float32_t3 const faceSelector = abs ( dir );
    float32_t const z = max ( faceSelector.x, max ( faceSelector.y, faceSelector.z ) );
    float32_t4 const cvv = mul ( _lightProjection, float32_t4 ( 0.0F, 0.0F, z, 1.0F ) );

    return (float16_t)g_shadowmap.SampleCmpLevelZero ( g_sampler, dir, cvv.z / cvv.w );
}

float32_t3 RestoreLocationView ( in float32_t2 locationImage )
{
    float32_t2 const alpha = mad ( locationImage, _invResolutionFactor, (float32_t2)( -1.0F ) );
    float32_t4 const v = mul ( _cvvToView, float32_t4 ( alpha, GetDepth (), 1.0F ) );
    return v.xyz * ( 1.0F / v.w );
}

//----------------------------------------------------------------------------------------------------------------------

OutputData PS ( in linear float32_t4 pixel: SV_Position )
{
    float32_t3 const locationView = RestoreLocationView ( pixel.xy );
    float32_t3 const fragToLight = _lightLocationView - locationView;

    float16_t const attenuation = GetAttenuation ( fragToLight );
    float16_t3 const toViewerView = -(float16_t3)normalize ( locationView );
    float16_t3 const toLightView = (float16_t3)normalize ( fragToLight );
    float16_t3 const halfVectorView = normalize ( toLightView + toViewerView );
    float16_t3 const normalView = RestoreNormalView ();

    float16_t roughness;
    float16_t metallic;
    GetParams ( roughness, metallic );

    float16_t const a = roughness * roughness;
    float16_t const aa = a * a;
    float16_t3 const albedo = GetAlbedo ();

    float16_t const nL = saturate ( dot ( normalView, toLightView ) );
    float16_t const nH = saturate ( dot ( normalView, halfVectorView ) );
    float16_t const nV = saturate ( dot ( normalView, toViewerView ) );
    float16_t const vH = saturate ( dot ( toViewerView, halfVectorView ) );

    float16_t const d = GetDistribution ( nH, aa );
    float16_t3 const f = GetFresnel ( vH, metallic, albedo );
    float16_t const ga = GetGeometryAttenuation ( nL, nV, aa );
    float16_t const shadow = GetShadowFactor ( locationView );
    float16_t3 const kD = GetDiffuseK ( f, metallic );

    float16_t const s = d * ga / mad ( 4.0H, nL * nV, EPSILON );
    float16_t3 const color = shadow * nL * attenuation * (float16_t3)_hue * mad ( kD * albedo, INVERSE_PI, f * s );

    OutputData result;
    result._color = float32_t4 ( (float32_t3)color, 1.0F );

    return result;
}
