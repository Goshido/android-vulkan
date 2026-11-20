#ifndef EDITOR_UI_COMPONENT_HEADER_HPP
#define EDITOR_UI_COMPONENT_HEADER_HPP


#include "string_property.hpp"
#include "ui_label.hpp"
#include "widget.hpp"


namespace editor {

class UIComponentHeader final : public Widget
{
    private:
        DIVUIElement            _root;
        DIVUIElement            _info;

        UILabel                 _glyph;
        UILabel                 _name;

        UILabel                 _expand;
        DIVUIElement            _underline;

        size_t                  _eventID = 0U;
        bool                    &_collapse;
        bool                    &_select;

        StringProperty*         _property = nullptr;
        Property::Connection    _connection = 42U;

    public:
        UIComponentHeader () = delete;

        UIComponentHeader ( UIComponentHeader const & ) = delete;
        UIComponentHeader &operator = ( UIComponentHeader const & ) = delete;

        UIComponentHeader ( UIComponentHeader && ) = delete;
        UIComponentHeader &operator = ( UIComponentHeader && ) = delete;

        explicit UIComponentHeader ( DIVUIElement &parent,
            StringProperty &name,
            std::string_view glyph,
            bool &collapse,
            bool &select
        ) noexcept;

        ~UIComponentHeader () noexcept override;

        void OnMouseButtonDown ( MouseButtonEvent const &event ) noexcept override;
        void OnMouseButtonUp ( MouseButtonEvent const &event ) noexcept override;
        void OnMouseMove ( MouseMoveEvent const &event ) noexcept override;
        void UpdatedRect () noexcept override;

        void Disconnect () noexcept;
        void DumpLayout ( pbr::SVGWriter &writer ) const noexcept;
        void SetName ( std::string const &name ) noexcept;

    private:
        void OnMouseLeave () noexcept override;

        void SetColor ( pbr::ColorValue const &color ) noexcept;
};

} // namespace editor


#endif // EDITOR_UI_COMPONENT_HEADER_HPP
