#ifndef EDITOR_SET_TEXT_EVENT_HPP
#define EDITOR_SET_TEXT_EVENT_HPP


#include <pbr/text_ui_element.hpp>


namespace editor {

class SetTextEvent final
{
    private:
        pbr::TextUIElement      &_element;
        std::string_view        _text {};

    public:
        SetTextEvent () = delete;

        SetTextEvent ( SetTextEvent const & ) = delete;
        SetTextEvent &operator = ( SetTextEvent const & ) = delete;

        SetTextEvent ( SetTextEvent && ) = delete;
        SetTextEvent &operator = ( SetTextEvent && ) = delete;

        ~SetTextEvent () = delete;

        void Apply () noexcept;

        [[nodiscard]] static SetTextEvent* Create ( pbr::TextUIElement &element, std::string_view text ) noexcept;
        static void Destroy ( SetTextEvent &event ) noexcept;

    private:
        explicit SetTextEvent ( pbr::TextUIElement &element, std::string_view text ) noexcept;
};

} // namespace editor


#endif // EDITOR_SET_TEXT_EVENT_HPP
