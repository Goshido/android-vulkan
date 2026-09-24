#ifndef EDITOR_TOOL_HPP
#define EDITOR_TOOL_HPP


#include "selection.hpp"


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

        virtual void Begin ( Selection::Actors &actors, GXQuat const &rotation ) noexcept = 0;
        virtual void End () noexcept = 0;
        virtual void Cancel () noexcept = 0;

    protected:
        explicit Tool () = default;
        virtual ~Tool () = default;

        [[nodiscard]] static GXVec3 GetCenter ( Selection::Actors const &actors ) noexcept;
};

} // namespace editor


#endif // EDITOR_TOOL_HPP
