// FUCK - remove namespace
#ifndef PBR_WINDOWS_UI_VERTEX_INFO_HPP
#define PBR_WINDOWS_UI_VERTEX_INFO_HPP


#include <half_types.hpp>
#include <pbr/ui_primitive_type.inc>
#include "resource_heap.inc"

GX_DISABLE_COMMON_WARNINGS

#include <span>

GX_RESTORE_WARNING_STATE


namespace pbr::windows {

// [2025/09/25] The whole UI vertex is described by 18 bytes. There is no way to read float32_t from offset
// non multiple of 4. So it's needed to use SoA pattern. One example where flexible vertex pulling is worse
// than classical input assembly.

#pragma pack ( push, 1 )

struct UIVertexStream0 final
{
    GXVec2                      _position {};
    android_vulkan::Half2       _uv {};
    GXColorUNORM                _color {};
};

struct UIVertexStream1 final
{
    uint16_t                    _image: UI_IMAGE_BITS = 0U;
    uint16_t                    _uiPrimitiveType: ( 16 - UI_IMAGE_BITS ) = PBR_UI_PRIMITIVE_TYPE_TEXT;
};

#pragma pack ( pop )

using UIVertexBufferStream0 = std::span<UIVertexStream0>;
using UIVertexBufferStream1 = std::span<UIVertexStream1>;

struct UIBufferStreams final
{
    UIVertexBufferStream0       _stream0 {};
    UIVertexBufferStream1       _stream1 {};
};

} // namespace pbr::windows


#endif // PBR_WINDOWS_UI_VERTEX_INFO_HPP
