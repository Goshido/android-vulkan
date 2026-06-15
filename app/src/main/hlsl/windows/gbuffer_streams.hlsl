#ifndef GBUFFER_STREAMS_HLSL
#define GBUFFER_STREAMS_HLSL


#include "color_packing.hlsl"
#include "tbn32.hlsl"
#include "tbn64.hlsl"


#define VK_INDEX_TYPE_UINT16        0U
#define VK_INDEX_TYPE_UINT32        1U
#define VK_INDEX_TYPE_NONE_KHR      1000165000U


struct Rest
{
    float16_t2      _uv;
    TBN32           _tbn;
};

struct Frame
{
    float32_t4x4    _viewProj;
};

struct Transform
{
    float32_t3x4    _model;
    TBN64           _normal;
};

struct Shading
{
    uint32_t        _albedo;
    uint32_t        _emission;
    uint32_t        _mask;
    uint32_t        _param;
    uint32_t        _normal;
    ColorData       _colors;
};


#endif // GBUFFER_STREAMS_HLSL
