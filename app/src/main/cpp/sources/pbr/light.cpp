#include <pbr/light.h>


namespace pbr {

[[maybe_unused]] eLightType Light::GetType () const
{
    return _type;
}

[[maybe_unused]] Light::Light ( eLightType type ) noexcept:
    _type ( type )
{
    // NOTHING
}

} // namespace pbr
