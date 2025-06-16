#ifndef EDITOR_UI_SLIDER_HPP
#define EDITOR_UI_SLIDER_HPP


#include "text_ui_element.hpp"
#include "widget.hpp"

GX_DISABLE_COMMON_WARNINGS

#include <functional>

GX_RESTORE_WARNING_STATE


namespace editor {

class UISlider final : public Widget
{
    public:
        using Callback = std::function<void ( double )>;

    private:
        using MouseButtonHandler = void ( UISlider::* ) ( MouseButtonEvent const &event ) noexcept;
        using MouseMoveHandler = void ( UISlider::* ) ( MouseMoveEvent const &event ) noexcept;

    private:
        DIVUIElement            _lineDIV;
        DIVUIElement            _columnDIV;

        DIVUIElement            _captionDIV;
        TextUIElement           _captionText;

        DIVUIElement            _valueDIV;
        DIVUIElement            _progressDIV;

        DIVUIElement            _numberDIV;
        TextUIElement           _number;

        Callback                _callback {};
        size_t                  _eventID = 0U;

        double                  _minValue = 0.0;
        double                  _maxValue = 1.0;
        double                  _range = 1.0;
        double                  _value = 0.0;
        double                  _defaultValue = 0.0;

        MouseButtonHandler      _onMouseKeyDown = &UISlider::OnMouseButtonDownNormal;
        MouseButtonHandler      _onMouseKeyUp = &UISlider::OnMouseButtonUpNormal;
        MouseMoveHandler        _onMouseMove = &UISlider::OnMouseMoveNormal;

    public:
        UISlider () = delete;

        UISlider ( UISlider const & ) = delete;
        UISlider &operator = ( UISlider const & ) = delete;

        UISlider ( UISlider && ) = delete;
        UISlider &operator = ( UISlider && ) = delete;

        explicit UISlider ( MessageQueue &messageQueue,
            DIVUIElement &parent,
            std::string_view caption,
            double minValue,
            double maxValue,
            double value,
            double defaultValue,
            std::string &&name
        ) noexcept;

        ~UISlider () override = default;

        void OnMouseButtonDown ( MouseButtonEvent const &event ) noexcept override;
        void OnMouseButtonUp ( MouseButtonEvent const &event ) noexcept override;
        void OnMouseMove ( MouseMoveEvent const &event ) noexcept override;
        void UpdatedRect () noexcept override;

        void Connect ( Callback &&callback ) noexcept;

    private:
        void OnMouseLeave () noexcept override;

        void OnMouseButtonDownDrag ( MouseButtonEvent const &event ) noexcept;
        void OnMouseButtonUpDrag ( MouseButtonEvent const &event ) noexcept;
        void OnMouseMoveDrag ( MouseMoveEvent const &event ) noexcept;

        void OnMouseButtonDownNormal ( MouseButtonEvent const &event ) noexcept;
        void OnMouseButtonUpNormal ( MouseButtonEvent const &event ) noexcept;
        void OnMouseMoveNormal ( MouseMoveEvent const &event ) noexcept;

        void SwitchToDragState () noexcept;
        void SwitchToNormalState () noexcept;

        void UpdateProgress ( double progress ) noexcept;
        void UpdateValue ( int32_t mouseX ) noexcept;
};

} // namespace editor


#endif // EDITOR_UI_SLIDER_HPP
