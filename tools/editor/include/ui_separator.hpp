#ifndef EDITOR_UI_SEPARATOR_HPP
#define EDITOR_UI_SEPARATOR_HPP


#include "div_ui_element.hpp"
#include <pbr/svg_writer.hpp>


namespace editor {

class UISeparator final
{
    private:
        DIVUIElement    _div;

    public:
        UISeparator () = delete;

        UISeparator ( UISeparator const & ) = delete;
        UISeparator &operator = ( UISeparator const & ) = delete;

        UISeparator ( UISeparator && ) = delete;
        UISeparator &operator = ( UISeparator && ) = delete;

        explicit UISeparator ( DIVUIElement &parent, std::string &&name ) noexcept;

        ~UISeparator () = default;

        void DumpLayout ( pbr::SVGWriter &writer ) const noexcept;
};

} // namespace editor


#endif // EDITOR_UI_SEPARATOR_HPP
