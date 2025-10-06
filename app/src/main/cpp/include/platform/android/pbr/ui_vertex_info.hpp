#ifndef PBR_UI_VERTEX_INFO_HPP
#define PBR_UI_VERTEX_INFO_HPP


#include <half_types.hpp>
#include <pbr/ui_primitive_type.inc>

GX_DISABLE_COMMON_WARNINGS

#include <span>

GX_RESTORE_WARNING_STATE


namespace pbr {

#pragma pack ( push, 1 )

using UIVertexStream0 = GXVec2;

struct UIVertexStream1 final
{
    android_vulkan::Half2       _uv {};
    uint8_t                     _atlasLayer = 0U;
    uint8_t                     _uiPrimitiveType = PBR_UI_PRIMITIVE_TYPE_TEXT;
    GXColorUNORM                _color {};
};

#pragma pack ( pop )

using UIVertexBufferStream0 = std::span<UIVertexStream0>;
using UIVertexBufferStream1 = std::span<UIVertexStream1>;

struct UIBufferStreams final
{
    UIVertexBufferStream0       _stream0 {};
    UIVertexBufferStream1       _stream1 {};
};

} // namespace pbr


#endif // PBR_UI_VERTEX_INFO_HPP
