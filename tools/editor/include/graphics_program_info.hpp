#ifndef EDITOR_GRAPHICS_PROGRAM_INFO_HPP
#define EDITOR_GRAPHICS_PROGRAM_INFO_HPP


#include "graphics_program_ref.hpp"

GX_DISABLE_COMMON_WARNINGS

#include <functional>

GX_RESTORE_WARNING_STATE


namespace editor {

using GraphicsProgramAddedNotify = std::move_only_function<void ( GraphicsProgramRef )>;

class GraphicsProgramInfo final
{
    public:
        GraphicsProgramRef              _program {};
        GraphicsProgramAddedNotify      _notify = nullptr;

    public:
        GraphicsProgramInfo () = delete;

        GraphicsProgramInfo ( GraphicsProgramInfo const & ) = delete;
        GraphicsProgramInfo &operator = ( GraphicsProgramInfo const & ) = delete;

        GraphicsProgramInfo ( GraphicsProgramInfo && ) = default;
        GraphicsProgramInfo &operator = ( GraphicsProgramInfo && ) = default;

        explicit GraphicsProgramInfo ( GraphicsProgramRef program,
            GraphicsProgramAddedNotify &&addedNotify
        ) noexcept;

        ~GraphicsProgramInfo () = default;
};

} // namespace editor


#endif // EDITOR_GRAPHICS_PROGRAM_INFO_HPP
