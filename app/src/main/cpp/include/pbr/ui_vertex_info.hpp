#ifndef PBR_UI_VERTEX_INFO_HPP
#define PBR_UI_VERTEX_INFO_HPP


#include <half_types.hpp>

GX_DISABLE_COMMON_WARNINGS

#include <span>

GX_RESTORE_WARNING_STATE


namespace pbr {

#pragma pack ( push, 1 )

struct UIAtlas final
{
    android_vulkan::Half2       _uv {};
    android_vulkan::Half        _layer { 0U };
};

struct UIVertex final
{
    android_vulkan::Half2       _image {};
    UIAtlas                     _atlas {};
    GXColorUNORM                _color {};
};

#pragma pack ( pop )

struct UIVertexBuffer final
{
    std::span<GXVec2>       _positions {};
    std::span<UIVertex>     _vertices {};
};

} // namespace pbr


#endif // PBR_UI_VERTEX_INFO_HPP
