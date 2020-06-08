// version 1.0

#include <GXCommon/GXMath.h>

GX_DISABLE_COMMON_WARNINGS

#include <cassert>

GX_RESTORE_WARNING_STATE


GXVoid GXMat4::Perspective ( GXFloat fieldOfViewYRadiands, GXFloat aspectRatio, GXFloat zNear, GXFloat zFar )
{
    const GXFloat halfFovy = fieldOfViewYRadiands * 0.5f;
    const GXFloat ctan = cosf ( halfFovy ) / sinf ( halfFovy );
    const GXFloat invRange = 1.0f / ( zFar - zNear );

    _m[ 0u ][ 0u ] = ctan / aspectRatio;
    _m[ 1u ][ 1u ] = -ctan;
    _m[ 2u ][ 2u ] = invRange * zFar;
    _m[ 2u ][ 3u ] = 1.0f;
    _m[ 3u ][ 2u ] = -invRange * zNear * zFar;

    _m[ 0u ][ 1u ] = _m[ 0u ][ 2u ] = _m[ 0u ][ 3u ] = 0.0f;
    _m[ 1u ][ 0u ] = _m[ 1u ][ 2u ] = _m[ 1u ][ 3u ] = 0.0f;
    _m[ 2u ][ 0u ] = _m[ 2u ][ 1u ] = 0.0f;
    _m[ 3u ][ 0u ] = _m[ 3u ][ 1u ] = _m[ 3u ][ 3u ] = 0.0f;
}

GXVoid GXMat4::Ortho ( GXFloat width, GXFloat height, GXFloat zNear, GXFloat zFar )
{
    const GXFloat invRange = 1.0f / ( zFar - zNear );

    _m[ 0u ][ 0u ] = 2.0f / width;
    _m[ 1u ][ 1u ] = -2.0f / height;
    _m[ 2u ][ 2u ] = invRange;
    _m[ 3u ][ 2u ] = -invRange * zNear;

    _m[ 0u ][ 1u ] = _m[ 0u ][ 2u ] = _m[ 0u ][ 3u ] = 0.0f;
    _m[ 1u ][ 0u ] = _m[ 1u ][ 2u ] = _m[ 1u ][ 3u ] = 0.0f;
    _m[ 2u ][ 0u ] = _m[ 2u ][ 1u ] = _m[ 2u ][ 3u ] = 0.0f;
    _m[ 3u ][ 0u ] = _m[ 3u ][ 1u ] = 0.0f;
    _m[ 3u ][ 3u ] = 1.0f;
}

GXVoid GXMat4::GetPerspectiveParams ( GXFloat& /*fieldOfViewYRadiands*/,
    GXFloat& /*aspectRatio*/,
    GXFloat& /*zNear*/,
    GXFloat& /*zFar*/
)
{
    assert ( !"GXMat4::GetPerspectiveParams - Implement me!" );
}

GXVoid GXMat4::GetOrthoParams ( GXFloat& /*width*/, GXFloat& /*height*/, GXFloat& /*zNear*/, GXFloat& /*zFar*/ )
{
    assert ( !"GXMat4::GetOrthoParams - Implement me!" );
}

GXVoid GXMat4::GetRayPerspective ( GXVec3& /*rayView*/, const GXVec2& /*mouseCVV*/ ) const
{
    assert ( !"GXMat4::GetRayPerspective - Implement me!" );
}
