#ifndef UI_PUSH_CONSTANTS_HLSL
#define UI_PUSH_CONSTANTS_HLSL


#include "color_packing.hlsl"
#include "platform/windows/pbr/resource_heap.inc"


// [2025/09/25] The whole UI vertex is described by 18 bytes. There is no way to read float32_t from offset
// non multiple of 4. So it's needed to use SoA pattern. One example where flexible vertex pulling is worse
// than classical input assembly.
struct UIVertex0
{
    float32_t2      _position;
    float16_t2      _uv;
    ColorUNORM      _color;
};

typedef vk::BufferPointer<UIVertex0, 4U>    UIVertices0;

struct UIVertex1
{
    uint16_t                                _image: UI_IMAGE_BITS;
    uint16_t                                _uiPrimitiveType: ( 16 - UI_IMAGE_BITS );
};

typedef vk::BufferPointer<UIVertex1, 2U>    UIVertices1;

struct PushConstants
{
    UIVertices0                             _uiVertices0;
    UIVertices1                             _uiVertices1;
    float32_t2x2                            _rotateScale;
    float32_t2                              _offset;
    uint32_t                                _textLUT;
};

[[vk::push_constant]]
PushConstants                               g_pushConstants;


#endif // UI_PUSH_CONSTANTS_HLSL
