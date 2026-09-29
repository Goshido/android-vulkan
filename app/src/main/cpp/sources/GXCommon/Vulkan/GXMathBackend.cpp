// version 1.11

#include <precompiled_headers.hpp>
#include <GXCommon/GXMath.hpp>

[[maybe_unused]] GXVoid GXMat4::Perspective ( GXFloat fieldOfViewYRadians, GXFloat aspectRatio, GXFloat zNear ) noexcept
{
    // Reverse Z from 0 to 1, infinite far plane projection matrix.
    // See https://github.com/Goshido/android-vulkan/issues/104
    GXFloat const halfFovy = fieldOfViewYRadians * 0.5F;
    GXFloat const cot = std::cos ( halfFovy ) / std::sin ( halfFovy );

    auto &m = _data;

    m[ 0U ][ 0U ] = cot / aspectRatio;
    m[ 1U ][ 1U ] = -cot;

    m[ 2U ][ 2U ] = 0.0F;
    m[ 2U ][ 3U ] = 1.0F;
    m[ 3U ][ 2U ] = zNear;
    m[ 3U ][ 3U ] = 0.0F;

    m[ 0U ][ 1U ] = 0.0F;
    m[ 0U ][ 2U ] = 0.0F;
    m[ 0U ][ 3U ] = 0.0F;

    m[ 1U ][ 0U ] = 0.0F;
    m[ 1U ][ 2U ] = 0.0F;
    m[ 1U ][ 3U ] = 0.0F;

    m[ 2U ][ 0U ] = 0.0F;
    m[ 2U ][ 1U ] = 0.0F;

    m[ 3U ][ 0U ] = 0.0F;
    m[ 3U ][ 1U ] = 0.0F;
}

[[maybe_unused]] GXVoid GXMat4::Ortho ( GXFloat width, GXFloat height, GXFloat zNear, GXFloat zFar ) noexcept
{
    GXFloat const invRange = 1.0f / ( zFar - zNear );
    auto &m = _data;

    m[ 0U ][ 0U ] = 2.0F / width;
    m[ 1U ][ 1U ] = -2.0F / height;
    m[ 2U ][ 2U ] = invRange;
    m[ 3U ][ 2U ] = -invRange * zNear;
    m[ 3U ][ 3U ] = 1.0F;

    m[ 0U ][ 1U ] = 0.0F;
    m[ 0U ][ 2U ] = 0.0F;
    m[ 0U ][ 3U ] = 0.0F;
    m[ 1U ][ 0U ] = 0.0F;
    m[ 1U ][ 2U ] = 0.0F;
    m[ 1U ][ 3U ] = 0.0F;
    m[ 2U ][ 0U ] = 0.0F;
    m[ 2U ][ 1U ] = 0.0F;
    m[ 2U ][ 3U ] = 0.0F;
    m[ 3U ][ 0U ] = 0.0F;
    m[ 3U ][ 1U ] = 0.0F;
}
