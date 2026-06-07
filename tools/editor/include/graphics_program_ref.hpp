#ifndef EDITOR_GRAPHICS_PROGRAM_REF_HPP
#define EDITOR_GRAPHICS_PROGRAM_REF_HPP


#include <platform/windows/pbr/graphics_program.hpp>

GX_DISABLE_COMMON_WARNINGS

#include <memory>

GX_RESTORE_WARNING_STATE


namespace editor {

using GraphicsProgramRef = std::unique_ptr<pbr::GraphicsProgram>;

} // namespace editor


#endif // EDITOR_GRAPHICS_PROGRAM_REF_HPP
