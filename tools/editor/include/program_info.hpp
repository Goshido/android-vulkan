#ifndef EDITOR_PROGRAM_INFO_HPP
#define EDITOR_PROGRAM_INFO_HPP


#include "program_ref.hpp"

GX_DISABLE_COMMON_WARNINGS

#include <functional>

GX_RESTORE_WARNING_STATE


namespace editor {

using ProgramAddedNotify = std::move_only_function<void ( ProgramRef )>;

class ProgramInfo final
{
    public:
        ProgramRef              _program {};
        ProgramAddedNotify      _notify = nullptr;

    public:
        ProgramInfo () = delete;

        ProgramInfo ( ProgramInfo const & ) = delete;
        ProgramInfo &operator = ( ProgramInfo const & ) = delete;

        ProgramInfo ( ProgramInfo && ) = default;
        ProgramInfo &operator = ( ProgramInfo && ) = default;

        explicit ProgramInfo ( ProgramRef program, ProgramAddedNotify &&addedNotify ) noexcept;

        ~ProgramInfo () = default;
};

} // namespace editor


#endif // EDITOR_PROGRAM_INFO_HPP
