#include <editor.hpp>
#include <stdlib.h>


[[nodiscard]] int main ( int argc, char** argv )
{
    return editor::Editor ( editor::CommandLine ( argv, static_cast<size_t> ( argc ) ) ).Run () ?
        EXIT_SUCCESS :
        EXIT_FAILURE;
}
