#ifndef EDITOR_SELECT_TOOL_HPP
#define EDITOR_SELECT_TOOL_HPP


#include <GXCommon/GXMath.hpp>
#include "tool.hpp"


namespace editor {

// FUCK - is it needed?
class SelectTool final : public Tool
{
    public:
        explicit SelectTool () = default;

        SelectTool ( SelectTool const & ) = delete;
        SelectTool &operator = ( SelectTool const & ) = delete;

        SelectTool ( SelectTool && ) = delete;
        SelectTool &operator = ( SelectTool && ) = delete;

        ~SelectTool () override = default;

        void Activate () noexcept override;
        void Deactivate () noexcept override;

        void Begin ( Selection::Items &items, GXQuat const &rotation ) noexcept override;
        void End () noexcept override;
        void Cancel () noexcept override;
};

} // namespace editor


#endif // EDITOR_SELECT_TOOL_HPP
