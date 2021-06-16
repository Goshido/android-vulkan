#include <epa.h>
#include <logger.h>


namespace android_vulkan {

EPA::EPA () noexcept:
    _depth ( 0.0F ),
    _normal ( 1.0F, 0.0F, 0.0F )
{
    // NOTHING
}

[[maybe_unused]] float EPA::GetDepth () const noexcept
{
    return _depth;
}

[[maybe_unused]] GXVec3 const& EPA::GetNormal () const noexcept
{
    return _normal;
}

void EPA::Reset () noexcept
{
    LogDebug ( "EPA::Reset - From me %p", this );
    // TODO
}

bool EPA::Run ( Simplex const &/*simplex*/, Shape const &/*shapeA*/, Shape const &/*shapeB*/ ) noexcept
{
    LogDebug ( "EPA::Run - From me %p", this );
    // TODO
    return false;
}

} // namespace android_vulkan
