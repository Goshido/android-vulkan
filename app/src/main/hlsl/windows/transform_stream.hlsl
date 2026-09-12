#ifndef TRANSFORM_STREAM_HLSL
#define TRANSFORM_STREAM_HLSL


#include "tbn64.hlsl"


struct Transform
{
    TBN64           _rotation;
    float32_t3      _location;
    float32_t3      _scale;
};

typedef vk::BufferPointer<Transform, 8U>        Transforms;


#endif // TRANSFORM_STREAM_HLSL
