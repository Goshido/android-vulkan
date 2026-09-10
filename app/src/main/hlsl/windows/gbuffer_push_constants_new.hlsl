#ifndef GBUFFER_PUSH_CONSTANTS_HLSL
#define GBUFFER_PUSH_CONSTANTS_HLSL


#include "windows/gbuffer_streams_new.hlsl"
#include "windows/id_stream_new.hlsl"


struct PushConstants
{
    Transforms      _transformStream;
    Shadings        _shadingStream;
    Frames          _frameStream;
    Positions       _positionStream;
    Rests           _restStream;
    uint64_t        _indexStream;
    uint32_t        _indexType;
};

struct PushConstantsWithID
{
    Transforms      _transformStream;
    Shadings        _shadingStream;
    Frames          _frameStream;
    IDs             _idStream;
    Positions       _positionStream;
    Rests           _restStream;
    uint64_t        _indexStream;
    uint32_t        _indexType;
    uint32_t        _idImage;
};


#endif // GBUFFER_PUSH_CONSTANTS_HLSL
