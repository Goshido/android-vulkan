#ifndef GIZMO_PREPASS_COMMON_HLSL
#define GIZMO_PREPASS_COMMON_HLSL


#include "tbn64.hlsl"


#define ATT_SLOT_CANVAS         0
#define ATT_SLOT_INSTANCE_ID    1


struct SDFShape
{
    uint32_t                    _palette: 24;
    uint32_t                    _type: 8;
};

struct SDFVertex
{
    TBN64                       _sdfOrientation;
    float32_t3x4                _toWorld;
    float32_t3                  _sdfOffset;
    uint32_t                    _pad0;
};

struct SDFPixel
{
    float32_t4                  _sdfParams;
    float32_t3                  _cameraLocationSDF;
    float32_t3                  _viSDF;
};

struct PushConstants
{
    float32_t4x4                _toCVV;
    float32_t3                  _cameraLocationWorld;
    float32_t                   _maxRayDistance;
    float32_t3                  _viWorld;
    float32_t                   _invMaxRayDistance;
    uint32_t                    _tileCountWidth;
    uint32_t                    _tileCounters;
    uint64_t                    _tileSamples;
    uint64_t                    _vertexStream;
    uint64_t                    _pixelStream;
    uint64_t                    _shapeStream;
};

[[vk::push_constant]]
PushConstants                   g_pushConstants;

struct Attributes
{
    linear float32_t4           _vertexH:       SV_Position;

    [[vk::location ( ATT_SLOT_CANVAS )]]
    linear float32_t3           _canvas:        CANVAS;

    [[vk::location ( ATT_SLOT_INSTANCE_ID )]]
    nointerpolation uint32_t    _instanceID:    INSTANCE_ID;
};


#endif // GIZMO_PREPASS_COMMON_HLSL
