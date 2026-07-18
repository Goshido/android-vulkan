#ifndef TONE_MAPPER_PUSH_CONSTANTS_HLSL
#define TONE_MAPPER_PUSH_CONSTANTS_HLSL


struct PushConstants
{
    uint32_t        _exposure;
    uint32_t        _hdrImage;
    uint32_t        _outlineBlurX;
    float32_t2      _resolution;
    float32_t2      _halfPixelMove;
    float32_t2x2    _transform;
};

[[vk::push_constant]]
PushConstants       g_pushConstants;


#endif // TONE_MAPPER_PUSH_CONSTANTS_HLSL
