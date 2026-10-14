#ifndef EDITOR_TRANSFORM_TOOL_HPP
#define EDITOR_TRANSFORM_TOOL_HPP


#include "hotkey.hpp"
#include "ui_transform.hpp"
#include "save_state.hpp"
#include "selection.hpp"


namespace editor {

class TransformTool final
{
    private:
        struct Item final
        {
            GXVec3              _location {};
            GXVec3              _scale {};
        };

        using Float = std::optional<float>;

    private:
        UITransform*            _ui = nullptr;
        Hotkey                  _toggle {};
        Rect                    _rect {};
        std::vector<Item>       _updated {};
        std::vector<Item>       _items {};

        Float                   _x = std::nullopt;
        Float                   _y = std::nullopt;
        Float                   _z = std::nullopt;

        Float                   _scaleX = std::nullopt;
        Float                   _scaleY = std::nullopt;
        Float                   _scaleZ = std::nullopt;

    public:
        TransformTool () = default;

        TransformTool ( TransformTool const & ) = delete;
        TransformTool &operator = ( TransformTool const & ) = delete;

        TransformTool ( TransformTool && ) = delete;
        TransformTool &operator = ( TransformTool && ) = delete;

        ~TransformTool () = default;

        void Init ( SaveState::Container const &save ) noexcept;
        void Destroy ( SaveState::Container &save ) noexcept;

        void OnContentUpdated ( Selection::Actors const &actors ) noexcept;
        void OnSelectionChanged ( Selection::Actors const &actors ) noexcept;

    private:
        void CreateUI () noexcept;
        void UpdateUI () noexcept;

        void NewLocation ( std::string const &value, Float &axis ) const noexcept;
        void CommitLocation () const noexcept;
        void RevertLocation () const noexcept;

        void CancelX () noexcept;
        void ChangeX ( std::string const &value ) noexcept;
        void NextX ( std::string const &value ) noexcept;

        void CancelY () noexcept;
        void ChangeY ( std::string const &value ) noexcept;
        void NextY ( std::string const &value ) noexcept;

        void CancelZ () noexcept;
        void ChangeZ ( std::string const &value ) noexcept;
        void NextZ ( std::string const &value ) noexcept;

        void CancelScaleX () noexcept;
        void ChangeScaleX ( std::string const &value ) noexcept;
        void NewScaleX ( std::string const &value ) noexcept;
        void NextScaleX ( std::string const &value ) noexcept;

        void CancelScaleY () noexcept;
        void ChangeScaleY ( std::string const &value ) noexcept;
        void NewScaleY ( std::string const &value ) noexcept;
        void NextScaleY ( std::string const &value ) noexcept;

        void CancelScaleZ () noexcept;
        void ChangeScaleZ ( std::string const &value ) noexcept;
        void NewScaleZ ( std::string const &value ) noexcept;
        void NextScaleZ ( std::string const &value ) noexcept;

        [[nodiscard]] static Float ToFloat ( std::string const &string ) noexcept;
};

} // namespace editor


#endif // EDITOR_TRANSFORM_TOOL_HPP
