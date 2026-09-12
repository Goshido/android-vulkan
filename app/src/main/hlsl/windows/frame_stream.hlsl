#ifndef FRAME_STREAM_HLSL
#define FRAME_STREAM_HLSL


#include "tbn64.hlsl"


struct Frame
{
    float32_t4x4                        _viewProj;
    TBN64                               _toView;
};

typedef vk::BufferPointer<Frame, 8U>    Frames;


#endif // FRAME_STREAM_HLSL
