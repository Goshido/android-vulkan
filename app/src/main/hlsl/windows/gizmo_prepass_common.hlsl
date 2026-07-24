#ifndef GIZMO_PREPASS_COMMON_HLSL
#define GIZMO_PREPASS_COMMON_HLSL


#include "windows/gbuffer_attribute_slots.hlsl"
#include "tbn64.hlsl"


#define ATT_SLOT_CANVAS     0


struct ShapeInfo
{
    uint32_t                _palette: 24;
    uint32_t                _type: 8;
};

struct PushConstants
{
    float32_t4x4            _toCVV;
    float32_t3x4            _toWorld;
    float32_t3              _cameraPositionWorld;
    float32_t               _maxRayDistance;
    float32_t3              _cameraPositionSDF;
    float32_t               _invMaxRayDistanceFactor;
    float32_t4              _sdfParams;
    float32_t3              _viWorld;
    uint32_t                _tileCountWidth;
    float32_t3              _viSDF;
    uint32_t                _tileCounters;
    float32_t3              _sdfOffset;
    uint32_t                _tileSamples;
    ShapeInfo               _shapeInfo;
    uint64_t                _palette;
    TBN64                   _sdfOrientation;
};

[[vk::push_constant]]
PushConstants               g_pushConstants;

struct Attributes
{
    linear float32_t4       _vertexH:       SV_Position;

    [[vk::location ( ATT_SLOT_CANVAS )]]
    linear float32_t3       _canvas:        CANVAS;
};


#endif // GIZMO_PREPASS_COMMON_HLSL
