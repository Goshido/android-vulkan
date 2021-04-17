// version 1.1

#include <GXCommon/GXMath.h>


[[maybe_unused]] GXVoid GXVec2::CalculateNormalFast ( GXVec2 const &a, GXVec2 const &b )
{
    _data[ 0U ] = a._data[ 1U ] - b._data[ 1U ];
    _data[ 1U ] = b._data[ 0U ] - a._data[ 0U ];
}

[[maybe_unused]] GXFloat GXVec2::DotProduct ( GXVec2 const &other ) const
{
    return _data[ 0U ] * other._data[ 0U ] + _data[ 1U ] * other._data[ 1U ];
}

[[maybe_unused]] GXVoid GXVec2::Sum ( GXVec2 const &a, GXVec2 const &b )
{
    _data[ 0U ] = a._data[ 0U ] + b._data[ 0U ];
    _data[ 1U ] = a._data[ 1U ] + b._data[ 1U ];
}

[[maybe_unused]] GXVoid GXVec2::Sum ( GXVec2 const &a, GXFloat bScale, GXVec2 const &b )
{
    _data[ 0U ] = a._data[ 0U ] + bScale * b._data[ 0U ];
    _data[ 1U ] = a._data[ 1U ] + bScale * b._data[ 1U ];
}

[[maybe_unused]] GXVoid GXVec2::Subtract ( GXVec2 const &a, GXVec2 const &b )
{
    _data[ 0U ] = a._data[ 0U ] - b._data[ 0U ];
    _data[ 1U ] = a._data[ 1U ] - b._data[ 1U ];
}

[[maybe_unused]] GXVoid GXVec2::Multiply ( GXVec2 const &a, GXVec2 const &b )
{
    _data[ 0U ] = a._data[ 0U ] * b._data[ 0U ];
    _data[ 1U ] = a._data[ 1U ] * b._data[ 1U ];
}

[[maybe_unused]] GXVoid GXVec2::Multiply ( GXVec2 const &v, GXFloat scale )
{
    _data[ 0U ] = v._data[ 0U ] * scale;
    _data[ 1U ] = v._data[ 1U ] * scale;
}

//----------------------------------------------------------------------------------------------------------------------

[[maybe_unused]] GXVoid GXVec3::Reverse ()
{
    _data[ 0U ] = -_data[ 0U ];
    _data[ 1U ] = -_data[ 1U ];
    _data[ 2U ] = -_data[ 2U ];
}

[[maybe_unused]] GXVoid GXVec3::Sum ( GXVec3 const &a, GXVec3 const &b )
{
    _data[ 0U ] = a._data[ 0U ] + b._data[ 0U ];
    _data[ 1U ] = a._data[ 1U ] + b._data[ 1U ];
    _data[ 2U ] = a._data[ 2U ] + b._data[ 2U ];
}

[[maybe_unused]] GXVoid GXVec3::Sum ( GXVec3 const &a, GXFloat bScale, GXVec3 const &b )
{
    _data[ 0U ] = a._data[ 0U ] + bScale * b._data[ 0U ];
    _data[ 1U ] = a._data[ 1U ] + bScale * b._data[ 1U ];
    _data[ 2U ] = a._data[ 2U ] + bScale * b._data[ 2U ];
}

[[maybe_unused]] GXVoid GXVec3::Subtract ( GXVec3 const &a, GXVec3 const &b )
{
    _data[ 0U ] = a._data[ 0U ] - b._data[ 0U ];
    _data[ 1U ] = a._data[ 1U ] - b._data[ 1U ];
    _data[ 2U ] = a._data[ 2U ] - b._data[ 2U ];
}

[[maybe_unused]] GXVoid GXVec3::Multiply ( GXVec3 const &a, GXFloat scale )
{
    _data[ 0U ] = a._data[ 0U ] * scale;
    _data[ 1U ] = a._data[ 1U ] * scale;
    _data[ 2U ] = a._data[ 2U ] * scale;
}

[[maybe_unused]] GXVoid GXVec3::Multiply ( GXVec3 const &a, GXVec3 const &b )
{
    _data[ 0U ] = a._data[ 0U ] * b._data[ 0U ];
    _data[ 1U ] = a._data[ 1U ] * b._data[ 1U ];
    _data[ 2U ] = a._data[ 2U ] * b._data[ 2U ];
}

[[maybe_unused]] GXFloat GXVec3::DotProduct ( GXVec3 const &other ) const
{
    return _data[ 0U ] * other._data[ 0U ] + _data[ 1U ] * other._data[ 1U ] + _data[ 2U ] * other._data[ 2U ];
}

[[maybe_unused]] GXVoid GXVec3::CrossProduct ( GXVec3 const &a, GXVec3 const &b )
{
    _data[ 0U ] = a._data[ 1U ] * b._data[ 2U ] - a._data[ 2U ] * b._data[ 1U ];
    _data[ 1U ] = a._data[ 2U ] * b._data[ 0U ] - a._data[ 0U ] * b._data[ 2U ];
    _data[ 2U ] = a._data[ 0U ] * b._data[ 1U ] - a._data[ 1U ] * b._data[ 0U ];
}

//----------------------------------------------------------------------------------------------------------------------

[[maybe_unused]] GXVoid GXMat4::Multiply ( GXMat4 const &a, GXMat4 const &b )
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
