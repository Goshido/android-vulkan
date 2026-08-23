#ifndef EDITOR_SELECT_TOOL_HPP
#define EDITOR_SELECT_TOOL_HPP


#include "tool.hpp"


namespace editor {

class SelectTool final : public Tool
{
    public:
        SelectTool () = default;

        SelectTool ( SelectTool const & ) = delete;
        SelectTool &operator = ( SelectTool const & ) = delete;

        SelectTool ( SelectTool && ) = delete;
        SelectTool &operator = ( SelectTool && ) = delete;

        ~SelectTool () = default;

        void Activate () noexcept override;
        void Deactivate () noexcept override;

        void Hover () noexcept override;
        void Click () noexcept override;
        void Begin () noexcept override;
        void Move () noexcept override;
        void End () noexcept override;
        void Cancel () noexcept override;
};

} // namespace editor


#endif // EDITOR_SELECT_TOOL_HPP
