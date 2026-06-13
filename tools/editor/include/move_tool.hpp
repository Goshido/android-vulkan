#ifndef EDITOR_MOVE_TOOL_HPP
#define EDITOR_MOVE_TOOL_HPP


#include "tool.hpp"


namespace editor {

class MoveTool final : public Tool
{
    public:
        MoveTool () = default;

        MoveTool ( MoveTool const & ) = delete;
        MoveTool &operator = ( MoveTool const & ) = delete;

        MoveTool ( MoveTool && ) = delete;
        MoveTool &operator = ( MoveTool && ) = delete;

        ~MoveTool () = default;

        void Activate () noexcept override;
        void Deactivate () noexcept override;

        void Click () noexcept override;
        void Begin () noexcept override;
        void Move () noexcept override;
        void End () noexcept override;
        void Cancel () noexcept override;
};

} // namespace editor


#endif // EDITOR_MOVE_TOOL_HPP
