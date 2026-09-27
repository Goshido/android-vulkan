#ifndef GIZMO_PREPASS_COMMON_HLSL
#define GIZMO_PREPASS_COMMON_HLSL


#include "quat.hlsl"


#define ATT_SLOT_CANVAS         0
#define ATT_SLOT_INSTANCE_ID    1


struct SDFShape
{
    uint32_t                                _palette: 24;
    uint32_t                                _type: 8;
};

typedef vk::BufferPointer<SDFShape>         SDFShapes;

struct SDFVertex
{
    QuatF                                   _sdfOrientation;
    float32_t3x4                            _toWorld;
    float32_t3                              _sdfOffset;
};

typedef vk::BufferPointer<SDFVertex, 4U>    SDFVertices;

struct SDFPixel
{
    float32_t4                              _sdfParams;
    float32_t3                              _cameraLocationSDF;
    float32_t3                              _viSDF;
};

typedef vk::BufferPointer<SDFPixel, 8U>     SDFPixels;

typedef vk::BufferPointer<uint32_t>         TileSamples;

struct PushConstants
{
    float32_t4x4                            _toCVV;
    float32_t3                              _cameraLocationWorld;
    float32_t                               _maxRayDistance;
    float32_t3                              _viWorld;
    float32_t                               _invMaxRayDistance;
    uint32_t                                _tileCountWidth;
    uint32_t                                _tileCounters;
    TileSamples                             _tileSamples;
    SDFVertices                             _vertexStream;
    SDFPixels                               _pixelStream;
    SDFShapes                               _shapeStream;
    float32_t                               _brightness;
};

[[vk::push_constant]]
PushConstants                               g_pushConstants;

struct Attributes
{
    linear float32_t4                       _vertexH:       SV_Position;

    [[vk::location ( ATT_SLOT_CANVAS )]]
    linear float32_t3                       _canvas:        CANVAS;

    [[vk::location ( ATT_SLOT_INSTANCE_ID )]]
    nointerpolation uint32_t                _instanceID:    INSTANCE_ID;
};


#endif // GIZMO_PREPASS_COMMON_HLSL
