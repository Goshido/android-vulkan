#include <precompiled_headers.hpp>
#include <pbr/light.hpp>


namespace pbr {

eLightType Light::GetType () const
{
    return _type;
}

Light::Light ( eLightType type ) noexcept:
    _type ( type )
{
    // NOTHING
}

} // namespace pbr
