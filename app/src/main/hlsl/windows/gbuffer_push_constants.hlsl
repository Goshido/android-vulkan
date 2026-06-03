#ifndef GBUFFER_PUSH_CONSTANTS_HLSL
#define GBUFFER_PUSH_CONSTANTS_HLSL


struct PushConstants
{
    uint64_t        _transformStream;
    uint64_t        _shadingStream;
    uint64_t        _frameStream;
    uint64_t        _positionStream;
    uint64_t        _restStream;
    uint64_t        _indexStream;
    uint32_t        _indexType;
};

[[vk::push_constant]]
PushConstants       g_pushConstants;


#endif // GBUFFER_PUSH_CONSTANTS_HLSL
