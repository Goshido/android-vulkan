#ifndef PBR_TEXT_UI_ELEMENT_HPP
#define PBR_TEXT_UI_ELEMENT_HPP


#include <pbr/text_ui_element_base.hpp>
#include "ui_element.hpp"


namespace pbr {

struct TextGlyph final
{
    int32_t                     _advance = 0;

    android_vulkan::Half2       _atlasTopLeft {};
    android_vulkan::Half2       _atlasBottomRight {};
    uint8_t                     _atlasPage = 0U;

    int32_t                     _offsetX = 0;
    int32_t                     _offsetY = 0;
    size_t                      _parentLine = 0U;

    int32_t                     _width = 0;
    int32_t                     _height = 0;
};

using TextUIElement =
    TextUIElementBase<UIElement, TextGlyph, GlyphInfo, UIVertexStream0, UIVertexStream1, UIPass, FontStorage>;

} // namespace pbr


#endif // PBR_TEXT_UI_ELEMENT_HPP
