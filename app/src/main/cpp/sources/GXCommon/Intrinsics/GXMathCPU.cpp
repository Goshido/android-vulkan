// version 1.4

#include <GXCommon/GXMath.h>

GX_DISABLE_COMMON_WARNINGS

#include <cassert>

GX_RESTORE_WARNING_STATE


[[maybe_unused]] GXVoid GXVec2::CalculateNormalFast ( GXVec2 const &a, GXVec2 const &b ) noexcept
{
    _data[ 0U ] = a._data[ 1U ] - b._data[ 1U ];
    _data[ 1U ] = b._data[ 0U ] - a._data[ 0U ];
}

[[maybe_unused]] GXFloat GXVec2::DotProduct ( GXVec2 const &other ) const noexcept
{
    return _data[ 0U ] * other._data[ 0U ] + _data[ 1U ] * other._data[ 1U ];
}

[[maybe_unused]] GXVoid GXVec2::Sum ( GXVec2 const &a, GXVec2 const &b ) noexcept
{
    _data[ 0U ] = a._data[ 0U ] + b._data[ 0U ];
    _data[ 1U ] = a._data[ 1U ] + b._data[ 1U ];
}

[[maybe_unused]] GXVoid GXVec2::Sum ( GXVec2 const &a, GXFloat bScale, GXVec2 const &b ) noexcept
{
    _data[ 0U ] = a._data[ 0U ] + bScale * b._data[ 0U ];
    _data[ 1U ] = a._data[ 1U ] + bScale * b._data[ 1U ];
}

[[maybe_unused]] GXVoid GXVec2::Subtract ( GXVec2 const &a, GXVec2 const &b ) noexcept
{
    _data[ 0U ] = a._data[ 0U ] - b._data[ 0U ];
    _data[ 1U ] = a._data[ 1U ] - b._data[ 1U ];
}

[[maybe_unused]] GXVoid GXVec2::Multiply ( GXVec2 const &a, GXVec2 const &b ) noexcept
{
    _data[ 0U ] = a._data[ 0U ] * b._data[ 0U ];
    _data[ 1U ] = a._data[ 1U ] * b._data[ 1U ];
}

[[maybe_unused]] GXVoid GXVec2::Multiply ( GXVec2 const &v, GXFloat scale ) noexcept
{
    _data[ 0U ] = v._data[ 0U ] * scale;
    _data[ 1U ] = v._data[ 1U ] * scale;
}

//----------------------------------------------------------------------------------------------------------------------

[[maybe_unused]] GXVoid GXVec3::Reverse () noexcept
{
    _data[ 0U ] = -_data[ 0U ];
    _data[ 1U ] = -_data[ 1U ];
    _data[ 2U ] = -_data[ 2U ];
}

[[maybe_unused]] GXVoid GXVec3::Sum ( GXVec3 const &a, GXVec3 const &b ) noexcept
{
    _data[ 0U ] = a._data[ 0U ] + b._data[ 0U ];
    _data[ 1U ] = a._data[ 1U ] + b._data[ 1U ];
    _data[ 2U ] = a._data[ 2U ] + b._data[ 2U ];
}

[[maybe_unused]] GXVoid GXVec3::Sum ( GXVec3 const &a, GXFloat bScale, GXVec3 const &b ) noexcept
{
    _data[ 0U ] = a._data[ 0U ] + bScale * b._data[ 0U ];
    _data[ 1U ] = a._data[ 1U ] + bScale * b._data[ 1U ];
    _data[ 2U ] = a._data[ 2U ] + bScale * b._data[ 2U ];
}

[[maybe_unused]] GXVoid GXVec3::Subtract ( GXVec3 const &a, GXVec3 const &b ) noexcept
{
    _data[ 0U ] = a._data[ 0U ] - b._data[ 0U ];
    _data[ 1U ] = a._data[ 1U ] - b._data[ 1U ];
    _data[ 2U ] = a._data[ 2U ] - b._data[ 2U ];
}

[[maybe_unused]] GXVoid GXVec3::Multiply ( GXVec3 const &a, GXFloat scale ) noexcept
{
    _data[ 0U ] = a._data[ 0U ] * scale;
    _data[ 1U ] = a._data[ 1U ] * scale;
    _data[ 2U ] = a._data[ 2U ] * scale;
}

[[maybe_unused]] GXVoid GXVec3::Multiply ( GXVec3 const &a, GXVec3 const &b ) noexcept
{
    _data[ 0U ] = a._data[ 0U ] * b._data[ 0U ];
    _data[ 1U ] = a._data[ 1U ] * b._data[ 1U ];
    _data[ 2U ] = a._data[ 2U ] * b._data[ 2U ];
}

[[maybe_unused]] GXFloat GXVec3::DotProduct ( GXVec3 const &other ) const noexcept
{
    return _data[ 0U ] * other._data[ 0U ] + _data[ 1U ] * other._data[ 1U ] + _data[ 2U ] * other._data[ 2U ];
}

[[maybe_unused]] GXVoid GXVec3::CrossProduct ( GXVec3 const &a, GXVec3 const &b ) noexcept
{
    _data[ 0U ] = a._data[ 1U ] * b._data[ 2U ] - a._data[ 2U ] * b._data[ 1U ];
    _data[ 1U ] = a._data[ 2U ] * b._data[ 0U ] - a._data[ 0U ] * b._data[ 2U ];
    _data[ 2U ] = a._data[ 0U ] * b._data[ 1U ] - a._data[ 1U ] * b._data[ 0U ];
}

//----------------------------------------------------------------------------------------------------------------------

[[maybe_unused]] GXVoid GXVec4::Sum ( GXVec4 const &a, GXVec4 const &b ) noexcept
{
    _data[ 0U ] = a._data[ 0U ] + b._data[ 0U ];
    _data[ 1U ] = a._data[ 1U ] + b._data[ 1U ];
    _data[ 2U ] = a._data[ 2U ] + b._data[ 2U ];
    _data[ 3U ] = a._data[ 3U ] + b._data[ 3U ];
}

[[maybe_unused]] GXVoid GXVec4::Sum ( GXVec4 const &a, GXFloat bScale, GXVec4 const &b ) noexcept
{
    _data[ 0U ] = a._data[ 0U ] + bScale * b._data[ 0U ];
    _data[ 1U ] = a._data[ 1U ] + bScale * b._data[ 1U ];
    _data[ 2U ] = a._data[ 2U ] + bScale * b._data[ 2U ];
    _data[ 3U ] = a._data[ 3U ] + bScale * b._data[ 3U ];
}

[[maybe_unused]] GXVoid GXVec4::Subtract ( GXVec4 const &a, GXVec4 const &b ) noexcept
{
    _data[ 0U ] = a._data[ 0U ] - b._data[ 0U ];
    _data[ 1U ] = a._data[ 1U ] - b._data[ 1U ];
    _data[ 2U ] = a._data[ 2U ] - b._data[ 2U ];
    _data[ 3U ] = a._data[ 3U ] - b._data[ 3U ];
}

[[maybe_unused]] GXFloat GXVec4::DotProduct ( GXVec4 const &other ) const noexcept
{
    return _data[ 0U ] * other._data[ 0U ] +
        _data[ 1U ] * other._data[ 1U ] +
        _data[ 2U ] * other._data[ 2U ] +
        _data[ 3U ] * other._data[ 3U ];
}

//----------------------------------------------------------------------------------------------------------------------

[[maybe_unused]] GXFloat GXVec6::DotProduct ( GXVec6 const &other ) const noexcept
{
    return _data[ 0U ] * other._data[ 0U ] +
        _data[ 1U ] * other._data[ 1U ] +
        _data[ 2U ] * other._data[ 2U ] +
        _data[ 3U ] * other._data[ 3U ] +
        _data[ 4U ] * other._data[ 4U ] +
        _data[ 5U ] * other._data[ 5U ];
}

[[maybe_unused]] GXVoid GXVec6::Sum ( GXVec6 const &a, GXVec6 const &b ) noexcept
{
    _data[ 0U ] = a._data[ 0U ] + b._data[ 0U ];
    _data[ 1U ] = a._data[ 1U ] + b._data[ 1U ];
    _data[ 2U ] = a._data[ 2U ] + b._data[ 2U ];
    _data[ 3U ] = a._data[ 3U ] + b._data[ 3U ];
    _data[ 4U ] = a._data[ 4U ] + b._data[ 4U ];
    _data[ 5U ] = a._data[ 5U ] + b._data[ 5U ];
}

[[maybe_unused]] GXVoid GXVec6::Sum ( GXVec6 const &a, GXFloat bScale, GXVec6 const &b ) noexcept
{
    _data[ 0U ] = a._data[ 0U ] + bScale * b._data[ 0U ];
    _data[ 1U ] = a._data[ 1U ] + bScale * b._data[ 1U ];
    _data[ 2U ] = a._data[ 2U ] + bScale * b._data[ 2U ];
    _data[ 3U ] = a._data[ 3U ] + bScale * b._data[ 3U ];
    _data[ 4U ] = a._data[ 4U ] + bScale * b._data[ 4U ];
    _data[ 5U ] = a._data[ 5U ] + bScale * b._data[ 5U ];
}

[[maybe_unused]] GXVoid GXVec6::Multiply ( GXVec6 const &a, GXFloat factor ) noexcept
{
    _data[ 0U ] = a._data[ 0U ] * factor;
    _data[ 1U ] = a._data[ 1U ] * factor;
    _data[ 2U ] = a._data[ 2U ] * factor;
    _data[ 3U ] = a._data[ 3U ] * factor;
    _data[ 4U ] = a._data[ 4U ] * factor;
    _data[ 5U ] = a._data[ 5U ] * factor;
}

//----------------------------------------------------------------------------------------------------------------------

[[maybe_unused]] GXVoid GXColorRGB::From ( GXUByte red, GXUByte green, GXUByte blue, GXFloat alpha ) noexcept
{
    _data[ 0U ] = static_cast<GXFloat> ( red ) * GX_MATH_UNORM_FACTOR;
    _data[ 1U ] = static_cast<GXFloat> ( green ) * GX_MATH_UNORM_FACTOR;
    _data[ 2U ] = static_cast<GXFloat> ( blue ) * GX_MATH_UNORM_FACTOR;
    _data[ 3U ] = alpha * GX_MATH_UNORM_FACTOR;
}

//----------------------------------------------------------------------------------------------------------------------

[[maybe_unused]] GXVoid GXQuat::Normalize () noexcept
{
    GXFloat const squaredLength = _data[ 0U ] * _data[ 0U ] +
        _data[ 1U ] * _data[ 1U ] +
        _data[ 2U ] * _data[ 2U ] +
        _data[ 3U ] * _data[ 3U ];

    assert ( squaredLength > GX_MATH_FLOAT_EPSILON );

    Multiply ( *this, 1.0F / std::sqrt ( squaredLength ) );
}

[[maybe_unused]] GXVoid GXQuat::Inverse ( GXQuat const &q ) noexcept
{
    GXFloat const squaredLength = q._data[ 0U ] * q._data[ 0U ] +
        q._data[ 1U ] * q._data[ 1U ] +
        q._data[ 2U ] * q._data[ 2U ] +
        q._data[ 3U ] * q._data[ 3U ];

    assert ( squaredLength > GX_MATH_FLOAT_EPSILON );

    GXFloat const inverseSquaredLength = 1.0F / squaredLength;

    _data[ 0U ] = q._data[ 0U ] * inverseSquaredLength;
    _data[ 1U ] = -q._data[ 1U ] * inverseSquaredLength;
    _data[ 2U ] = -q._data[ 2U ] * inverseSquaredLength;
    _data[ 3U ] = -q._data[ 3U ] * inverseSquaredLength;
}

[[maybe_unused]] GXVoid GXQuat::FromAxisAngle ( GXFloat x, GXFloat y, GXFloat z, GXFloat angle ) noexcept
{
    GXFloat const halfAngle = 0.5F * angle;
    GXFloat const sinom = std::sin ( halfAngle );

    _data[ 0U ] = std::cos ( halfAngle );
    _data[ 1U ] = x * sinom;
    _data[ 2U ] = y * sinom;
    _data[ 3U ] = z * sinom;
}

[[maybe_unused]] GXVoid GXQuat::Multiply ( GXQuat const &q, GXFloat scale ) noexcept
{
    _data[ 0U ] = q._data[ 0U ] * scale;
    _data[ 1U ] = q._data[ 1U ] * scale;
    _data[ 2U ] = q._data[ 2U ] * scale;
    _data[ 3U ] = q._data[ 3U ] * scale;
}

[[maybe_unused]] GXVoid GXQuat::Sum ( GXQuat const &a, GXQuat const &b ) noexcept
{
    _data[ 0U ] = a._data[ 0U ] + b._data[ 0U ];
    _data[ 1U ] = a._data[ 1U ] + b._data[ 1U ];
    _data[ 2U ] = a._data[ 2U ] + b._data[ 2U ];
    _data[ 3U ] = a._data[ 3U ] + b._data[ 3U ];
}

[[maybe_unused]] GXVoid GXQuat::Subtract ( GXQuat const &a, GXQuat const &b ) noexcept
{
    _data[ 0U ] = a._data[ 0U ] - b._data[ 0U ];
    _data[ 1U ] = a._data[ 1U ] - b._data[ 1U ];
    _data[ 2U ] = a._data[ 2U ] - b._data[ 2U ];
    _data[ 3U ] = a._data[ 3U ] - b._data[ 3U ];
}

//----------------------------------------------------------------------------------------------------------------------

[[maybe_unused]] GXVoid GXMat3::Multiply ( GXMat3 const &a, GXMat3 const &b ) noexcept
{
    _m[ 0U ][ 0U ] = a._m[ 0U ][ 0U ] * b._m[ 0U ][ 0U ] +
        a._m[ 0U ][ 1U ] * b._m[ 1U ][ 0U ] +
        a._m[ 0U ][ 2U ] * b._m[ 2U ][ 0U ];

    _m[ 0U ][ 1U ] = a._m[ 0U ][ 0U ] * b._m[ 0U ][ 1U ] +
        a._m[ 0U ][ 1U ] * b._m[ 1U ][ 1U ] +
        a._m[ 0U ][ 2U ] * b._m[ 2U ][ 1U ];

    _m[ 0U ][ 2U ] = a._m[ 0U ][ 0U ] * b._m[ 0U ][ 2U ] +
        a._m[ 0U ][ 1U ] * b._m[ 1U ][ 2U ] +
        a._m[ 0U ][ 2U ] * b._m[ 2U ][ 2U ];

    _m[ 1U ][ 0U ] = a._m[ 1U ][ 0U ] * b._m[ 0U ][ 0U ] +
        a._m[ 1U ][ 1U ] * b._m[ 1U ][ 0U ] +
        a._m[ 1U ][ 2U ] * b._m[ 2U ][ 0U ];

    _m[ 1U ][ 1U ] = a._m[ 1U ][ 0U ] * b._m[ 0U ][ 1U ] +
        a._m[ 1U ][ 1U ] * b._m[ 1U ][ 1U ] +
        a._m[ 1U ][ 2U ] * b._m[ 2U ][ 1U ];

    _m[ 1U ][ 2U ] = a._m[ 1U ][ 0U ] * b._m[ 0U ][ 2U ] +
        a._m[ 1U ][ 1U ] * b._m[ 1U ][ 2U ] +
        a._m[ 1U ][ 2U ] * b._m[ 2U ][ 2U ];

    _m[ 2U ][ 0U ] = a._m[ 2U ][ 0U ] * b._m[ 0U ][ 0U ] +
        a._m[ 2U ][ 1U ] * b._m[ 1U ][ 0U ] +
        a._m[ 2U ][ 2U ] * b._m[ 2U ][ 0U ];

    _m[ 2U ][ 1U ] = a._m[ 2U ][ 0U ] * b._m[ 0U ][ 1U ] +
        a._m[ 2U ][ 1U ] * b._m[ 1U ][ 1U ] +
        a._m[ 2U ][ 2U ] * b._m[ 2U ][ 1U ];

    _m[ 2U ][ 2U ] = a._m[ 2U ][ 0U ] * b._m[ 0U ][ 2U ] +
        a._m[ 2U ][ 1U ] * b._m[ 1U ][ 2U ] +
        a._m[ 2U ][ 2U ] * b._m[ 2U ][ 2U ];
}

[[maybe_unused]] GXVoid GXMat3::MultiplyVectorMatrix ( GXVec3 &out, GXVec3 const &v ) const noexcept
{
    out._data[ 0U ] = v._data[ 0U ] * _m[ 0U ][ 0U ] + v._data[ 1U ] * _m[ 1U ][ 0U ] + v._data[ 2U ] * _m[ 2U ][ 0U ];
    out._data[ 1U ] = v._data[ 0U ] * _m[ 0U ][ 1U ] + v._data[ 1U ] * _m[ 1U ][ 1U ] + v._data[ 2U ] * _m[ 2U ][ 1U ];
    out._data[ 2U ] = v._data[ 0U ] * _m[ 0U ][ 2U ] + v._data[ 1U ] * _m[ 1U ][ 2U ] + v._data[ 2U ] * _m[ 2U ][ 2U ];
}

[[maybe_unused]] GXVoid GXMat3::MultiplyMatrixVector ( GXVec3 &out, GXVec3 const &v ) const noexcept
{
    out._data[ 0U ] = _m[ 0U ][ 0U ] * v._data[ 0U ] + _m[ 0U ][ 1U ] * v._data[ 1U ] + _m[ 0U ][ 2U ] * v._data[ 2U ];
    out._data[ 1U ] = _m[ 1U ][ 0U ] * v._data[ 0U ] + _m[ 1U ][ 1U ] * v._data[ 1U ] + _m[ 1U ][ 2U ] * v._data[ 2U ];
    out._data[ 2U ] = _m[ 2U ][ 0U ] * v._data[ 0U ] + _m[ 2U ][ 1U ] * v._data[ 1U ] + _m[ 2U ][ 2U ] * v._data[ 2U ];
}

//----------------------------------------------------------------------------------------------------------------------

[[maybe_unused]] GXVoid GXMat4::Multiply ( GXMat4 const &a, GXMat4 const &b ) noexcept
{
    _m[ 0U ][ 0U ] = a._m[ 0U ][ 0U ] * b._m[ 0U ][ 0U ] +
        a._m[ 0U ][ 1U ] * b._m[ 1U ][ 0U ] +
        a._m[ 0U ][ 2U ] * b._m[ 2U ][ 0U ] +
        a._m[ 0U ][ 3U ] * b._m[ 3U ][ 0U ];

    _m[ 0U ][ 1U ] = a._m[ 0U ][ 0U ] * b._m[ 0U ][ 1U ] +
        a._m[ 0U ][ 1U ] * b._m[ 1U ][ 1U ] +
        a._m[ 0U ][ 2U ] * b._m[ 2U ][ 1U ] +
        a._m[ 0U ][ 3U ] * b._m[ 3U ][ 1U ];

    _m[ 0U ][ 2U ] = a._m[ 0U ][ 0U ] * b._m[ 0U ][ 2U ] +
        a._m[ 0U ][ 1U ] * b._m[ 1U ][ 2U ] +
        a._m[ 0U ][ 2U ] * b._m[ 2U ][ 2U ] +
        a._m[ 0U ][ 3U ] * b._m[ 3U ][ 2U ];

    _m[ 0U ][ 3U ] = a._m[ 0U ][ 0U ] * b._m[ 0U ][ 3U ] +
        a._m[ 0U ][ 1U ] * b._m[ 1U ][ 3U ] +
        a._m[ 0U ][ 2U ] * b._m[ 2U ][ 3U ] +
        a._m[ 0U ][ 3U ] * b._m[ 3U ][ 3U ];

    _m[ 1U ][ 0U ] = a._m[ 1U ][ 0U ] * b._m[ 0U ][ 0U ] +
        a._m[ 1U ][ 1U ] * b._m[ 1U ][ 0U ] +
        a._m[ 1U ][ 2U ] * b._m[ 2U ][ 0U ] +
        a._m[ 1U ][ 3U ] * b._m[ 3U ][ 0U ];

    _m[ 1U ][ 1U ] = a._m[ 1U ][ 0U ] * b._m[ 0U ][ 1U ] +
        a._m[ 1U ][ 1U ] * b._m[ 1U ][ 1U ] +
        a._m[ 1U ][ 2U ] * b._m[ 2U ][ 1U ] +
        a._m[ 1U ][ 3U ] * b._m[ 3U ][ 1U ];

    _m[ 1U ][ 2U ] = a._m[ 1U ][ 0U ] * b._m[ 0U ][ 2U ] +
        a._m[ 1U ][ 1U ] * b._m[ 1U ][ 2U ] +
        a._m[ 1U ][ 2U ] * b._m[ 2U ][ 2U ] +
        a._m[ 1U ][ 3U ] * b._m[ 3U ][ 2U ];

    _m[ 1U ][ 3U ] = a._m[ 1U ][ 0U ] * b._m[ 0U ][ 3U ] +
        a._m[ 1U ][ 1U ] * b._m[ 1U ][ 3U ] +
        a._m[ 1U ][ 2U ] * b._m[ 2U ][ 3U ] +
        a._m[ 1U ][ 3U ] * b._m[ 3U ][ 3U ];

    _m[ 2U ][ 0U ] = a._m[ 2U ][ 0U ] * b._m[ 0U ][ 0U ] +
        a._m[ 2U ][ 1U ] * b._m[ 1U ][ 0U ] +
        a._m[ 2U ][ 2U ] * b._m[ 2U ][ 0U ] +
        a._m[ 2U ][ 3U ] * b._m[ 3U ][ 0U ];

    _m[ 2U ][ 1U ] = a._m[ 2U ][ 0U ] * b._m[ 0U ][ 1U ] +
        a._m[ 2U ][ 1U ] * b._m[ 1U ][ 1U ] +
        a._m[ 2U ][ 2U ] * b._m[ 2U ][ 1U ] +
        a._m[ 2U ][ 3U ] * b._m[ 3U ][ 1U ];

    _m[ 2U ][ 2U ] = a._m[ 2U ][ 0U ] * b._m[ 0U ][ 2U ] +
        a._m[ 2U ][ 1U ] * b._m[ 1U ][ 2U ] +
        a._m[ 2U ][ 2U ] * b._m[ 2U ][ 2U ] +
        a._m[ 2U ][ 3U ] * b._m[ 3U ][ 2U ];

    _m[ 2U ][ 3U ] = a._m[ 2U ][ 0U ] * b._m[ 0U ][ 3U ] +
        a._m[ 2U ][ 1U ] * b._m[ 1U ][ 3U ] +
        a._m[ 2U ][ 2U ] * b._m[ 2U ][ 3U ] +
        a._m[ 2U ][ 3U ] * b._m[ 3U ][ 3U ];

    _m[ 3U ][ 0U ] = a._m[ 3U ][ 0U ] * b._m[ 0U ][ 0U ] +
        a._m[ 3U ][ 1U ] * b._m[ 1U ][ 0U ] +
        a._m[ 3U ][ 2U ] * b._m[ 2U ][ 0U ] +
        a._m[ 3U ][ 3U ] * b._m[ 3U ][ 0U ];

    _m[ 3U ][ 1U ] = a._m[ 3U ][ 0U ] * b._m[ 0U ][ 1U ] +
        a._m[ 3U ][ 1U ] * b._m[ 1U ][ 1U ] +
        a._m[ 3U ][ 2U ] * b._m[ 2U ][ 1U ] +
        a._m[ 3U ][ 3U ] * b._m[ 3U ][ 1U ];

    _m[ 3U ][ 2U ] = a._m[ 3U ][ 0U ] * b._m[ 0U ][ 2U ] +
        a._m[ 3U ][ 1U ] * b._m[ 1U ][ 2U ] +
        a._m[ 3U ][ 2U ] * b._m[ 2U ][ 2U ] +
        a._m[ 3U ][ 3U ] * b._m[ 3U ][ 2U ];

    _m[ 3U ][ 3U ] = a._m[ 3U ][ 0U ] * b._m[ 0U ][ 3U ] +
        a._m[ 3U ][ 1U ] * b._m[ 1U ][ 3U ] +
        a._m[ 3U ][ 2U ] * b._m[ 2U ][ 3U ] + a._m[ 3U ][ 3U ] * b._m[ 3U ][ 3U ];
}

[[maybe_unused]] GXVoid GXMat4::MultiplyVectorMatrix ( GXVec4 &out, GXVec4 const &v ) const noexcept
{
    out._data[ 0U ] = v._data[ 0U ] * _m[ 0U ][ 0U ] +
        v._data[ 1U ] * _m[ 1U ][ 0U ] +
        v._data[ 2U ] * _m[ 2U ][ 0U ] +
        v._data[ 3U ] * _m[ 3U ][ 0U ];

    out._data[ 1U ] = v._data[ 0U ] * _m[ 0U ][ 1U ] +
        v._data[ 1U ] * _m[ 1U ][ 1U ] +
        v._data[ 2U ] * _m[ 2U ][ 1U ] +
        v._data[ 3U ] * _m[ 3U ][ 1U ];

    out._data[ 2U ] = v._data[ 0U ] * _m[ 0U ][ 2U ] +
        v._data[ 1U ] * _m[ 1U ][ 2U ] +
        v._data[ 2U ] * _m[ 2U ][ 2U ] +
        v._data[ 3U ] * _m[ 3U ][ 2U ];

    out._data[ 3U ] = v._data[ 0U ] * _m[ 0U ][ 3U ] +
        v._data[ 1U ] * _m[ 1U ][ 3U ] +
        v._data[ 2U ] * _m[ 2U ][ 3U ] +
        v._data[ 3U ] * _m[ 3U ][ 3U ];
}

[[maybe_unused]] GXVoid GXMat4::MultiplyMatrixVector ( GXVec4 &out, GXVec4 const &v ) const noexcept
{
    out._data[ 0U ] = _m[ 0U ][ 0U ] * v._data[ 0U ] +
        _m[ 0U ][ 1U ] * v._data[ 1U ] +
        _m[ 0U ][ 2U ] * v._data[ 2U ] +
        _m[ 0U ][ 3U ] * v._data[ 3U ];

    out._data[ 1U ] = _m[ 1U ][ 0U ] * v._data[ 0U ] +
        _m[ 1U ][ 1U ] * v._data[ 1U ] +
        _m[ 1U ][ 2U ] * v._data[ 2U ] +
        _m[ 1U ][ 3U ] * v._data[ 3U ];

    out._data[ 2U ] = _m[ 2U ][ 0U ] * v._data[ 0U ] +
        _m[ 2U ][ 1U ] * v._data[ 1U ] +
        _m[ 2U ][ 2U ] * v._data[ 2U ] +
        _m[ 2U ][ 3U ] * v._data[ 3U ];

    out._data[ 3U ] = _m[ 3U ][ 0U ] * v._data[ 0U ] +
        _m[ 3U ][ 1U ] * v._data[ 1U ] +
        _m[ 3U ][ 2U ] * v._data[ 2U ] +
        _m[ 3U ][ 3U ] * v._data[ 3U ];
}

[[maybe_unused]] GXVoid GXMat4::MultiplyAsNormal ( GXVec3 &out, GXVec3 const &v ) const noexcept
{
    out._data[ 0U ] = v._data[ 0U ] * _m[ 0U ][ 0U ] + v._data[ 1U ] * _m[ 1U ][ 0U ] + v._data[ 2U ] * _m[ 2U ][ 0U ];
    out._data[ 1U ] = v._data[ 0U ] * _m[ 0U ][ 1U ] + v._data[ 1U ] * _m[ 1U ][ 1U ] + v._data[ 2U ] * _m[ 2U ][ 1U ];
    out._data[ 2U ] = v._data[ 0U ] * _m[ 0U ][ 2U ] + v._data[ 1U ] * _m[ 1U ][ 2U ] + v._data[ 2U ] * _m[ 2U ][ 2U ];
}

[[maybe_unused]] GXVoid GXMat4::MultiplyAsPoint ( GXVec3 &out, GXVec3 const &v ) const noexcept
{
    out._data[ 0U ] = v._data[ 0U ] * _m[ 0U ][ 0U ] +
        v._data[ 1U ] * _m[ 1U ][ 0U ] +
        v._data[ 2U ] * _m[ 2U ][ 0U ] +
        _m[ 3U ][ 0U ];

    out._data[ 1U ] = v._data[ 0U ] * _m[ 0U ][ 1U ] +
        v._data[ 1U ] * _m[ 1U ][ 1U ] +
        v._data[ 2U ] * _m[ 2U ][ 1U ] +
        _m[ 3U ][ 1U ];

    out._data[ 2U ] = v._data[ 0U ] * _m[ 0U ][ 2U ] +
        v._data[ 1U ] * _m[ 1U ][ 2U ] +
        v._data[ 2U ] * _m[ 2U ][ 2U ] +
        _m[ 3U ][ 2U ];
}
