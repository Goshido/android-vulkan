#ifndef EDITOR_COLOR_DATA_HPP
#define EDITOR_COLOR_DATA_HPP


#include <vulkan_utils.hpp>


namespace editor {

AV_DX_ALIGNMENT_BEGIN

struct ColorData final
{
    uint32_t    _emiR : 8;
    uint32_t    _0rgb : 24;

    uint32_t    _emiB : 8;
    uint32_t    _1rgb : 24;

    uint32_t    _emiG : 8;
    uint32_t    _2rgb : 24;

    uint32_t    _0A : 8;
    uint32_t    _emiIntensity : 24;
};

AV_DX_ALIGNMENT_END

} // namespace editor


#endif // EDITOR_COLOR_DATA_HPP
