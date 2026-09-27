#ifndef EDITOR_PROGRAM_REF_HPP
#define EDITOR_PROGRAM_REF_HPP


#include <pbr/program.hpp>

GX_DISABLE_COMMON_WARNINGS

#include <memory>

GX_RESTORE_WARNING_STATE


namespace editor {

using ProgramRef = std::unique_ptr<pbr::Program>;

} // namespace editor


#endif // EDITOR_PROGRAM_REF_HPP
