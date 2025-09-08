#ifndef TONE_MAPPER_PUSH_CONSTANTS_HLSL
#define TONE_MAPPER_PUSH_CONSTANTS_HLSL


struct PushConstants
{
    uint32_t        _exposure;
    uint32_t        _hdrImage;
};

[[vk::push_constant]]
PushConstants       g_toneMapperInfo;


#endif // TONE_MAPPER_PUSH_CONSTANTS_HLSL
