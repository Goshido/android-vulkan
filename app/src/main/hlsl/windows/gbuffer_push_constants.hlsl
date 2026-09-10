#ifndef GBUFFER_PUSH_CONSTANTS_HLSL
#define GBUFFER_PUSH_CONSTANTS_HLSL


#include "windows/gbuffer_streams.hlsl"
#include "windows/id_stream.hlsl"
#include "windows/index_stream.hlsl"


struct PushConstants
{
    Transforms      _transformStream;
    Shadings        _shadingStream;
    Frames          _frameStream;
    Positions       _positionStream;
    Rests           _restStream;
    uint64_t        _indexStream;
    eIndex          _indexType;
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
    eIndex          _indexType;
    uint32_t        _idImage;
};


#endif // GBUFFER_PUSH_CONSTANTS_HLSL
