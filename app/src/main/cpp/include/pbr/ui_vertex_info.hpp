#ifndef PBR_UI_VERTEX_INFO_HPP
#define PBR_UI_VERTEX_INFO_HPP


#include <GXCommon/GXMath.hpp>

GX_DISABLE_COMMON_WARNINGS

#include <span>

GX_RESTORE_WARNING_STATE


namespace pbr {

#pragma pack ( push, 1 )

struct UIAtlas final
{
    GXVec2      _uv;
    uint8_t     _layer;

    // See <repo>/docs/data-compression.md#opt-2-further-compression
    uint8_t     _padding[ 3U ] {};
};

struct UIVertex final
{
    GXVec2          _image;
    UIAtlas         _atlas;
    GXColorUNORM    _color;
};

#pragma pack ( pop )

struct UIVertexBuffer final
{
    std::span<GXVec2>       _positions {};
    std::span<UIVertex>     _vertices {};
};

} // namespace pbr


#endif // PBR_UI_VERTEX_INFO_HPP
