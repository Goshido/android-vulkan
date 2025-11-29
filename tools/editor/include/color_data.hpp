#ifndef EDITOR_COLOR_DATA_HPP
#define EDITOR_COLOR_DATA_HPP


#include <vulkan_utils.hpp>


namespace editor {

AV_DX_ALIGNMENT_BEGIN

struct ColorData final
{
    uint32_t    _emiRcol0rgb = 0U;
    uint32_t    _emiGcol1rgb = 0U;
    uint32_t    _emiBcol2rgb = 0U;
    uint32_t    _col0aEmiIntens = 0U;
};

AV_DX_ALIGNMENT_END

} // namespace editor


#endif // EDITOR_COLOR_DATA_HPP
