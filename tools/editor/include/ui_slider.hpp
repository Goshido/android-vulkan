#ifndef EDITOR_UI_SLIDER_HPP
#define EDITOR_UI_SLIDER_HPP


#include <pbr/div_ui_element.hpp>
#include <pbr/text_ui_element.hpp>
#include "widget.hpp"

GX_DISABLE_COMMON_WARNINGS

#include <functional>

GX_RESTORE_WARNING_STATE


namespace editor {

class UIManager;

class UISlider final : public Widget
{
    public:
        using Callback = std::function<void ( double )>;

    private:
        using MouseKeyHandler = void ( UISlider::* ) ( MouseKeyEvent const &event ) noexcept;
        using MouseMoveHandler = void ( UISlider::* ) ( MouseMoveEvent const &event ) noexcept;

    private:
        pbr::DIVUIElement       _lineDIV;
        pbr::DIVUIElement       _columnDIV;

        pbr::DIVUIElement       _captionDIV;
        pbr::TextUIElement      _captionText;

        pbr::DIVUIElement       _valueDIV;
        pbr::DIVUIElement       _progressDIV;

        pbr::DIVUIElement       _numberDIV;
        pbr::TextUIElement      _number;

        Callback                _callback {};
        size_t                  _eventID = 0U;

        double                  _minValue = 0.0;
        double                  _maxValue = 1.0;
        double                  _range = 1.0;
        double                  _value = 0.0;
        double                  _defaultValue = 0.0;

        MouseKeyHandler         _onMouseKeyDown = &UISlider::OnMouseKeyDownNormal;
        MouseKeyHandler         _onMouseKeyUp = &UISlider::OnMouseKeyUpNormal;
        MouseMoveHandler        _onMouseMove = &UISlider::OnMouseMoveNormal;

    public:
        UISlider () = delete;

        UISlider ( UISlider const & ) = delete;
        UISlider &operator = ( UISlider const & ) = delete;

        UISlider ( UISlider && ) = delete;
        UISlider &operator = ( UISlider && ) = delete;

        explicit UISlider ( MessageQueue &messageQueue,
            pbr::DIVUIElement &parent,
            std::string_view caption,
            double minValue,
            double maxValue,
            double value,
            double defaultValue,
            std::string &&name
        ) noexcept;

        ~UISlider () override = default;

        void OnMouseKeyDown ( MouseKeyEvent const &event ) noexcept override;
        void OnMouseKeyUp ( MouseKeyEvent const &event ) noexcept override;
        void OnMouseMove ( MouseMoveEvent const &event ) noexcept override;
        void UpdatedRect () noexcept override;

        void Connect ( Callback &&callback ) noexcept;

    private:
        void OnMouseLeave () noexcept override;

        void OnMouseKeyDownDrag ( MouseKeyEvent const &event ) noexcept;
        void OnMouseKeyUpDrag ( MouseKeyEvent const &event ) noexcept;
        void OnMouseMoveDrag ( MouseMoveEvent const &event ) noexcept;

        void OnMouseKeyDownNormal ( MouseKeyEvent const &event ) noexcept;
        void OnMouseKeyUpNormal ( MouseKeyEvent const &event ) noexcept;
        void OnMouseMoveNormal ( MouseMoveEvent const &event ) noexcept;

        void SwitchToDragState () noexcept;
        void SwitchToNormalState () noexcept;

        void UpdateProgress ( double progress ) noexcept;
        void UpdateValue ( int32_t mouseX ) noexcept;
};

} // namespace editor


#endif // EDITOR_UI_SLIDER_HPP
