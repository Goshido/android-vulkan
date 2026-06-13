#ifndef EDITOR_SCALE_TOOL_HPP
#define EDITOR_SCALE_TOOL_HPP


#include "tool.hpp"


namespace editor {

class ScaleTool final : public Tool
{
    public:
        ScaleTool () = default;

        ScaleTool ( ScaleTool const & ) = delete;
        ScaleTool &operator = ( ScaleTool const & ) = delete;

        ScaleTool ( ScaleTool && ) = delete;
        ScaleTool &operator = ( ScaleTool && ) = delete;

        ~ScaleTool () = default;

        void Activate () noexcept override;
        void Deactivate () noexcept override;

        void Click () noexcept override;
        void Begin () noexcept override;
        void Move () noexcept override;
        void End () noexcept override;
        void Cancel () noexcept override;
};

} // namespace editor


#endif // EDITOR_SCALE_TOOL_HPP
