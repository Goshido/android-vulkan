#ifndef EDITOR_TRANSFORM_TOOL_HPP
#define EDITOR_TRANSFORM_TOOL_HPP


#include "hotkey.hpp"
#include "ui_transform.hpp"
#include "save_state.hpp"


namespace editor {

class TransformTool final
{
    private:
        UITransform*    _ui = nullptr;
        Hotkey          _toggle {};
        Rect            _rect {};

    public:
        TransformTool () = default;

        TransformTool ( TransformTool const & ) = delete;
        TransformTool &operator = ( TransformTool const & ) = delete;

        TransformTool ( TransformTool && ) = delete;
        TransformTool &operator = ( TransformTool && ) = delete;

        ~TransformTool () = default;

        void Init ( SaveState::Container const &save ) noexcept;
        void Destroy ( SaveState::Container &save ) noexcept;

    private:
        void CreateUI () noexcept;
};

} // namespace editor


#endif // EDITOR_TRANSFORM_TOOL_HPP
