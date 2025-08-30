#ifndef UI_PUSH_CONSTANTS_HLSL
#define UI_PUSH_CONSTANTS_HLSL


struct PushConstants
{
    // FUCK - use single vertex data buffer
    uint64_t        _positionBDA;
    uint64_t        _restBDA;

    float32_t2x2    _rotateScale;
    float32_t2      _offset;

    // FUCK make better packing
    uint32_t        _textLUT;
};

[[vk::push_constant]]
PushConstants       g_uiInfo;


#endif // UI_PUSH_CONSTANTS_HLSL
