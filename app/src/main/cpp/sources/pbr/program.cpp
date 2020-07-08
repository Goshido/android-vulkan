#include <pbr/program.h>

namespace pbr {

Program::Program ( std::string &&name ):
    _fragmentShader ( VK_NULL_HANDLE ),
    _vertexShader ( VK_NULL_HANDLE ),
    _name ( std::move ( name ) ),
    _pipeline ( VK_NULL_HANDLE ),
    _pipelineLayout ( VK_NULL_HANDLE ),
    _state ( eProgramState::Unknown )
{
    // NOTHING
}

} // namespace pbr
