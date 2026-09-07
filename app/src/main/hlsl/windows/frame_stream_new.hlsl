#ifndef FRAME_STREAM_HLSL
#define FRAME_STREAM_HLSL


struct Frame
{
    float32_t4x4    _viewProj;
};

// FUCK - try 54
typedef vk::BufferPointer<Frame, 4U>    Frames;


#endif // FRAME_STREAM_HLSL
