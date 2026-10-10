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

    private:
        UITransform*            _ui = nullptr;
        Hotkey                  _toggle {};
        Rect                    _rect {};
        std::vector<Item>       _updated {};
        std::vector<Item>       _items {};

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
};

} // namespace editor


#endif // EDITOR_TRANSFORM_TOOL_HPP
