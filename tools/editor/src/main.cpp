#include <editor.hpp>
#include <stdlib.h>

GX_DISABLE_COMMON_WARNINGS

#include <memory>

GX_RESTORE_WARNING_STATE


[[nodiscard]] int main ( int argc, char** argv )
{
    std::unique_ptr<editor::Editor> editor = std::make_unique<editor::Editor> (
        editor::CommandLine ( argv, static_cast<size_t> ( argc ) )
    );

    return editor->Run () ? EXIT_SUCCESS : EXIT_FAILURE;
}
