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

        void ChangeLocation ( std::string const &value, Float &axis ) const noexcept;
        void CommitLocation () noexcept;
        void NewLocation ( std::string const &value, Float &axis ) noexcept;
        void RevertLocation () const noexcept;

        void ChangeScale ( std::string const &value, Float &axis ) const noexcept;
        void CommitScale () noexcept;
        void NewScale ( std::string const &value, Float &axis ) noexcept;
        void RevertScale () const noexcept;

        // Note - sending 'before' by value is intentional.
        [[nodiscard]] std::optional<GXVec3> ResolveLocation ( GXVec3 before ) const noexcept;

        // Note - sending 'before' by value is intentional.
        [[nodiscard]] std::optional<GXVec3> ResolveScale ( GXVec3 before ) const noexcept;

        [[nodiscard]] static bool CheckAxis ( float &before, Float const &after ) noexcept;
        [[nodiscard]] static Float ToFloat ( std::string const &string ) noexcept;
};

} // namespace editor


#endif // EDITOR_TRANSFORM_TOOL_HPP
