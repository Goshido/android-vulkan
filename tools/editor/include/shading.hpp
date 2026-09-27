#ifndef EDITOR_SHADING_HPP
#define EDITOR_SHADING_HPP


#include "color_data.hpp"
#include <vulkan_utils.hpp>


namespace editor {

AV_DX_ALIGNMENT_BEGIN

struct Shading final
{
    uint32_t        _albedo;
    uint32_t        _emission;
    uint32_t        _mask;
    uint32_t        _param;
    uint32_t        _normal;
    ColorData       _colors;
};

AV_DX_ALIGNMENT_END

} // namespace editor


#endif // EDITOR_SHADING_HPP
