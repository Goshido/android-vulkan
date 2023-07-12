#ifndef PBR_UI_VERTEX_INFO_HPP
#define PBR_UI_VERTEX_INFO_HPP


#include <GXCommon/GXMath.hpp>

GX_DISABLE_COMMON_WARNINGS

#include <span>

GX_RESTORE_WARNING_STATE


namespace pbr {

#pragma pack ( push, 1 )

struct UIVertexInfo final
{
    GXVec2          _vertex;
    GXColorRGB      _color;
    GXVec3          _atlas;
    GXVec2          _imageUV;
};

#pragma pack ( pop )

using UIVertexBuffer = std::span<UIVertexInfo>;

} // namespace pbr


#endif // PBR_UI_VERTEX_INFO_HPP
