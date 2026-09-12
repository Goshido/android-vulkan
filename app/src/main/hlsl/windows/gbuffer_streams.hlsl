#ifndef GBUFFER_STREAMS_HLSL
#define GBUFFER_STREAMS_HLSL


#include "color_packing.hlsl"
#include "tbn32.hlsl"
#include "windows/frame_stream.hlsl"
#include "windows/position_stream.hlsl"
#include "windows/transform_stream.hlsl"


struct Rest
{
    float16_t2      _uv;
    TBN32           _tbn;
};

typedef vk::BufferPointer<Rest, 4U>             Rests;

struct Shading
{
    uint32_t        _albedo;
    uint32_t        _emission;
    uint32_t        _mask;
    uint32_t        _param;
    uint32_t        _normal;
    ColorData       _colors;
};

typedef vk::BufferPointer<Shading, 4U>          Shadings;


#endif // GBUFFER_STREAMS_HLSL
