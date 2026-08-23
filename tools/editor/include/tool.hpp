#ifndef EDITOR_TOOL_HPP
#define EDITOR_TOOL_HPP


namespace editor {

class Tool
{
    public:
        Tool ( Tool const & ) = delete;
        Tool &operator = ( Tool const & ) = delete;

        Tool ( Tool && ) = delete;
        Tool &operator = ( Tool && ) = delete;

        virtual void Activate () noexcept = 0;
        virtual void Deactivate () noexcept = 0;

        virtual void Hover () noexcept = 0;
        virtual void Click () noexcept = 0;
        virtual void Begin () noexcept = 0;
        virtual void Move () noexcept = 0;
        virtual void End () noexcept = 0;
        virtual void Cancel () noexcept = 0;

    protected:
        Tool () = default;
        ~Tool () = default;
};

} // namespace editor


#endif // EDITOR_TOOL_HPP
