#ifndef EDITOR_ROTATE_TOOL_HPP
#define EDITOR_ROTATE_TOOL_HPP


#include "tool.hpp"


namespace editor {

class RotateTool final : public Tool
{
    public:
        RotateTool () = default;

        RotateTool ( RotateTool const & ) = delete;
        RotateTool &operator = ( RotateTool const & ) = delete;

        RotateTool ( RotateTool && ) = delete;
        RotateTool &operator = ( RotateTool && ) = delete;

        ~RotateTool () = default;

        void Activate () noexcept override;
        void Deactivate () noexcept override;

        void Click () noexcept override;
        void Begin () noexcept override;
        void Move () noexcept override;
        void End () noexcept override;
        void Cancel () noexcept override;
};

} // namespace editor


#endif // EDITOR_ROTATE_TOOL_HPP
