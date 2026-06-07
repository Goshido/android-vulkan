#include <precompiled_headers.hpp>
#include <graphics_program_info.hpp>


namespace editor {

GraphicsProgramInfo::GraphicsProgramInfo ( GraphicsProgramRef program,
    GraphicsProgramAddedNotify &&addedNotify ) noexcept:
    _program ( std::move ( program ) ),
    _notify ( std::move ( addedNotify ) )
{
    // NOTHING
}

} // namespace editor
