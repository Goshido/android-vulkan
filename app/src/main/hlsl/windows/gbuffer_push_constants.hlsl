#ifndef GBUFFER_PUSH_CONSTANTS_HLSL
#define GBUFFER_PUSH_CONSTANTS_HLSL


#include "platform/windows/pbr/push_constant_range.inc"


struct PushConstants
{
    uint64_t        _transformStream;
    uint64_t        _shadingStream;
    uint64_t        _frameStream;
    uint64_t        _positionStream;
    uint64_t        _restStream;
    uint64_t        _indexStream;
    uint32_t        _indexType;
    uint32_t        _pad[ 19U ];
};

struct PushConstantsWithID
{
    uint64_t        _transformStream;
    uint64_t        _shadingStream;
    uint64_t        _frameStream;
    uint64_t        _idStream;
    uint64_t        _positionStream;
    uint64_t        _restStream;
    uint64_t        _indexStream;
    uint32_t        _indexType;
    uint32_t        _idImage;
    uint32_t        _pad[ 16U ];
};


#endif // GBUFFER_PUSH_CONSTANTS_HLSL
