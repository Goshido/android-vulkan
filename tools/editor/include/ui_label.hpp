#ifndef EDITOR_UI_LABEL_HPP
#define EDITOR_UI_LABEL_HPP


#include "div_ui_element.hpp"
#include "text_ui_element.hpp"
#include <pbr/svg_writer.hpp>


namespace editor {

class UILabel final
{
    private:
        DIVUIElement        _div;
        TextUIElement       _text;

    public:
        UILabel () = delete;

        UILabel ( UILabel const & ) = delete;
        UILabel &operator = ( UILabel const & ) = delete;

        UILabel ( UILabel && ) = delete;
        UILabel &operator = ( UILabel && ) = delete;

        explicit UILabel ( DIVUIElement &parent, std::string_view text, std::string &&name ) noexcept;

        explicit UILabel ( DIVUIElement &parent,
            pbr::CSSComputedValues &&css,
            std::string_view text,
            std::string &&name
        ) noexcept;

        ~UILabel () = default;

        [[nodiscard]] pbr::CSSComputedValues &GetCSS () noexcept;
        void DumpLayout ( pbr::SVGWriter &writer ) const noexcept;

        void SetText ( std::string const &text ) noexcept;
        void SetText ( std::string_view text ) noexcept;

        void SetColor ( pbr::ColorValue const &color ) noexcept;
};

} // namespace editor


#endif // EDITOR_UI_LABEL_HPP
