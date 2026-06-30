#ifndef UI_PUSH_CONSTANTS_HLSL
#define UI_PUSH_CONSTANTS_HLSL


struct PushConstants
{
    uint64_t        _bdaStream0;
    uint64_t        _bdaStream1;
    float32_t2x2    _rotateScale;
    float32_t2      _offset;
    uint32_t        _textLUT;
    uint32_t        _pad[ 21U ];
};

[[vk::push_constant]]
PushConstants       g_pushConstants;


#endif // UI_PUSH_CONSTANTS_HLSL
