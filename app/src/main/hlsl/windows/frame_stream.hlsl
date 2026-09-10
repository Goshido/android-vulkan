#ifndef FRAME_STREAM_HLSL
#define FRAME_STREAM_HLSL


struct Frame
{
    float32_t4x4                            _viewProj;
};

typedef vk::BufferPointer<Frame, 64U>       Frames;


#endif // FRAME_STREAM_HLSL
