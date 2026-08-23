#include <precompiled_headers.hpp>
#include <sdf.hpp>


namespace editor {

SDF::SDF ( GXVec3 &&location, GXVec3 &&scale, eSDFPalette palette ) noexcept:
    _location ( std::move ( location ) ),
    _scale ( std::move ( scale ) ),
    _palette ( palette )
{
    // NOTHING
}

GXVec3 const &SDF::GetScale () const noexcept
{
    return _scale;
}

void SDF::SetScale ( GXVec3 const &scale ) noexcept
{
    _scale = scale;
}

GXVec3 const &SDF::GetLocationWorld () const noexcept
{
    return _locationWorld;
}

GXQuat const &SDF::GetRotationWorld () const noexcept
{
    return _rotationWorld;
}

void SDF::SetColor ( eSDFPalette palette ) noexcept
{
    _palette = palette;
}

void SDF::Hide () noexcept
{
    _node = {};
}

} // namespace editor
