#include <precompiled_headers.hpp>
#include <simplex.hpp>


namespace android_vulkan {

void Simplex::PushPoint ( GXVec3 const &supportPoint ) noexcept
{
    std::memmove ( _supportPoints + 1U, _supportPoints, _pointCount * sizeof ( GXVec3 ) );
    _supportPoints[ 0U ] = supportPoint;
    ++_pointCount;
}

} // namespace android_vulkan
