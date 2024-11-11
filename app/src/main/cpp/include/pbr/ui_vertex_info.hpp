#ifndef PBR_UI_VERTEX_INFO_HPP
#define PBR_UI_VERTEX_INFO_HPP


#include <GXCommon/GXMath.hpp>

GX_DISABLE_COMMON_WARNINGS

#include <span>

GX_RESTORE_WARNING_STATE


namespace pbr {

#pragma pack ( push, 1 )

struct UIVertex final
{
    GXColorRGB      _color;
    GXVec3          _atlas;
    GXVec2          _imageUV;
};

#pragma pack ( pop )

struct UIVertexBuffer final
{
    std::span<GXVec2>       _positions {};
    std::span<UIVertex>     _vertices {};
};

} // namespace pbr


#endif // PBR_UI_VERTEX_INFO_HPP
