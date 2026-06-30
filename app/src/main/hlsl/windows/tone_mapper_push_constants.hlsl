#ifndef TONE_MAPPER_PUSH_CONSTANTS_HLSL
#define TONE_MAPPER_PUSH_CONSTANTS_HLSL


struct PushConstants
{
    uint32_t        _exposure;
    uint32_t        _hdrImage;
    float32_t2x2    _transform;
    uint32_t        _pad[ 26U ];
};

[[vk::push_constant]]
PushConstants       g_pushConstants;


#endif // TONE_MAPPER_PUSH_CONSTANTS_HLSL
