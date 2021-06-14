#include <simplex.h>

GX_DISABLE_COMMON_WARNINGS

#include <cstring>

GX_RESTORE_WARNING_STATE


namespace android_vulkan {

Simplex::Simplex () noexcept:
    _pointCount ( 0U ),
    _supportPoints {}
{
    // NOTHING
}

[[maybe_unused]] void Simplex::PushPoint ( GXVec3 const &supportPoint ) noexcept
{
    std::memmove ( _supportPoints + 1U, _supportPoints, _pointCount * sizeof ( GXVec3 ) );
    _supportPoints[ 0U ] = supportPoint;
    ++_pointCount;
}

GXVec3 Simplex::FindSupportPoint ( GXVec3 const &direction, Shape const &shapeA, Shape const &shapeB ) noexcept
{
    GXVec3 opposite ( direction );
    opposite.Reverse ();

    GXVec3 result {};
    result.Subtract ( shapeA.GetExtremePointWorld ( direction ), shapeB.GetExtremePointWorld ( opposite ) );

    return result;
}

} // namespace android_vulkan
