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

#pragma pack ( push, 1 )

struct UIVertex final
{
    GXVec2                      _position {};
    android_vulkan::Half2       _uv {};
    GXColorUNORM                _color {};
    uint16_t                    _image: UI_IMAGE_BITS = 0U;
    uint16_t                    _uiPrimitiveType: ( 16 - UI_IMAGE_BITS ) = PBR_UI_PRIMITIVE_TYPE_TEXT;
};

#pragma pack ( pop )

using UIVertexBuffer = std::span<UIVertex>;

} // namespace pbr::windows


#endif // PBR_WINDOWS_UI_VERTEX_INFO_HPP
