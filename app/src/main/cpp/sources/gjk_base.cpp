#include <precompiled_headers.hpp>
#include <gjk_base.hpp>


namespace android_vulkan {

Simplex const &GJKBase::GetSimplex () const noexcept
{
    return _simplex;
}

uint16_t GJKBase::GetSteps () const noexcept
{
    return _steps;
}

[[maybe_unused]] uint16_t GJKBase::GetTestLines () const noexcept
{
    return _testLine;
}

[[maybe_unused]] uint16_t GJKBase::GetTestTetrahedrons () const noexcept
{
    return _testTetrahedron;
}

[[maybe_unused]] uint16_t GJKBase::GetTestTriangles () const noexcept
{
    return _testTriangle;
}

void GJKBase::ResetInternal () noexcept
{
    _simplex._pointCount = 0U;
    _steps = 0U;
    _testLine = 0U;
    _testTetrahedron = 0U;
    _testTriangle = 0U;
}

} // namespace android_vulkan
