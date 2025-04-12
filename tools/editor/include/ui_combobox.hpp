#ifndef EDITOR_UI_COMBOBOX_HPP
#define EDITOR_UI_COMBOBOX_HPP


#include "text_ui_element.hpp"
#include "widget.hpp"

GX_DISABLE_COMMON_WARNINGS

#include <functional>

GX_RESTORE_WARNING_STATE


namespace editor {

class UIManager;

class UICombobox final : public Widget
{
    public:
        using ID = uint32_t;
        using Callback = std::function<void ( ID )>;

        struct Item final
        {
            std::string_view                        _caption {};
            ID                                      _id = 0U;
        };

        using Items = std::span<Item const>;

    private:
        constexpr static size_t NO_INDEX = std::numeric_limits<size_t>::max ();

        using MouseKeyHandler = void ( UICombobox::* ) ( MouseKeyEvent const &event ) noexcept;
        using MouseMoveHandler = void ( UICombobox::* ) ( MouseMoveEvent const &event ) noexcept;
        using UpdateRectHandler = void ( UICombobox::* ) () noexcept;

        class MenuItem final
        {
            public:
                std::unique_ptr<DIVUIElement>       _div {};
                std::unique_ptr<TextUIElement>      _text {};

            public:
                MenuItem () = delete;

                MenuItem ( MenuItem const & ) = delete;
                MenuItem &operator = ( MenuItem const & ) = delete;

                MenuItem ( MenuItem && ) = default;
                MenuItem &operator = ( MenuItem && ) = default;

                explicit MenuItem ( std::unique_ptr<DIVUIElement> &&div,
                    std::unique_ptr<TextUIElement> &&text
                ) noexcept;

                ~MenuItem () = default;
        };

    private:
        DIVUIElement                                _lineDIV;
        DIVUIElement                                _columnDIV;

        DIVUIElement                                _captionDIV;
        TextUIElement                               _captionText;

        DIVUIElement                                _valueDIV;

        DIVUIElement                                _textDIV;
        TextUIElement                               _text;

        DIVUIElement                                _iconDIV;
        TextUIElement                               _icon;

        DIVUIElement                                _menuAnchorDIV;
        DIVUIElement                                _menuDIV;
        std::vector<MenuItem>                       _menuItems {};

        Callback                                    _callback {};
        size_t                                      _eventID = 0U;

        Items const                                 _items;
        size_t                                      _focused = NO_INDEX;
        size_t                                      _selected = NO_INDEX;
        size_t                                      _targeted = NO_INDEX;

        MouseKeyHandler                             _onMouseKeyDown = &UICombobox::OnMouseKeyDownNormal;
        MouseKeyHandler                             _onMouseKeyUp = &UICombobox::OnMouseKeyUpNormal;
        MouseMoveHandler                            _onMouseMove = &UICombobox::OnMouseMoveNormal;
        UpdateRectHandler                           _updateRect = &UICombobox::UpdatedRectNormal;

    public:
        UICombobox () = delete;

        UICombobox ( UICombobox const & ) = delete;
        UICombobox &operator = ( UICombobox const & ) = delete;

        UICombobox ( UICombobox && ) = delete;
        UICombobox &operator = ( UICombobox && ) = delete;

        explicit UICombobox ( MessageQueue &messageQueue,
            DIVUIElement &parent,
            std::string_view caption,
            Items items,
            uint32_t selected,
            std::string &&name
        ) noexcept;

        ~UICombobox () override = default;

        void OnMouseKeyDown ( MouseKeyEvent const &event ) noexcept override;
        void OnMouseKeyUp ( MouseKeyEvent const &event ) noexcept override;
        void OnMouseMove ( MouseMoveEvent const &event ) noexcept override;
        void UpdatedRect () noexcept override;

        void Connect ( Callback &&callback ) noexcept;

    private:
        void OnMouseLeave () noexcept override;

        void OnMouseKeyDownMenu ( MouseKeyEvent const &event ) noexcept;
        void OnMouseKeyUpMenu ( MouseKeyEvent const &event ) noexcept;
        void OnMouseMoveMenu ( MouseMoveEvent const &event ) noexcept;
        void UpdatedRectMenu () noexcept;

        void OnMouseKeyDownNormal ( MouseKeyEvent const &event ) noexcept;
        void OnMouseKeyUpNormal ( MouseKeyEvent const &event ) noexcept;
        void OnMouseMoveNormal ( MouseMoveEvent const &event ) noexcept;
        void UpdatedRectNormal () noexcept;

        void SwitchToNormalState () noexcept;
        void SwitchToMenuState () noexcept;
};

} // namespace editor


#endif // EDITOR_UI_COMBOBOX_HPP
