// version 1.6

#include <GXCommon/GXMath.h>

GX_DISABLE_COMMON_WARNINGS

#include <cassert>

GX_RESTORE_WARNING_STATE


[[maybe_unused]] GXVoid GXMat4::Perspective ( GXFloat fieldOfViewYRadians,
    GXFloat aspectRatio,
    GXFloat zNear,
    GXFloat zFar
) noexcept
{
    // The implementation is using reverse Z trick.
    // See https://developer.nvidia.com/content/depth-precision-visualized

    GXFloat const halfFovy = fieldOfViewYRadians * 0.5F;
    GXFloat const ctan = std::cos ( halfFovy ) / std::sin ( halfFovy );
    GXFloat const invRange = 1.0F / ( zNear - zFar );

    _m[ 0U ][ 0U ] = ctan / aspectRatio;
    _m[ 1U ][ 1U ] = -ctan;
    _m[ 2U ][ 2U ] = invRange * zNear;
    _m[ 2U ][ 3U ] = 1.0F;
    _m[ 3U ][ 2U ] = -invRange * zFar * zNear;

    _m[ 0U ][ 1U ] = _m[ 0U ][ 2U ] = _m[ 0U ][ 3U ] = 0.0F;
    _m[ 1U ][ 0U ] = _m[ 1U ][ 2U ] = _m[ 1U ][ 3U ] = 0.0F;
    _m[ 2U ][ 0U ] = _m[ 2U ][ 1U ] = 0.0F;
    _m[ 3U ][ 0U ] = _m[ 3U ][ 1U ] = _m[ 3U ][ 3U ] = 0.0F;
}

[[maybe_unused]] GXVoid GXMat4::Ortho ( GXFloat width, GXFloat height, GXFloat zNear, GXFloat zFar ) noexcept
{
    GXFloat const invRange = 1.0f / ( zFar - zNear );

    _m[ 0U ][ 0U ] = 2.0F / width;
    _m[ 1U ][ 1U ] = -2.0F / height;
    _m[ 2U ][ 2U ] = invRange;
    _m[ 3U ][ 2U ] = -invRange * zNear;

    _m[ 0U ][ 1U ] = _m[ 0U ][ 2U ] = _m[ 0U ][ 3U ] = 0.0F;
    _m[ 1U ][ 0U ] = _m[ 1U ][ 2U ] = _m[ 1U ][ 3U ] = 0.0F;
    _m[ 2U ][ 0U ] = _m[ 2U ][ 1U ] = _m[ 2U ][ 3U ] = 0.0F;
    _m[ 3U ][ 0U ] = _m[ 3U ][ 1U ] = 0.0F;
    _m[ 3U ][ 3U ] = 1.0F;
}

// NOLINTNEXTLINE
[[maybe_unused]] GXVoid GXMat4::GetPerspectiveParams ( GXFloat& /*fieldOfViewYRadians*/,
    GXFloat& /*aspectRatio*/,
    GXFloat& /*zNear*/,
    GXFloat& /*zFar*/
) noexcept
{
    assert ( !"GXMat4::GetPerspectiveParams - Implement me!" );
}

// NOLINTNEXTLINE
[[maybe_unused]] GXVoid GXMat4::GetOrthoParams ( GXFloat& /*width*/,
    GXFloat& /*height*/,
    GXFloat& /*zNear*/,
    GXFloat& /*zFar*/
) noexcept
{
    assert ( !"GXMat4::GetOrthoParams - Implement me!" );
}

// NOLINTNEXTLINE
[[maybe_unused]] GXVoid GXMat4::GetRayPerspective ( GXVec3& /*rayView*/, const GXVec2& /*mouseCVV*/ ) const noexcept
{
    assert ( !"GXMat4::GetRayPerspective - Implement me!" );
}
