#include "color_space.hlsl"
#include "rotating_mesh/bindings.inc"


// Main reference is Source Engine
// https://developer.valvesoftware.com/wiki/Phong_materials
#define MAXIMUM_SHININESS               150.0F
#define SHININESS_CONVERT               ( 1.0F / MAXIMUM_SHININESS )

#define TARGET_SHININESS                128.0F
#define TARGET_LIGHT_SOURCE_ORIGIN      float32_t3 ( 0.0F, 0.0F, 0.0F )

[[vk::binding ( BIND_SAMPLER, SET_FIXED )]]
SamplerState                g_sampler:              register ( s0 );

[[vk::binding ( BIND_DIFFUSE_TEXTURE, SET_MATERIAL )]]
Texture2D<float32_t4>       g_diffuseTexture:       register ( t0 );

[[vk::binding ( BIND_NORMAL_TEXTURE, SET_MATERIAL )]]
Texture2D<float32_t4>       g_normalTexture:        register ( t1 );

[[vk::binding ( BIND_LUT_TEXTURE, SET_MATERIAL )]]
Texture2D<float32_t4>       g_specLUTTexture:       register ( t2 );

struct InputData
{
    [[vk::location ( ATT_SLOT_FRAGMENT_VIEW )]]
    linear float32_t3       _fragmentView:          FRAGMENT;

    [[vk::location ( ATT_SLOT_UV )]]
    linear float32_t2       _uv:                    UV;

    [[vk::location ( ATT_SLOT_NORMAL_VIEW )]]
    linear float32_t3       _normalView:            NORMAL;

    [[vk::location ( ATT_SLOT_TANGENT_VIEW )]]
    linear float32_t3       _tangentView:           TANGENT;

    [[vk::location ( ATT_SLOT_BITANGENT_VIEW )]]
    linear float32_t3       _bitangentView:         BITANGENT;
};

//----------------------------------------------------------------------------------------------------------------------

float32_t3 GetFinalNormalView ( in float32_t3 tangentView,
    in float32_t3 bitangentView,
    in float32_t3 normalView,
    in float32_t2 uv
)
{
    float32_t3 const normalData = mad ( g_normalTexture.Sample ( g_sampler, uv ).xyz,
        (float32_t3)2.0F,
        (float32_t3)( -1.0F )
    );

    float32_t3x3 const tbnView = float32_t3x3 ( tangentView, bitangentView, normalView );
    return normalize ( mul ( normalData, tbnView ) );
}

float32_t GetSpecular ( in float32_t3 finalNormalView,
    in float32_t3 toViewerView,
    in float32_t3 toLightView,
    in float32_t shininess
)
{
    // See https://en.wikipedia.org/wiki/Blinn%E2%80%93Phong_reflection_model

    float32_t3 const halfVector = normalize ( toViewerView + toLightView );
    float32_t const angleFactor = max ( dot ( halfVector, finalNormalView ), 0.0F );
    float32_t2 const uv = float32_t2 ( angleFactor, shininess * SHININESS_CONVERT );

    return g_specLUTTexture.SampleLevel ( g_sampler, uv, 0.0F ).x;
}

float32_t4 PS ( in InputData inputData ): SV_Target0
{
    float32_t3 const diffuseData = g_diffuseTexture.Sample ( g_sampler, inputData._uv ).xyz;

    float32_t3 const normalView = GetFinalNormalView ( inputData._tangentView,
        inputData._bitangentView,
        inputData._normalView,
        inputData._uv
    );

    float32_t3 const toView = -normalize ( inputData._fragmentView );
    float32_t const lambertian = max ( 0.0F, dot ( toView, normalView ) );

    float32_t3 const toLight = normalize ( TARGET_LIGHT_SOURCE_ORIGIN - inputData._fragmentView );
    float32_t3 const specularFactor = (float32_t3)GetSpecular ( normalView, toView, toLight, TARGET_SHININESS );

    float32_t3 const color = mad ( diffuseData, (float32_t3)lambertian, specularFactor );
    return float32_t4 ( (float32_t3)LinearToSRGB ( (float16_t3)color ), 1.0F );
}
