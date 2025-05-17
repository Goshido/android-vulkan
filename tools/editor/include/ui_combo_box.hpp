#ifndef EDITOR_UI_COMBO_BOX_HPP
#define EDITOR_UI_COMBO_BOX_HPP


#include "text_ui_element.hpp"
#include "widget.hpp"

GX_DISABLE_COMMON_WARNINGS

#include <functional>

GX_RESTORE_WARNING_STATE


namespace editor {

class UIComboBox final : public Widget
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

        using MouseKeyHandler = void ( UIComboBox::* ) ( MouseKeyEvent const &event ) noexcept;
        using MouseMoveHandler = void ( UIComboBox::* ) ( MouseMoveEvent const &event ) noexcept;
        using UpdateRectHandler = void ( UIComboBox::* ) () noexcept;

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

        class Popup final : public Widget
        {
            private:
                DIVUIElement                        _div;
                std::vector<MenuItem>               _menuItems {};

                TextUIElement                       &_text;

                Callback                            &_callback;

                std::vector<float>                  _lineHeights { 1U, 0.0F };
                bool                                _isChanged = false;

                Items const                         _items;
                size_t                              _focused = NO_INDEX;
                size_t                              &_selected;
                size_t                              _targeted = NO_INDEX;

            public:
                Popup () = delete;

                Popup ( Popup const & ) = delete;
                Popup &operator = ( Popup const & ) = delete;

                Popup ( Popup && ) = delete;
                Popup &operator = ( Popup && ) = delete;

                explicit Popup ( MessageQueue &messageQueue,
                    DIVUIElement const &positionAnchor,
                    DIVUIElement const &widthAnchor,
                    Items items,
                    size_t &selected,
                    TextUIElement &text,
                    Callback &callback,
                    std::string const &name
                ) noexcept;

                ~Popup () = default;

                void OnMouseMove ( MouseMoveEvent const &event ) noexcept override;

                [[nodiscard]] LayoutStatus ApplyLayout ( android_vulkan::Renderer &renderer,
                    pbr::FontStorage &fontStorage
                ) noexcept override;

                void Submit ( pbr::UIElement::SubmitInfo &info ) noexcept override;

                [[nodiscard]] bool UpdateCache ( pbr::FontStorage &fontStorage,
                    VkExtent2D const &viewport ) noexcept override;

                // Returns true if popup menu should be closed. Otherwise the method returns false.
                [[nodiscard]] bool HandleMouseKeyDown ( MouseKeyEvent const &event ) noexcept;

                // Returns true if popup menu should be closed. Otherwise the method returns false.
                [[nodiscard]] bool HandleMouseKeyUp ( MouseKeyEvent const &event ) noexcept;

                [[nodiscard]] Rect const &HandleUpdatedRect () noexcept;
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

        Callback                                    _callback {};
        size_t                                      _eventID = 0U;

        std::string const                           _name;
        Items const                                 _items;
        size_t                                      _selected = NO_INDEX;

        MouseKeyHandler                             _onMouseKeyDown = &UIComboBox::OnMouseKeyDownNormal;
        MouseKeyHandler                             _onMouseKeyUp = &UIComboBox::OnMouseKeyUpNormal;
        MouseMoveHandler                            _onMouseMove = &UIComboBox::OnMouseMoveNormal;
        UpdateRectHandler                           _updateRect = &UIComboBox::UpdatedRectNormal;

        Popup*                                      _popup = nullptr;
        bool                                        _cancelNextLeftMouseKeyDownEvent = false;

    public:
        UIComboBox () = delete;

        UIComboBox ( UIComboBox const & ) = delete;
        UIComboBox &operator = ( UIComboBox const & ) = delete;

        UIComboBox ( UIComboBox && ) = delete;
        UIComboBox &operator = ( UIComboBox && ) = delete;

        explicit UIComboBox ( MessageQueue &messageQueue,
            DIVUIElement &parent,
            std::string_view caption,
            Items items,
            uint32_t selected,
            std::string &&name
        ) noexcept;

        ~UIComboBox () override = default;

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


#endif // EDITOR_UI_COMBO_BOX_HPP
