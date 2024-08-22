#ifndef EDITOR_EDITOR_HPP
#define EDITOR_EDITOR_HPP


#include "command_line.hpp"


namespace editor {

class Editor final
{
    private:
        bool    _run = true;

    public:
        Editor () = delete;

        Editor ( Editor const & ) = delete;
        Editor &operator = ( Editor const & ) = delete;

        Editor ( Editor && ) = delete;
        Editor &operator = ( Editor && ) = delete;

        explicit Editor ( CommandLine &&args ) noexcept;

        ~Editor () = default;

        [[nodiscard]] bool Run () noexcept;
};

} // namespace editor


#endif // EDITOR_EDITOR_HPP
