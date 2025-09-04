#ifndef UI_PUSH_CONSTANTS_HLSL
#define UI_PUSH_CONSTANTS_HLSL


struct PushConstants
{
    uint64_t        _bda;
    float32_t2x2    _rotateScale;
    float32_t2      _offset;
    uint32_t        _textLUT;
};

[[vk::push_constant]]
PushConstants       g_uiInfo;


#endif // UI_PUSH_CONSTANTS_HLSL
