#include <precompiled_headers.hpp>
#include <program_info.hpp>


namespace editor {

ProgramInfo::ProgramInfo ( ProgramRef program, ProgramAddedNotify &&addedNotify ) noexcept:
    _program ( std::move ( program ) ),
    _notify ( std::move ( addedNotify ) )
{
    // NOTHING
}

} // namespace editor
