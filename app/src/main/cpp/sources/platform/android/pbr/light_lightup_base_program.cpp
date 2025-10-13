#include <precompiled_headers.hpp>
#include <platform/android/pbr/light_lightup_base_program.hpp>


namespace pbr {

LightLightupBaseProgram::LightLightupBaseProgram ( std::string &&name ) noexcept:
    GraphicsProgram ( std::move ( name ) )
{
    // NOTHING
}

} // namespace pbr
