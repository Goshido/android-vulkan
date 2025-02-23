// version 1.94

#include <precompiled_headers.hpp>
#include <GXCommon/GXMath.hpp>


namespace {

constexpr GXFloat HSVA_FACTOR = 0.016666F;
constexpr GXFloat HSVA_TO_RGBA_FLOAT = 0.01F;
constexpr GXFloat RGBA_TO_UBYTE_FACTOR = 255.0F;

constexpr GXFloat DEGREES_TO_RADIANS_FACTOR = 0.0174533F;
constexpr GXFloat RADIANS_TO_DEGREES_FACTOR = 57.295779F;

constexpr GXFloat INVERSE_RAND_MAX = 3.05185e-5F;

constexpr GXUByte SOLUTION_ALPHA = 0U;
constexpr GXUByte SOLUTION_BETTA = 1U;
constexpr GXUByte SOLUTION_GAMMA = 2U;
constexpr GXUByte SOLUTION_YOTTA = 3U;

} // end of anonymous namespace

//----------------------------------------------------------------------------------------------------------------------

[[maybe_unused]] GXVoid GXVec2::SetX ( GXFloat x ) noexcept
{
    _data[ 0U ] = x;
}

[[maybe_unused]] GXFloat GXVec2::GetX () const noexcept
{
    return _data[ 0U ];
}

[[maybe_unused]] GXVoid GXVec2::SetY ( GXFloat y ) noexcept
{
    _data[ 1U ] = y;
}

[[maybe_unused]] GXFloat GXVec2::GetY () const noexcept
{
    return _data[ 1U ];
}

[[maybe_unused]] GXVoid GXVec2::Init ( GXFloat x, GXFloat y ) noexcept
{
    auto &d = _data;
    d[ 0U ] = x;
    d[ 1U ] = y;
}

[[maybe_unused]] GXVoid GXVec2::Normalize () noexcept
{
    Multiply ( *this, 1.0F / Length () );
}

[[maybe_unused]] GXVoid GXVec2::CalculateNormal ( GXVec2 const &a, GXVec2 const &b ) noexcept
{
    CalculateNormalFast ( a, b );
    Normalize ();
}

[[maybe_unused]] GXFloat GXVec2::Length () const noexcept
{
    return std::sqrt ( SquaredLength () );
}

[[maybe_unused]] GXFloat GXVec2::SquaredLength () const noexcept
{
    return DotProduct ( *this );
}

[[maybe_unused]] GXBool GXVec2::IsEqual ( GXVec2 const &other ) const noexcept
{
    auto const &d = _data;
    auto const &otherData = other._data;
    return ( d[ 0U ] == otherData[ 0U ] ) & ( d[ 1U ] == otherData[ 1U ] );
}

//----------------------------------------------------------------------------------------------------------------------

[[maybe_unused]] eGXLineRelationship GXCALL GXLineIntersection2D ( GXVec2 &intersectionPoint,
    const GXVec2 &a0,
    const GXVec2 &a1,
    const GXVec2 &b0,
    const GXVec2 &b1
) noexcept
{
    GXVec2 alpha {};
    alpha.Subtract ( a1, a0 );

    GXVec2 betta {};
    betta.Subtract ( b1, b0 );

    GXVec2 const yotta ( -alpha.GetY (), alpha.GetX () );
    GXFloat const omega = yotta.DotProduct ( betta );

    if ( omega == 0.0F )
    {
        GXVec2 const gamma ( a0.IsEqual ( b0 ) ? b1 : b0 );

        GXVec2 zetta {};
        zetta.Subtract ( gamma, a0 );
        zetta.Normalize ();

        alpha.Normalize ();

        GXFloat const eta = alpha.DotProduct ( zetta );

        if ( eta == 1.0F || eta == -1.0F )
        {
            intersectionPoint = a0;
            return eGXLineRelationship::Overlap;
        }

        intersectionPoint.Init ( FLT_MAX, FLT_MAX );
        return eGXLineRelationship::NoIntersection;
    }

    GXVec2 phi {};
    phi.Subtract ( a0, b0 );

    intersectionPoint.Sum ( b0, phi.DotProduct ( yotta ) / omega, betta );
    return eGXLineRelationship::Intersection;
}

//----------------------------------------------------------------------------------------------------------------------

[[maybe_unused]] GXVoid GXVec3::SetX ( GXFloat x ) noexcept
{
    _data[ 0U ] = x;
}

[[maybe_unused]] GXFloat GXVec3::GetX () const noexcept
{
    return _data[ 0U ];
}

[[maybe_unused]] GXVoid GXVec3::SetY ( GXFloat y ) noexcept
{
    _data[ 1U ] = y;
}

[[maybe_unused]] GXFloat GXVec3::GetY () const noexcept
{
    return _data[ 1U ];
}

[[maybe_unused]] GXVoid GXVec3::SetZ ( GXFloat z ) noexcept
{
    _data[ 2U ] = z;
}

[[maybe_unused]] GXFloat GXVec3::GetZ () const noexcept
{
    return _data[ 2U ];
}

[[maybe_unused]] GXVoid GXVec3::Init ( GXFloat x, GXFloat y, GXFloat z ) noexcept
{
    auto &d = _data;
    d[ 0U ] = x;
    d[ 1U ] = y;
    d[ 2U ] = z;
}

[[maybe_unused]] GXVoid GXVec3::Normalize () noexcept
{
    Multiply ( *this, 1.0F / Length () );
}

[[maybe_unused]] GXFloat GXVec3::Length () const noexcept
{
    return std::sqrt ( DotProduct ( *this ) );
}

[[maybe_unused]] GXFloat GXVec3::SquaredLength () const noexcept
{
    return DotProduct ( *this );
}

[[maybe_unused]] GXFloat GXVec3::Distance ( GXVec3 const &other ) const noexcept
{
    GXVec3 difference {};
    difference.Subtract ( *this, other );
    return difference.Length ();
}

[[maybe_unused]] GXFloat GXVec3::SquaredDistance ( GXVec3 const &other ) const noexcept
{
    GXVec3 difference {};
    difference.Subtract ( *this, other );
    return difference.SquaredLength ();
}


[[maybe_unused]] GXVoid GXVec3::Project ( GXVec3 const &vector, GXVec3 const &axis ) noexcept
{
    GXVec3 normalVector ( vector );
    normalVector.Normalize ();
    Multiply ( axis, vector.Length () * axis.DotProduct ( normalVector ) );
}

[[maybe_unused]] GXBool GXVec3::IsEqual ( GXVec3 const &other ) noexcept
{
    auto const &d = _data;
    auto const &otherData = other._data;
    return ( d[ 0U ] == otherData[ 0U ] ) & ( d[ 1U ] == otherData[ 1U ] ) & ( d[ 2U ] == otherData[ 2U ] );
}

[[maybe_unused]] GXVec3 const &GXVec3::GetAbsoluteX () noexcept
{
    constexpr static GXVec3 absoluteX ( 1.0F, 0.0F, 0.0F );
    return absoluteX;
}

[[maybe_unused]] GXVec3 const &GXVec3::GetAbsoluteY () noexcept
{
    constexpr static GXVec3 absoluteY ( 0.0F, 1.0F, 0.0F );
    return absoluteY;
}

[[maybe_unused]] GXVec3 const &GXVec3::GetAbsoluteZ () noexcept
{
    constexpr static GXVec3 absoluteZ ( 0.0F, 0.0F, 1.0F );
    return absoluteZ;
}

[[maybe_unused]] GXVoid GXCALL GXVec3::MakeOrthonormalBasis ( GXVec3 &baseX,
    GXVec3 &adjustedY,
    GXVec3 &adjustedZ
) noexcept
{
    adjustedZ.CrossProduct ( baseX, adjustedY );

    if ( adjustedZ.SquaredLength () == 0.0F )
    {
        assert ( !"GXVec3::MakeOrthonormalBasis - Unexpected value." );
        return;
    }

    adjustedY.CrossProduct ( adjustedZ, baseX );

    baseX.Normalize ();
    adjustedY.Normalize ();
    adjustedZ.Normalize ();
}

//----------------------------------------------------------------------------------------------------------------------

[[maybe_unused]] GXBool GXCALL GXRayTriangleIntersection3D ( GXFloat &outT,
    GXVec3 const &origin,
    GXVec3 const &direction,
    GXFloat length,
    GXVec3 const &a,
    GXVec3 const &b,
    GXVec3 const &c
) noexcept
{
    // Implementation https://graphics.stanford.edu/courses/cs348b-98/gg/intersect.html

    GXPlane plane {};
    plane.From ( a, b, c );
    GXVec3 const normal ( plane._a, plane._b, plane._c );

    GXFloat const t = ( plane._d + normal.DotProduct ( origin ) ) / ( -normal.DotProduct ( direction ) );

    if ( t < 0.0F )
        return GX_FALSE;

    if ( t > length )
        return GX_FALSE;

    GXVec3 const triangle[ 3U ] = { a, b, c };

    GXVec3 point {};
    point.Sum ( origin, t, direction );

    GXUByte selector;

    GXFloat gamma = std::abs ( plane._a );
    GXFloat const omega = std::abs ( plane._b );
    GXFloat const yotta = std::abs ( plane._c );

    if ( gamma > omega )
        selector = gamma > yotta ? static_cast<GXUByte> ( 0U ) : static_cast<GXUByte> ( 2U );
    else
        selector = omega > yotta ? static_cast<GXUByte> ( 1U ) : static_cast<GXUByte> ( 2U );

    GXUByte i1 = 0xFFU;
    GXUByte i2 = 0xFFU;

    switch ( selector )
    {
        case 0U:
            i1 = 1u;
            i2 = 2u;
        break;

        case 1U:
            i1 = 2u;
            i2 = 0u;
        break;

        case 2U:
            i1 = 0u;
            i2 = 1u;
        break;

        default:
            // NOTHING
        break;
    }

    GXFloat const u0 = point._data[ i1 ] - triangle[ 0U ]._data[ i1 ];
    GXFloat const v0 = point._data[ i2 ] - triangle[ 0U ]._data[ i2 ];

    GXFloat const u1 = triangle[ 1U ]._data[ i1 ] - triangle[ 0U ]._data[ i1 ];
    GXFloat const v1 = triangle[ 1U ]._data[ i2 ] - triangle[ 0U ]._data[ i2 ];

    GXFloat const u2 = triangle[ 2U ]._data[ i1 ] - triangle[ 0U ]._data[ i1 ];
    GXFloat const v2 = triangle[ 2U ]._data[ i2 ] - triangle[ 0U ]._data[ i2 ];

    gamma = 1.0F / ( u1 * v2 - v1 * u2 );
    GXFloat const alpha = ( u0 * v2 - v0 * u2 ) * gamma;

    if ( alpha < 0.0F || alpha > 1.0F )
        return GX_FALSE;

    GXFloat const betta = ( u1 * v0 - v1 * u0 ) * gamma;

    if ( betta < 0.0F || betta > 1.0F || ( alpha + betta ) > 1.0F )
        return GX_FALSE;

    outT = t;
    return GX_TRUE;
}

//----------------------------------------------------------------------------------------------------------------------

[[maybe_unused]] GXVoid GXVec4::Init ( GXFloat x, GXFloat y, GXFloat z, GXFloat w ) noexcept
{
    auto &d = _data;
    d[ 0U ] = x;
    d[ 1U ] = y;
    d[ 2U ] = z;
    d[ 3U ] = w;
}

[[maybe_unused]] GXVoid GXVec4::SetX ( GXFloat x ) noexcept
{
    _data[ 0U ] = x;
}

[[maybe_unused]] GXFloat GXVec4::GetX () const noexcept
{
    return _data[ 0U ];
}

[[maybe_unused]] GXVoid GXVec4::SetY ( GXFloat y ) noexcept
{
    _data[ 1U ] = y;
}

[[maybe_unused]] GXFloat GXVec4::GetY () const noexcept
{
    return _data[ 1U ];
}

[[maybe_unused]] GXVoid GXVec4::SetZ ( GXFloat z ) noexcept
{
    _data[ 2U ] = z;
}

[[maybe_unused]] GXFloat GXVec4::GetZ () const noexcept
{
    return _data[ 2U ];
}

[[maybe_unused]] GXVoid GXVec4::SetW ( GXFloat w ) noexcept
{
    _data[ 3U ] = w;
}

[[maybe_unused]] GXFloat GXVec4::GetW () const noexcept
{
    return _data[ 3U ];
}

[[maybe_unused]] GXFloat GXVec4::Length () const noexcept
{
    return std::sqrt ( DotProduct ( *this ) );
}

[[maybe_unused]] GXFloat GXVec4::SquaredLength () const noexcept
{
    return DotProduct ( *this );
}

//----------------------------------------------------------------------------------------------------------------------

[[maybe_unused]] GXVoid GXVec6::Init ( GXFloat a1, GXFloat a2, GXFloat a3, GXFloat a4, GXFloat a5, GXFloat a6 ) noexcept
{
    auto &d = _data;
    d[ 0U ] = a1;
    d[ 1U ] = a2;
    d[ 2U ] = a3;
    d[ 3U ] = a4;
    d[ 4U ] = a5;
    d[ 5U ] = a6;
}

[[maybe_unused]] GXVoid GXVec6::From ( GXVec3 const &v1, GXVec3 const &v2 ) noexcept
{
    auto &d = _data;
    std::memcpy ( d, &v1, sizeof ( GXVec3 ) );
    std::memcpy ( d + 3U, &v2, sizeof ( GXVec3 ) );
}

//----------------------------------------------------------------------------------------------------------------------

// NOLINTNEXTLINE - does not initialize "_data"
[[maybe_unused]] GXColorRGB::GXColorRGB ( GXUByte red, GXUByte green, GXUByte blue, GXFloat alpha ) noexcept
{
    From ( red, green, blue, alpha );
}

// NOLINTNEXTLINE - does not initialize "_data"
[[maybe_unused]] GXColorRGB::GXColorRGB ( GXColorHSV const &color ) noexcept
{
    From ( color );
}

[[maybe_unused]] GXVoid GXColorRGB::Init ( GXFloat red, GXFloat green, GXFloat blue, GXFloat alpha ) noexcept
{
    auto &d = _data;
    d[ 0U ] = red;
    d[ 1U ] = green;
    d[ 2U ] = blue;
    d[ 3U ] = alpha;
}

[[maybe_unused]] GXVoid GXColorRGB::SetRed ( GXFloat red ) noexcept
{
    _data[ 0U ] = red;
}

[[maybe_unused]] GXFloat GXColorRGB::GetRed () const noexcept
{
    return _data[ 0U ];
}

[[maybe_unused]] GXVoid GXColorRGB::SetGreen ( GXFloat green ) noexcept
{
    _data[ 1U ] = green;
}

[[maybe_unused]] GXFloat GXColorRGB::GetGreen () const noexcept
{
    return _data[ 1U ];
}

[[maybe_unused]] GXVoid GXColorRGB::SetBlue ( GXFloat blue ) noexcept
{
    _data[ 2U ] = blue;
}

[[maybe_unused]] GXFloat GXColorRGB::GetBlue () const noexcept
{
    return _data[ 2U ];
}

[[maybe_unused]] GXVoid GXColorRGB::SetAlpha ( GXFloat alpha ) noexcept
{
    _data[ 3U ] = alpha;
}

[[maybe_unused]] GXFloat GXColorRGB::GetAlpha () const noexcept
{
    return _data[ 3U ];
}

[[maybe_unused]] GXVoid GXColorRGB::From ( GXColorHSV const &color ) noexcept
{
    GXFloat correctedHue = color.GetHue ();

    while ( correctedHue >= 360.0F )
        correctedHue -= 360.0F;

    while ( correctedHue < 0.0F )
        correctedHue += 360.0F;

    GXFloat const value = color.GetValue ();

    auto const selector = static_cast<GXUByte const> ( ( static_cast<GXInt> ( correctedHue ) / 60 ) % 6 );
    GXFloat const minValue = ( ( 100.0F - color.GetSaturation () ) * value ) * 0.01F;

    GXFloat const delta = ( value - minValue ) *
        ( static_cast<GXFloat> ( static_cast<GXInt> ( correctedHue ) % 60 ) * HSVA_FACTOR );

    GXFloat const increment = minValue + delta;
    GXFloat const decrement = value - delta;

    auto &d = _data;
    d[ 3 ] = color.GetAlpha () * HSVA_TO_RGBA_FLOAT;

    switch ( selector )
    {
        case 0U:
            d[ 0U ] = value * HSVA_TO_RGBA_FLOAT;
            d[ 1U ] = increment * HSVA_TO_RGBA_FLOAT;
            d[ 2U ] = minValue * HSVA_TO_RGBA_FLOAT;
        break;

        case 1U:
            d[ 0U ] = decrement * HSVA_TO_RGBA_FLOAT;
            d[ 1U ] = value * HSVA_TO_RGBA_FLOAT;
            d[ 2U ] = minValue * HSVA_TO_RGBA_FLOAT;
        break;

        case 2U:
            d[ 0U ] = minValue * HSVA_TO_RGBA_FLOAT;
            d[ 1U ] = value * HSVA_TO_RGBA_FLOAT;
            d[ 2U ] = increment * HSVA_TO_RGBA_FLOAT;
        break;

        case 3U:
            d[ 0U ] = minValue * HSVA_TO_RGBA_FLOAT;
            d[ 1U ] = decrement * HSVA_TO_RGBA_FLOAT;
            d[ 2U ] = value * HSVA_TO_RGBA_FLOAT;
        break;

        case 4U:
            d[ 0U ] = increment * HSVA_TO_RGBA_FLOAT;
            d[ 1U ] = minValue * HSVA_TO_RGBA_FLOAT;
            d[ 2U ] = value * HSVA_TO_RGBA_FLOAT;
        break;

        case 5U:
            d[ 0U ] = value * HSVA_TO_RGBA_FLOAT;
            d[ 1U ] = minValue * HSVA_TO_RGBA_FLOAT;
            d[ 2U ] = decrement * HSVA_TO_RGBA_FLOAT;
        break;

        default:
            // IMPOSSIBLE
        break;
    }
}

[[maybe_unused]] GXColorRGB GXColorRGB::ToLinearSpace () const noexcept
{
    // See <repo>/docs/srgb.md#srgb-to-linear
    float const* c = _data;
    GXVec3 const &srgb = *reinterpret_cast<GXVec3 const*> ( this );

    GXVec3 lin {};
    lin.Multiply ( srgb, 7.74e-2F );

    constexpr float alpha = 5.213e-2F;
    constexpr GXVec3 beta ( alpha, alpha, alpha );
    GXVec3 omega {};
    omega.Sum ( beta, 9.479e-1F, srgb );

    float const r[] = { lin._data[ 0U ], std::pow ( omega._data[ 0U ], 2.4F ) };
    float const g[] = { lin._data[ 1U ], std::pow ( omega._data[ 1U ], 2.4F ) };

    constexpr float cutoff = 4.045e-2F;
    GXColorRGB result ( r[ static_cast<size_t> ( c[ 0U ] > cutoff ) ], 0.0F, 0.0F, c[ 3U ] );

    float const b[] = { lin._data[ 2U ], std::pow ( omega._data[ 2U ], 2.4F ) };
    result._data[ 1U ] = g[ static_cast<size_t> ( c[ 1U ] > cutoff ) ];
    result._data[ 2U ] = b[ static_cast<size_t> ( c[ 2U ] > cutoff ) ];

    return result;
}

[[maybe_unused]] GXVoid GXColorRGB::ConvertToUByte ( GXUByte &red,
    GXUByte &green,
    GXUByte &blue,
    GXUByte &alpha
) const noexcept
{
    auto const &d = _data;
    red = static_cast<GXUByte> ( std::lround ( d[ 0U ] * RGBA_TO_UBYTE_FACTOR ) );
    green = static_cast<GXUByte> ( std::lround ( d[ 1U ] * RGBA_TO_UBYTE_FACTOR ) );
    blue = static_cast<GXUByte> ( std::lround ( d[ 2U ] * RGBA_TO_UBYTE_FACTOR ) );
    alpha = static_cast<GXUByte> ( std::lround ( d[ 3U ] * RGBA_TO_UBYTE_FACTOR ) );
}

//----------------------------------------------------------------------------------------------------------------------

// NOLINTNEXTLINE
[[maybe_unused]] GXColorHSV::GXColorHSV ( GXColorRGB const &color ) noexcept
{
    From ( color );
}

[[maybe_unused]] GXVoid GXColorHSV::SetHue ( GXFloat hue ) noexcept
{
    _data[ 0U ] = hue;
}

[[maybe_unused]] GXFloat GXColorHSV::GetHue () const noexcept
{
    return _data[ 0U ];
}

[[maybe_unused]] GXVoid GXColorHSV::SetSaturation ( GXFloat saturation ) noexcept
{
    _data[ 1U ] = saturation;
}

[[maybe_unused]] GXFloat GXColorHSV::GetSaturation () const noexcept
{
    return _data[ 1U ];
}

[[maybe_unused]] GXVoid GXColorHSV::SetValue ( GXFloat value ) noexcept
{
    _data[ 2U ] = value;
}

[[maybe_unused]] GXFloat GXColorHSV::GetValue () const noexcept
{
    return _data[ 2U ];
}

[[maybe_unused]] GXVoid GXColorHSV::SetAlpha ( GXFloat alpha ) noexcept
{
    _data[ 3U ] = alpha;
}

[[maybe_unused]] GXFloat GXColorHSV::GetAlpha () const noexcept
{
    return _data[ 3U ];
}

[[maybe_unused]] GXVoid GXColorHSV::From ( GXColorRGB const &color ) noexcept
{
    GXFloat const maxValue = GXMaxf ( GXMaxf ( color.GetRed (), color.GetGreen () ), color.GetBlue () );
    GXFloat const minValue = GXMinf ( GXMinf ( color.GetRed (), color.GetGreen () ), color.GetBlue () );
    auto &d = _data;

    if ( maxValue == minValue )
    {
        d[ 0U ] = 0.0F;
    }
    else if ( maxValue == color.GetRed () && color.GetGreen () >= color.GetBlue () )
    {
        d[ 0U ] = 60.0F * ( ( color.GetGreen () - color.GetBlue () ) / ( maxValue - minValue ) );
    }
    else if ( maxValue == color.GetRed () && color.GetGreen () < color.GetBlue () )
    {
        d[ 0U ] = 60.0F * ( ( color.GetGreen () - color.GetBlue () ) / ( maxValue - minValue ) ) + 360.0F;
    }
    else if ( maxValue == color.GetGreen () )
    {
        d[ 0U ] = 60.0F * ( ( color.GetBlue () - color.GetRed () ) / ( maxValue - minValue ) ) + 120.0F;
    }
    else if ( maxValue == color.GetBlue () )
    {
        d[ 0U ] = 60.0F * ( ( color.GetRed () - color.GetGreen () ) / ( maxValue - minValue ) ) + 240.0F;
    }
    else
    {
        assert ( !"GXColorHSV::From - Something wrong!" );
    }

    if ( maxValue == 0.0F )
        d[ 1U ] = 0.0F;
    else
        d[ 1U ] = 100.0F * ( 1.0F - minValue / maxValue );

    d[ 2U ] = 100.0F * maxValue;
    d[ 3U ] = 100.0F * color.GetAlpha ();
}

//----------------------------------------------------------------------------------------------------------------------

[[maybe_unused]] GXVoid GXPreciseComplex::Init ( GXDouble real, GXDouble imaginary ) noexcept
{
    _r = real;
    _i = imaginary;
}

[[maybe_unused]] GXDouble GXPreciseComplex::Length () const noexcept
{
    return sqrt ( SquaredLength () );
}

[[maybe_unused]] GXDouble GXPreciseComplex::SquaredLength () const noexcept
{
    return _r * _r + _i * _i;
}

[[maybe_unused]] GXBool GXPreciseComplex::Power ( GXUInt power ) noexcept
{
    if ( power == 1U )
        return GX_TRUE;

    if ( power > 1U )
    {
        GXPreciseComplex const alpha ( *this );

        while ( power > 0U )
        {
            *this = *this * alpha;
            --power;
        }

        return GX_TRUE;
    }

    if ( _r == 0.0 && _i == 0.0 )
    {
        assert ( !"GXPreciseComplex::Power - ( 0.0 + 0.0i ) ^ 0 is undefined!" );
        return GX_FALSE;
    }

    _r = 1.0;
    _i = 0.0;

    return GX_FALSE;
}

[[maybe_unused]] GXPreciseComplex GXPreciseComplex::operator + ( GXPreciseComplex const &other ) const noexcept
{
    return
    {
        _r + other._r,
        _i + other._i
    };
}

[[maybe_unused]] GXPreciseComplex GXPreciseComplex::operator - ( GXPreciseComplex const &other ) const noexcept
{
    return
    {
        _r - other._r,
        _i - other._i
    };
}

[[maybe_unused]] GXPreciseComplex GXPreciseComplex::operator * ( GXPreciseComplex const &other ) const noexcept
{
    return
    {
        _r * other._r - _i * other._i,
        _r * other._i + _i * other._r
    };
}

[[maybe_unused]] GXPreciseComplex GXPreciseComplex::operator * ( GXDouble a ) const noexcept
{
    return
    {
        _r * a,
        _i * a
    };
}

[[maybe_unused]] GXPreciseComplex GXPreciseComplex::operator / ( GXDouble a ) const noexcept
{
    GXDouble const invA = 1.0 / a;

    return
    {
        _r * invA,
        _i * invA
    };
}

//----------------------------------------------------------------------------------------------------------------------

// NOLINTNEXTLINE - constructor does not initialize these fields: _data
[[maybe_unused]] GXQuat::GXQuat ( GXMat3 const &rotationMatrix ) noexcept
{
    From ( rotationMatrix );
}

// NOLINTNEXTLINE - constructor does not initialize these fields: _data
[[maybe_unused]] GXQuat::GXQuat ( GXMat4 const &rotationMatrix ) noexcept
{
    From ( rotationMatrix );
}

[[maybe_unused]] GXUInt GXQuat::Compress32 ( bool reflectBitangent ) const noexcept
{
    GXVec3 imaginary = *reinterpret_cast<GXVec3 const*> ( _data + 1U );

    // Shader code expects only positive real value for reconstruction.
    // Using quaternion duality property to satisfy that convention.
    if ( _data[ 0U ] < 0.0F )
        imaginary.Reverse ();

    constexpr uint32_t fixedPoint = 1U << 10U;
    constexpr uint32_t halfFixedPoint = fixedPoint >> 1U;
    constexpr auto snormScale = static_cast<float> ( halfFixedPoint - 1U );
    constexpr auto snormOffset = static_cast<float> ( halfFixedPoint );
    constexpr GXVec3 snormOffsetV3 ( snormOffset, snormOffset, snormOffset );
    imaginary.Sum ( snormOffsetV3, snormScale, imaginary );

    auto const aSnorm = static_cast<uint32_t> ( imaginary._data[ 0U ] );
    auto const bSnorm = static_cast<uint32_t> ( imaginary._data[ 1U ] );
    auto const cSnorm = static_cast<uint32_t> ( imaginary._data[ 2U ] );

    constexpr uint32_t const mirroring[] = { 0b11U << 30U, 0U };
    uint32_t const mirror = mirroring[ static_cast<size_t> ( reflectBitangent ) ];

    return mirror | ( aSnorm << 20U ) | ( bSnorm << 10U ) | cSnorm;
}

[[maybe_unused]] GXUBigInt GXQuat::Compress64 () const noexcept
{
    GXVec4 q = *reinterpret_cast<GXVec4 const*> ( this );

    constexpr uint32_t fixedPoint = 1U << 16U;
    constexpr uint32_t halfFixedPoint = fixedPoint >> 1U;
    constexpr auto snormScale = static_cast<float> ( halfFixedPoint - 1U );
    constexpr auto snormOffset = static_cast<float> ( halfFixedPoint );
    constexpr GXVec4 snormOffsetV4 ( snormOffset, snormOffset, snormOffset, snormOffset );
    q.Sum ( snormOffsetV4, snormScale, q );

    auto const rSnorm = static_cast<uint64_t> ( q._data[ 0U ] );
    auto const aSnorm = static_cast<uint64_t> ( q._data[ 1U ] );
    auto const bSnorm = static_cast<uint64_t> ( q._data[ 2U ] );
    auto const cSnorm = static_cast<uint64_t> ( q._data[ 3U ] );

    return ( rSnorm << 48U ) | ( aSnorm << 32U ) | ( bSnorm << 16U ) | cSnorm;
}

[[maybe_unused]] GXVoid GXQuat::Init ( GXFloat r, GXFloat a, GXFloat b, GXFloat c ) noexcept
{
    auto &d = _data;
    d[ 0U ] = r;
    d[ 1U ] = a;
    d[ 2U ] = b;
    d[ 3U ] = c;
}

[[maybe_unused]] GXVoid GXQuat::SetR ( GXFloat r ) noexcept
{
    _data[ 0U ] = r;
}

[[maybe_unused]] GXFloat GXQuat::GetR () const noexcept
{
    return _data[ 0U ];
}

[[maybe_unused]] GXVoid GXQuat::SetA ( GXFloat a ) noexcept
{
    _data[ 1U ] = a;
}

[[maybe_unused]] GXFloat GXQuat::GetA () const noexcept
{
    return _data[ 1U ];
}

[[maybe_unused]] GXVoid GXQuat::SetB ( GXFloat b ) noexcept
{
    _data[ 2U ] = b;
}

[[maybe_unused]] GXFloat GXQuat::GetB () const noexcept
{
    return _data[ 2U ];
}

[[maybe_unused]] GXVoid GXQuat::SetC ( GXFloat c ) noexcept
{
    _data[ 3U ] = c;
}

[[maybe_unused]] GXFloat GXQuat::GetC () const noexcept
{
    return _data[ 3U ];
}

[[maybe_unused]] GXVoid GXQuat::Identity () noexcept
{
    auto &d = _data;
    d[ 0U ] = 1.0F;
    d[ 1U ] = 0.0F;
    d[ 2U ] = 0.0F;
    d[ 3U ] = 0.0F;
}

[[maybe_unused]] GXVoid GXQuat::FromAxisAngle ( const GXVec3 &axis, GXFloat angle ) noexcept
{
    FromAxisAngle ( axis._data[ 0U ], axis._data[ 1U ], axis._data[ 2U ], angle );
}

[[maybe_unused]] GXVoid GXQuat::InverseFast ( GXQuat const &unitQuaternion ) noexcept
{
    auto &d = _data;
    auto const &unitData = unitQuaternion._data;

    // See https://www.3dgep.com/understanding-quaternions/#Quaternion_Inverse
    d[ 0U ] = unitData[ 0U ];
    d[ 1U ] = -unitData[ 1U ];
    d[ 2U ] = -unitData[ 2U ];
    d[ 3U ] = -unitData[ 3U ];
}

[[maybe_unused]] GXVoid GXQuat::From ( GXMat3 const &rotationMatrix ) noexcept
{
    GXMat3 pureRotationMatrix {};
    pureRotationMatrix.ClearRotation ( rotationMatrix );
    FromFast ( pureRotationMatrix );
}

[[maybe_unused]] GXVoid GXQuat::From ( GXMat4 const &rotationMatrix ) noexcept
{
    GXMat4 pureRotationMatrix {};
    pureRotationMatrix.ClearRotation ( rotationMatrix );
    FromFast ( pureRotationMatrix );
}

[[maybe_unused]] GXVoid GXQuat::FromFast ( GXMat3 const &pureRotationMatrix ) noexcept
{
    // In ideal mathematics world all solutions are right.
    // But in practice more precise solution is the biggest "solutionFactorXXX" because of square root operation.

    auto const &m = pureRotationMatrix._data;

    GXFloat const solutionFactorAlpha = m[ 0U ][ 0U ] + m[ 1U ][ 1U ] + m[ 2U ][ 2U ] + 1.0F;
    GXFloat const solutionFactorBetta = m[ 0U ][ 0U ] - m[ 1U ][ 1U ] - m[ 2U ][ 2U ] + 1.0F;
    GXFloat const solutionFactorGamma = -m[ 0U ][ 0U ] + m[ 1U ][ 1U ] - m[ 2U ][ 2U ] + 1.0F;
    GXFloat const solutionFactorYotta = -m[ 0U ][ 0U ] - m[ 1U ][ 1U ] + m[ 2U ][ 2U ] + 1.0F;

    GXUByte solution;

    if ( solutionFactorAlpha > solutionFactorBetta )
    {
        if ( solutionFactorAlpha > solutionFactorGamma )
        {
            constexpr GXUByte const cases[] = { SOLUTION_YOTTA, SOLUTION_ALPHA };
            solution = cases[ static_cast<size_t> ( solutionFactorAlpha > solutionFactorYotta ) ];
        }
        else
        {
            constexpr GXUByte const cases[] = { SOLUTION_YOTTA, SOLUTION_GAMMA };
            solution = cases[ static_cast<size_t> ( solutionFactorGamma > solutionFactorYotta ) ];
        }
    }
    else if ( solutionFactorBetta > solutionFactorGamma )
    {
        constexpr GXUByte const cases[] = { SOLUTION_YOTTA, SOLUTION_BETTA };
        solution = cases[ static_cast<size_t> ( solutionFactorBetta > solutionFactorYotta ) ];
    }
    else
    {
        constexpr GXUByte const cases[] = { SOLUTION_YOTTA, SOLUTION_GAMMA };
        solution = cases[ static_cast<size_t> ( solutionFactorGamma > solutionFactorYotta ) ];
    }

    auto &d = _data;

    switch ( solution )
    {
        case SOLUTION_ALPHA:
        {
            GXFloat const phi = 0.5F * std::sqrt ( solutionFactorAlpha );
            GXFloat const omega = 0.25F / phi;

            d[ 0U ] = phi;
            d[ 1U ] = omega * ( m[ 1U ][ 2U ] - m[ 2U ][ 1U ] );
            d[ 2U ] = omega * ( m[ 2U ][ 0U ] - m[ 0U ][ 2U ] );
            d[ 3U ] = omega * ( m[ 0U ][ 1U ] - m[ 1U ][ 0U ] );
        }
        break;

        case SOLUTION_BETTA:
        {
            GXFloat const phi = 0.5F * std::sqrt ( solutionFactorBetta );
            GXFloat const omega = 0.25F / phi;

            d[ 0U ] = omega * ( m[ 1U ][ 2U ] - m[ 2U ][ 1U ] );
            d[ 1U ] = phi;
            d[ 2U ] = omega * ( m[ 0U ][ 1U ] + m[ 1U ][ 0U ] );
            d[ 3U ] = omega * ( m[ 0U ][ 2U ] + m[ 2U ][ 0U ] );
        }
        break;

        case SOLUTION_GAMMA:
        {
            GXFloat const phi = 0.5F * std::sqrt ( solutionFactorGamma );
            GXFloat const omega = 0.25F / phi;

            d[ 0U ] = omega * ( m[ 2U ][ 0U ] - m[ 0U ][ 2U ] );
            d[ 1U ] = omega * ( m[ 0U ][ 1U ] + m[ 1U ][ 0U ] );
            d[ 2U ] = phi;
            d[ 3U ] = omega * ( m[ 1U ][ 2U ] + m[ 2U ][ 1U ] );
        }
        break;

        case SOLUTION_YOTTA:
        {
            GXFloat const phi = 0.5F * std::sqrt ( solutionFactorYotta );
            GXFloat const omega = 0.25F / phi;

            d[ 0U ] = omega * ( m[ 0U ][ 1U ] - m[ 1U ][ 0U ] );
            d[ 1U ] = omega * ( m[ 0U ][ 2U ] + m[ 2U ][ 0U ] );
            d[ 2U ] = omega * ( m[ 1U ][ 2U ] + m[ 2U ][ 1U ] );
            d[ 3U ] = phi;
        }
        break;

        default:
            // NOTHING
        break;
    }
}

[[maybe_unused]] GXVoid GXQuat::FromFast ( GXMat4 const &pureRotationMatrix ) noexcept
{
    // In ideal mathematics world all solutions are right.
    // But in practice more precise solution is the biggest "solutionFactorXXX" because of square root operation.

    auto const &m = pureRotationMatrix._data;

    GXFloat const solutionFactorAlpha = m[ 0U ][ 0U ] + m[ 1U ][ 1U ] + m[ 2U ][ 2U ] + 1.0F;
    GXFloat const solutionFactorBetta = m[ 0U ][ 0U ] - m[ 1U ][ 1U ] - m[ 2U ][ 2U ] + 1.0F;
    GXFloat const solutionFactorGamma = -m[ 0U ][ 0U ] + m[ 1U ][ 1U ] - m[ 2U ][ 2U ] + 1.0F;
    GXFloat const solutionFactorYotta = -m[ 0U ][ 0U ] - m[ 1U ][ 1U ] + m[ 2U ][ 2U ] + 1.0F;

    GXUByte solution;

    if ( solutionFactorAlpha > solutionFactorBetta )
    {
        if ( solutionFactorAlpha > solutionFactorGamma )
        {
            constexpr GXUByte const cases[] = { SOLUTION_YOTTA, SOLUTION_ALPHA };
            solution = cases[ static_cast<size_t> ( solutionFactorAlpha > solutionFactorYotta ) ];
        }
        else
        {
            constexpr GXUByte const cases[] = { SOLUTION_YOTTA, SOLUTION_GAMMA };
            solution = cases[ static_cast<size_t> ( solutionFactorGamma > solutionFactorYotta ) ];
        }
    }
    else if ( solutionFactorBetta > solutionFactorGamma )
    {
        constexpr GXUByte const cases[] = { SOLUTION_YOTTA, SOLUTION_BETTA };
        solution = cases[ static_cast<size_t> ( solutionFactorBetta > solutionFactorYotta ) ];
    }
    else
    {
        constexpr GXUByte const cases[] = { SOLUTION_YOTTA, SOLUTION_GAMMA };
        solution = cases[ static_cast<size_t> ( solutionFactorGamma > solutionFactorYotta ) ];
    }

    auto &d = _data;

    switch ( solution )
    {
        case SOLUTION_ALPHA:
        {
            GXFloat const phi = 0.5F * std::sqrt ( solutionFactorAlpha );
            GXFloat const omega = 0.25F / phi;

            d[ 0U ] = phi;
            d[ 1U ] = omega * ( m[ 1U ][ 2U ] - m[ 2U ][ 1U ] );
            d[ 2U ] = omega * ( m[ 2U ][ 0U ] - m[ 0U ][ 2U ] );
            d[ 3U ] = omega * ( m[ 0U ][ 1U ] - m[ 1U ][ 0U ] );
        }
        break;

        case SOLUTION_BETTA:
        {
            GXFloat const phi = 0.5F * std::sqrt ( solutionFactorBetta );
            GXFloat const omega = 0.25F / phi;

            d[ 0U ] = omega * ( m[ 1U ][ 2U ] - m[ 2U ][ 1U ] );
            d[ 1U ] = phi;
            d[ 2U ] = omega * ( m[ 0U ][ 1U ] + m[ 1U ][ 0U ] );
            d[ 3U ] = omega * ( m[ 0U ][ 2U ] + m[ 2U ][ 0U ] );
        }
        break;

        case SOLUTION_GAMMA:
        {
            GXFloat const phi = 0.5F * std::sqrt ( solutionFactorGamma );
            GXFloat const omega = 0.25F / phi;

            d[ 0U ] = omega * ( m[ 2U ][ 0U ] - m[ 0U ][ 2U ] );
            d[ 1U ] = omega * ( m[ 0U ][ 1U ] + m[ 1U ][ 0U ] );
            d[ 2U ] = phi;
            d[ 3U ] = omega * ( m[ 1U ][ 2U ] + m[ 2U ][ 1U ] );
        }
        break;

        case SOLUTION_YOTTA:
        {
            GXFloat const phi = 0.5F * std::sqrt ( solutionFactorYotta );
            GXFloat const omega = 0.25F / phi;

            d[ 0U ] = omega * ( m[ 0U ][ 1U ] - m[ 1U ][ 0U ] );
            d[ 1U ] = omega * ( m[ 0U ][ 2U ] + m[ 2U ][ 0U ] );
            d[ 2U ] = omega * ( m[ 1U ][ 2U ] + m[ 2U ][ 1U ] );
            d[ 3U ] = phi;
        }
        break;

        default:
            // NOTHING
        break;
    }
}

[[maybe_unused]] GXVoid GXQuat::GetAxisAngle ( GXVec3 &axis, GXFloat &angle ) const noexcept
{
    GXQuat q ( *this );

    if ( std::abs ( q._data[ 0U ] ) > 1.0F )
        q.Normalize ();

    angle = 2.0F * std::acos ( q._data[ 0U ] );

    auto &axisData = axis._data;
    axisData[ 0U ] = q._data[ 1U ];
    axisData[ 1U ] = q._data[ 2U ];
    axisData[ 2U ] = q._data[ 3U ];

    GXFloat const s = std::sqrt ( 1.0F - q._data[ 0U ] * q._data[ 0U ] );

    if ( s < GX_MATH_FLOAT_EPSILON )
        return;

    axis.Multiply ( axis, 1.0F / s );
}

[[maybe_unused]] GXVoid GXQuat::Transform ( GXVec3 &out, GXVec3 const &v ) const noexcept
{
    auto const &d = _data;

    GXFloat const rr = d[ 0U ] * d[ 0U ];
    GXFloat const ra2 = d[ 0U ] * d[ 1U ] * 2.0F;
    GXFloat const rb2 = d[ 0U ] * d[ 2U ] * 2.0F;
    GXFloat const rc2 = d[ 0U ] * d[ 3U ] * 2.0F;

    GXFloat const aa = d[ 1U ] * d[ 1U ];
    GXFloat const ab2 = d[ 1U ] * d[ 2U ] * 2.0F;
    GXFloat const ac2 = d[ 1U ] * d[ 3U ] * 2.0F;

    GXFloat const bb = d[ 2U ] * d[ 2U ];
    GXFloat const bc2 = d[ 2U ] * d[ 3U ] * 2.0F;

    GXFloat const cc = d[ 3U ] * d[ 3U ];

    GXFloat const inverseSquaredLength = 1.0F / ( rr + aa + bb + cc );

    auto const &vData = v._data;
    auto &result = out._data;

    result[ 0U ] = inverseSquaredLength *
        ( vData[ 0U ] * ( rr + aa - bb - cc ) + vData[ 1U ] * ( ab2 - rc2 ) + vData[ 2U ] * ( rb2 + ac2 ) );

    result[ 1U ] = inverseSquaredLength *
        ( vData[ 0U ] * ( rc2 + ab2 ) + vData[ 1U ] * ( rr - aa + bb - cc ) + vData[ 2U ] * ( bc2 - ra2 ) );

    result[ 2U ] = inverseSquaredLength *
        ( vData[ 0U ] * ( ac2 - rb2 ) + vData[ 1U ] * ( ra2 + bc2 ) + vData[ 2U ] * ( rr - aa - bb + cc ) );
}

//----------------------------------------------------------------------------------------------------------------------

// NOLINTNEXTLINE
[[maybe_unused]] GXMat3::GXMat3 ( GXMat4 const &matrix ) noexcept
{
    From ( matrix );
}

[[maybe_unused]] GXVoid GXMat3::From ( GXQuat const &quaternion ) noexcept
{
    auto const &qData = quaternion._data;

    GXFloat const rr = qData[ 0U ] * qData[ 0U ];
    GXFloat const ra2 = qData[ 0U ] * qData[ 1U ] * 2.0F;
    GXFloat const rb2 = qData[ 0U ] * qData[ 2U ] * 2.0F;
    GXFloat const rc2 = qData[ 0U ] * qData[ 3u ] * 2.0F;

    GXFloat const aa = qData[ 1U ] * qData[ 1U ];
    GXFloat const ab2 = qData[ 1U ] * qData[ 2U ] * 2.0F;
    GXFloat const ac2 = qData[ 1U ] * qData[ 3u ] * 2.0F;

    GXFloat const bb = qData[ 2U ] * qData[ 2U ];
    GXFloat const bc2 = qData[ 2U ] * qData[ 3u ] * 2.0F;

    GXFloat const cc = qData[ 3U ] * qData[ 3U ];

    GXFloat const inverseSquaredLength = 1.0F / ( rr + aa + bb + cc );

    auto &d = _data;

    d[ 0U ][ 0U ] = inverseSquaredLength * ( rr + aa - bb - cc );
    d[ 0U ][ 1U ] = inverseSquaredLength * ( rc2 + ab2 );
    d[ 0U ][ 2U ] = inverseSquaredLength * ( ac2 - rb2 );

    d[ 1U ][ 0U ] = inverseSquaredLength * ( ab2 - rc2 );
    d[ 1U ][ 1U ] = inverseSquaredLength * ( rr - aa + bb - cc );
    d[ 1U ][ 2U ] = inverseSquaredLength * ( ra2 + bc2 );

    d[ 2U ][ 0U ] = inverseSquaredLength * ( rb2 + ac2 );
    d[ 2U ][ 1U ] = inverseSquaredLength * ( bc2 - ra2 );
    d[ 2U ][ 2U ] = inverseSquaredLength * ( rr - aa - bb + cc );
}

[[maybe_unused]] GXVoid GXMat3::From ( GXMat4 const &matrix ) noexcept
{
    constexpr GXUPointer lineSize = 3U * sizeof ( GXFloat );
    auto &d = _data;
    auto const &m = matrix._data;

    std::memcpy ( d, m, lineSize );
    std::memcpy ( d + 1U, m + 1U, lineSize );
    std::memcpy ( d + 2U, m + 2U, lineSize );
}

[[maybe_unused]] GXVoid GXMat3::From ( const GXVec3 &zDirection ) noexcept
{
    GXVec3 xAxis {};
    GXVec3 yAxis {};

    if ( std::abs ( zDirection.DotProduct ( GXVec3::GetAbsoluteX () ) ) < 0.5F )
    {
        GXVec3 tmp {};
        tmp.CrossProduct ( zDirection, GXVec3::GetAbsoluteX () );
        xAxis.CrossProduct ( tmp, zDirection );
        xAxis.Normalize ();
        yAxis.CrossProduct ( zDirection, xAxis );
    }
    else
    {
        GXVec3 tmp {};
        tmp.CrossProduct ( zDirection, GXVec3::GetAbsoluteY () );
        yAxis.CrossProduct ( zDirection, tmp );
        yAxis.Normalize ();
        xAxis.CrossProduct ( yAxis, zDirection );
    }

    SetX ( xAxis );
    SetY ( yAxis );
    SetZ ( zDirection );
}

[[maybe_unused]] GXVoid GXMat3::FromFast ( GXQuat const &quaternion ) noexcept
{
    auto const &qData = quaternion._data;

    GXFloat const rr = qData[ 0U ] * qData[ 0U ];
    GXFloat const ra2 = qData[ 0U ] * qData[ 1U ] * 2.0F;
    GXFloat const rb2 = qData[ 0U ] * qData[ 2U ] * 2.0F;
    GXFloat const rc2 = qData[ 0U ] * qData[ 3u ] * 2.0F;

    GXFloat const aa = qData[ 1U ] * qData[ 1U ];
    GXFloat const ab2 = qData[ 1U ] * qData[ 2U ] * 2.0F;
    GXFloat const ac2 = qData[ 1U ] * qData[ 3u ] * 2.0F;

    GXFloat const bb = qData[ 2U ] * qData[ 2U ];
    GXFloat const bc2 = qData[ 2U ] * qData[ 3u ] * 2.0F;

    GXFloat const cc = qData[ 3u ] * qData[ 3u ];

    auto &d = _data;

    d[ 0U ][ 0U ] = rr + aa - bb - cc;
    d[ 0U ][ 1U ] = rc2 + ab2;
    d[ 0U ][ 2U ] = ac2 - rb2;

    d[ 1U ][ 0U ] = ab2 - rc2;
    d[ 1U ][ 1U ] = rr - aa + bb - cc;
    d[ 1U ][ 2U ] = ra2 + bc2;

    d[ 2U ][ 0U ] = rb2 + ac2;
    d[ 2U ][ 1U ] = bc2 - ra2;
    d[ 2U ][ 2U ] = rr - aa - bb + cc;
}

[[maybe_unused]] GXVoid GXMat3::SetX ( GXVec3 const &x ) noexcept
{
    std::memcpy ( _data, &x, sizeof ( GXVec3 ) );
}

[[maybe_unused]] GXVoid GXMat3::GetX ( GXVec3 &x ) const noexcept
{
    std::memcpy ( &x, _data, sizeof ( GXVec3 ) );
}

[[maybe_unused]] GXVoid GXMat3::SetY ( GXVec3 const &y ) noexcept
{
    std::memcpy ( _data + 1U, &y, sizeof ( GXVec3 ) );
}

[[maybe_unused]] GXVoid GXMat3::GetY ( GXVec3 &y ) const noexcept
{
    std::memcpy ( &y, _data + 1U, sizeof ( GXVec3 ) );
}

[[maybe_unused]] GXVoid GXMat3::SetZ ( GXVec3 const &z ) noexcept
{
    std::memcpy ( _data + 2U, &z, sizeof ( GXVec3 ) );
}

[[maybe_unused]] GXVoid GXMat3::GetZ ( GXVec3 &z ) const noexcept
{
    std::memcpy ( &z, _data + 2U, sizeof ( GXVec3 ) );
}

[[maybe_unused]] GXVoid GXMat3::Identity () noexcept
{
    auto &d = _data;

    d[ 0U ][ 0U ] = 1.0F;
    d[ 1U ][ 1U ] = 1.0F;
    d[ 2U ][ 2U ] = 1.0F;

    d[ 0U ][ 1U ] = 0.0F;
    d[ 0U ][ 2U ] = 0.0F;
    d[ 1U ][ 0U ] = 0.0F;
    d[ 1U ][ 2U ] = 0.0F;
    d[ 2U ][ 0U ] = 0.0F;
    d[ 2U ][ 1U ] = 0.0F;
}

[[maybe_unused]] GXVoid GXMat3::Zeros () noexcept
{
    memset ( _data, 0, sizeof ( _data ) );
}

[[maybe_unused]] GXVoid GXMat3::Inverse ( GXMat3 const &sourceMatrix ) noexcept
{
    auto const &m = sourceMatrix._data;

    GXFloat determinant = m[ 0U ][ 0U ] * ( m[ 1U ][ 1U ] * m[ 2U ][ 2U ] - m[ 2U ][ 1U ] * m[ 1U ][ 2U ] );
    determinant -= m[ 0U ][ 1U ] * ( m[ 1U ][ 0U ] * m[ 2U ][ 2U ] - m[ 2U ][ 0U ] * m[ 1U ][ 2U ] );
    determinant += m[ 0U ][ 2U ] * ( m[ 1U ][ 0U ] * m[ 2U ][ 1U ] - m[ 2U ][ 0U ] * m[ 1U ][ 1U ] );

    GXFloat const invDeterminant = 1.0F / determinant;
    auto &d = _data;

    d[ 0U ][ 0U ] = invDeterminant * ( m[ 1U ][ 1U ] * m[ 2U ][ 2U ] - m[ 2U ][ 1U ] * m[ 1U ][ 2U ] );
    d[ 0U ][ 1U ] = invDeterminant * ( m[ 0U ][ 2U ] * m[ 2U ][ 1U ] - m[ 2U ][ 2U ] * m[ 0U ][ 1U ] );
    d[ 0U ][ 2U ] = invDeterminant * ( m[ 0U ][ 1U ] * m[ 1U ][ 2U ] - m[ 1U ][ 1U ] * m[ 0U ][ 2U ] );

    d[ 1U ][ 0U ] = invDeterminant * ( m[ 1U ][ 2U ] * m[ 2U ][ 0U ] - m[ 2U ][ 2U ] * m[ 1U ][ 0U ] );
    d[ 1U ][ 1U ] = invDeterminant * ( m[ 0U ][ 0U ] * m[ 2U ][ 2U ] - m[ 2U ][ 0U ] * m[ 0U ][ 2U ] );
    d[ 1U ][ 2U ] = invDeterminant * ( m[ 0U ][ 2U ] * m[ 1U ][ 0U ] - m[ 1U ][ 2U ] * m[ 0U ][ 0U ] );

    d[ 2U ][ 0U ] = invDeterminant * ( m[ 1U ][ 0U ] * m[ 2U ][ 1U ] - m[ 2U ][ 0U ] * m[ 1U ][ 1U ] );
    d[ 2U ][ 1U ] = invDeterminant * ( m[ 0U ][ 1U ] * m[ 2U ][ 0U ] - m[ 2U ][ 1U ] * m[ 0U ][ 0U ] );
    d[ 2U ][ 2U ] = invDeterminant * ( m[ 0U ][ 0U ] * m[ 1U ][ 1U ] - m[ 1U ][ 0U ] * m[ 0U ][ 1U ] );
}

[[maybe_unused]] GXVoid GXMat3::Transpose ( GXMat3 const &sourceMatrix ) noexcept
{
    auto const &m = sourceMatrix._data;
    auto &d = _data;

    d[ 0U ][ 0U ] = m[ 0U ][ 0U ];
    d[ 0U ][ 1U ] = m[ 1U ][ 0U ];
    d[ 0U ][ 2U ] = m[ 2U ][ 0U ];

    d[ 1U ][ 0U ] = m[ 0U ][ 1U ];
    d[ 1U ][ 1U ] = m[ 1U ][ 1U ];
    d[ 1U ][ 2U ] = m[ 2U ][ 1U ];

    d[ 2U ][ 0U ] = m[ 0U ][ 2U ];
    d[ 2U ][ 1U ] = m[ 1U ][ 2U ];
    d[ 2U ][ 2U ] = m[ 2U ][ 2U ];
}

[[maybe_unused]] GXVoid GXMat3::ClearRotation ( GXMat3 const &sourceMatrix ) noexcept
{
    GXVec3 modelX {};
    GXVec3 modelY {};
    GXVec3 modelZ {};

    sourceMatrix.GetX ( modelX );
    sourceMatrix.GetY ( modelY );
    sourceMatrix.GetZ ( modelZ );

    GXVec3 tmp {};
    tmp.Multiply ( modelX, 1.0F / modelX.Length () );
    SetX ( tmp );

    tmp.Multiply ( modelY, 1.0F / modelY.Length () );
    SetY ( tmp );

    tmp.Multiply ( modelZ, 1.0F / modelZ.Length () );
    SetZ ( tmp );
}

[[maybe_unused]] GXVoid GXMat3::ClearRotation ( GXMat4 const &sourceMatrix ) noexcept
{
    GXVec3 modelX {};
    GXVec3 modelY {};
    GXVec3 modelZ {};

    sourceMatrix.GetX ( modelX );
    sourceMatrix.GetY ( modelY );
    sourceMatrix.GetZ ( modelZ );

    GXVec3 tmp {};
    tmp.Multiply ( modelX, 1.0F / modelX.Length () );
    SetX ( tmp );

    tmp.Multiply ( modelY, 1.0F / modelY.Length () );
    SetY ( tmp );

    tmp.Multiply ( modelZ, 1.0F / modelZ.Length () );
    SetZ ( tmp );
}

[[maybe_unused]] GXVoid GXMat3::SkewSymmetric ( GXVec3 const &base ) noexcept
{
    auto const &bData = base._data;
    auto &d = _data;

    d[ 0U ][ 0U ] = 0.0F;
    d[ 1U ][ 1U ] = 0.0F;
    d[ 2U ][ 2U ] = 0.0F;

    d[ 0U ][ 1U ] = -bData[ 2U ];
    d[ 0U ][ 2U ] = bData[ 1U ];

    d[ 1U ][ 0U ] = bData[ 2U ];
    d[ 1U ][ 2U ] = -bData[ 0U ];

    d[ 2U ][ 0U ] = -bData[ 1U ];
    d[ 2U ][ 1U ] = bData[ 0U ];
}

[[maybe_unused]] GXVoid GXMat3::Sum ( GXMat3 const &a, GXMat3 const &b ) noexcept
{
    auto const &aData = a._data;
    auto const &bData = b._data;
    auto &d = _data;

    d[ 0U ][ 0U ] = aData[ 0U ][ 0U ] + bData[ 0U ][ 0U ];
    d[ 0U ][ 1U ] = aData[ 0U ][ 1U ] + bData[ 0U ][ 1U ];
    d[ 0U ][ 2U ] = aData[ 0U ][ 2U ] + bData[ 0U ][ 2U ];

    d[ 1U ][ 0U ] = aData[ 1U ][ 0U ] + bData[ 1U ][ 0U ];
    d[ 1U ][ 1U ] = aData[ 1U ][ 1U ] + bData[ 1U ][ 1U ];
    d[ 1U ][ 2U ] = aData[ 1U ][ 2U ] + bData[ 1U ][ 2U ];

    d[ 2U ][ 0U ] = aData[ 2U ][ 0U ] + bData[ 2U ][ 0U ];
    d[ 2U ][ 1U ] = aData[ 2U ][ 1U ] + bData[ 2U ][ 1U ];
    d[ 2U ][ 2U ] = aData[ 2U ][ 2U ] + bData[ 2U ][ 2U ];
}

[[maybe_unused]] GXVoid GXMat3::Subtract ( GXMat3 const &a, GXMat3 const &b ) noexcept
{
    auto const &aData = a._data;
    auto const &bData = b._data;
    auto &d = _data;

    d[ 0U ][ 0U ] = aData[ 0U ][ 0U ] - bData[ 0U ][ 0U ];
    d[ 0U ][ 1U ] = aData[ 0U ][ 1U ] - bData[ 0U ][ 1U ];
    d[ 0U ][ 2U ] = aData[ 0U ][ 2U ] - bData[ 0U ][ 2U ];

    d[ 1U ][ 0U ] = aData[ 1U ][ 0U ] - bData[ 1U ][ 0U ];
    d[ 1U ][ 1U ] = aData[ 1U ][ 1U ] - bData[ 1U ][ 1U ];
    d[ 1U ][ 2U ] = aData[ 1U ][ 2U ] - bData[ 1U ][ 2U ];

    d[ 2U ][ 0U ] = aData[ 2U ][ 0U ] - bData[ 2U ][ 0U ];
    d[ 2U ][ 1U ] = aData[ 2U ][ 1U ] - bData[ 2U ][ 1U ];
    d[ 2U ][ 2U ] = aData[ 2U ][ 2U ] - bData[ 2U ][ 2U ];
}

[[maybe_unused]] GXVoid GXMat3::Multiply ( GXMat3 const &a, GXFloat factor ) noexcept
{
    auto const &aData = a._data;
    auto &d = _data;

    d[ 0U ][ 0U ] = aData[ 0U ][ 0U ] * factor;
    d[ 0U ][ 1U ] = aData[ 0U ][ 1U ] * factor;
    d[ 0U ][ 2U ] = aData[ 0U ][ 2U ] * factor;

    d[ 1U ][ 0U ] = aData[ 1U ][ 0U ] * factor;
    d[ 1U ][ 1U ] = aData[ 1U ][ 1U ] * factor;
    d[ 1U ][ 2U ] = aData[ 1U ][ 2U ] * factor;

    d[ 2U ][ 0U ] = aData[ 2U ][ 0U ] * factor;
    d[ 2U ][ 1U ] = aData[ 2U ][ 1U ] * factor;
    d[ 2U ][ 2U ] = aData[ 2U ][ 2U ] * factor;
}

//----------------------------------------------------------------------------------------------------------------------

[[maybe_unused]] GXVoid GXMat4::SetRotation ( GXQuat const &quaternion ) noexcept
{
    auto const &qData = quaternion._data;

    GXFloat const rr = qData[ 0U ] * qData[ 0U ];
    GXFloat const ra2 = qData[ 0U ] * qData[ 1U ] * 2.0F;
    GXFloat const rb2 = qData[ 0U ] * qData[ 2U ] * 2.0F;
    GXFloat const rc2 = qData[ 0U ] * qData[ 3U ] * 2.0F;

    GXFloat const aa = qData[ 1U ] * qData[ 1U ];
    GXFloat const ab2 = qData[ 1U ] * qData[ 2U ] * 2.0F;
    GXFloat const ac2 = qData[ 1U ] * qData[ 3U ] * 2.0F;

    GXFloat const bb = qData[ 2U ] * qData[ 2U ];
    GXFloat const bc2 = qData[ 2U ] * qData[ 3U ] * 2.0F;

    GXFloat const cc = qData[ 3U ] * qData[ 3U ];

    GXFloat const inverseSquaredLength = 1.0F / ( rr + aa + bb + cc );
    auto &d = _data;

    d[ 0U ][ 0U ] = inverseSquaredLength * ( rr + aa - bb - cc );
    d[ 0U ][ 1U ] = inverseSquaredLength * ( rc2 + ab2 );
    d[ 0U ][ 2U ] = inverseSquaredLength * ( ac2 - rb2 );

    d[ 1U ][ 0U ] = inverseSquaredLength * ( ab2 - rc2 );
    d[ 1U ][ 1U ] = inverseSquaredLength * ( rr - aa + bb - cc );
    d[ 1U ][ 2U ] = inverseSquaredLength * ( ra2 + bc2 );

    d[ 2U ][ 0U ] = inverseSquaredLength * ( rb2 + ac2 );
    d[ 2U ][ 1U ] = inverseSquaredLength * ( bc2 - ra2 );
    d[ 2U ][ 2U ] = inverseSquaredLength * ( rr - aa - bb + cc );
}

[[maybe_unused]] GXVoid GXMat4::SetRotationFast ( GXQuat const &quaternion ) noexcept
{
    auto const &qData = quaternion._data;

    GXFloat const rr = qData[ 0U ] * qData[ 0U ];
    GXFloat const ra2 = qData[ 0U ] * qData[ 1U ] * 2.0F;
    GXFloat const rb2 = qData[ 0U ] * qData[ 2U ] * 2.0F;
    GXFloat const rc2 = qData[ 0U ] * qData[ 3U ] * 2.0F;

    GXFloat const aa = qData[ 1U ] * qData[ 1U ];
    GXFloat const ab2 = qData[ 1U ] * qData[ 2U ] * 2.0F;
    GXFloat const ac2 = qData[ 1U ] * qData[ 3U ] * 2.0F;

    GXFloat const bb = qData[ 2U ] * qData[ 2U ];
    GXFloat const bc2 = qData[ 2U ] * qData[ 3U ] * 2.0F;

    GXFloat const cc = qData[ 3U ] * qData[ 3U ];

    auto &d = _data;

    d[ 0U ][ 0U ] = rr + aa - bb - cc;
    d[ 0U ][ 1U ] = rc2 + ab2;
    d[ 0U ][ 2U ] = ac2 - rb2;

    d[ 1U ][ 0U ] = ab2 - rc2;
    d[ 1U ][ 1U ] = rr - aa + bb - cc;
    d[ 1U ][ 2U ] = ra2 + bc2;

    d[ 2U ][ 0U ] = rb2 + ac2;
    d[ 2U ][ 1U ] = bc2 - ra2;
    d[ 2U ][ 2U ] = rr - aa - bb + cc;
}

[[maybe_unused]] GXVoid GXMat4::SetOrigin ( GXVec3 const &origin ) noexcept
{
    SetW ( origin );
}

[[maybe_unused]] GXVoid GXMat4::From ( GXQuat const &quaternion, GXVec3 const &origin ) noexcept
{
    SetRotation ( quaternion );
    SetOrigin ( origin );

    auto &d = _data;

    d[ 0U ][ 3U ] = 0.0F;
    d[ 1U ][ 3U ] = 0.0F;
    d[ 2U ][ 3U ] = 0.0F;
    d[ 3U ][ 3U ] = 1.0F;
}

[[maybe_unused]] GXVoid GXMat4::From ( GXMat3 const &rotation, GXVec3 const &origin ) noexcept
{
    GXVec3 tmp {};
    rotation.GetX ( tmp );
    SetX ( tmp );

    rotation.GetY ( tmp );
    SetY ( tmp );

    rotation.GetZ ( tmp );
    SetZ ( tmp );

    SetW ( origin );

    auto &d = _data;

    d[ 0U ][ 3U ] = 0.0F;
    d[ 1U ][ 3U ] = 0.0F;
    d[ 2U ][ 3U ] = 0.0F;
    d[ 3U ][ 3U ] = 1.0F;
}

[[maybe_unused]] GXVoid GXMat4::From ( GXVec3 const &zDirection, GXVec3 const &origin ) noexcept
{
    GXVec3 xAxis {};
    GXVec3 yAxis {};

    if ( std::abs ( zDirection.DotProduct ( GXVec3::GetAbsoluteX () ) ) < 0.5F )
    {
        GXVec3 tmp {};
        tmp.CrossProduct ( zDirection, GXVec3::GetAbsoluteX () );
        xAxis.CrossProduct ( tmp, zDirection );
        xAxis.Normalize ();
        yAxis.CrossProduct ( zDirection, xAxis );
    }
    else
    {
        GXVec3 tmp {};
        tmp.CrossProduct ( zDirection, GXVec3::GetAbsoluteY () );
        yAxis.CrossProduct ( zDirection, tmp );
        yAxis.Normalize ();
        xAxis.CrossProduct ( yAxis, zDirection );
    }

    SetX ( xAxis );
    SetY ( yAxis );
    SetZ ( zDirection );
    SetW ( origin );

    auto &d = _data;

    d[ 0U ][ 3U ] = 0.0F;
    d[ 1U ][ 3U ] = 0.0F;
    d[ 2U ][ 3U ] = 0.0F;
    d[ 3U ][ 3U ] = 1.0F;
}

[[maybe_unused]] GXVoid GXMat4::FromFast ( GXQuat const &quaternion, GXVec3 const &origin ) noexcept
{
    SetRotationFast ( quaternion );
    SetOrigin ( origin );

    auto &d = _data;

    d[ 0U ][ 3U ] = 0.0F;
    d[ 1U ][ 3U ] = 0.0F;
    d[ 2U ][ 3U ] = 0.0F;
    d[ 3U ][ 3U ] = 1.0F;
}

[[maybe_unused]] GXVoid GXMat4::SetX ( GXVec3 const &x ) noexcept
{
    std::memcpy ( _data, &x, sizeof ( GXVec3 ) );
}

[[maybe_unused]] GXVoid GXMat4::GetX ( GXVec3 &x ) const noexcept
{
    std::memcpy ( &x, _data, sizeof ( GXVec3 ) );
}

[[maybe_unused]] GXVoid GXMat4::SetY ( GXVec3 const &y ) noexcept
{
    std::memcpy ( _data + 1U, &y, sizeof ( GXVec3 ) );
}

[[maybe_unused]] GXVoid GXMat4::GetY ( GXVec3 &y ) const noexcept
{
    std::memcpy ( &y, _data + 1U, sizeof ( GXVec3 ) );
}

[[maybe_unused]] GXVoid GXMat4::SetZ ( GXVec3 const &z ) noexcept
{
    std::memcpy ( _data + 2U, &z, sizeof ( GXVec3 ) );
}

[[maybe_unused]] GXVoid GXMat4::GetZ ( GXVec3 &z ) const noexcept
{
    std::memcpy ( &z, _data + 2U, sizeof ( GXVec3 ) );
}

[[maybe_unused]] GXVoid GXMat4::SetW ( GXVec3 const &w ) noexcept
{
    std::memcpy ( _data + 3U, &w, sizeof ( GXVec3 ) );
}

[[maybe_unused]] GXVoid GXMat4::GetW ( GXVec3 &w ) const noexcept
{
    std::memcpy ( &w, _data + 3U, sizeof ( GXVec3 ) );
}

[[maybe_unused]] GXVoid GXMat4::Identity () noexcept
{
    auto &d = _data;

    d[ 0U ][ 1U ] = 0.0F;
    d[ 0U ][ 2U ] = 0.0F;
    d[ 0U ][ 3U ] = 0.0F;
    d[ 1U ][ 0U ] = 0.0F;
    d[ 1U ][ 2U ] = 0.0F;
    d[ 1U ][ 3U ] = 0.0F;
    d[ 2U ][ 0U ] = 0.0F;
    d[ 2U ][ 1U ] = 0.0F;
    d[ 2U ][ 3U ] = 0.0F;
    d[ 3U ][ 0U ] = 0.0F;
    d[ 3U ][ 1U ] = 0.0F;
    d[ 3U ][ 2U ] = 0.0F;

    d[ 0U ][ 0U ] = 1.0F;
    d[ 1U ][ 1U ] = 1.0F;
    d[ 2U ][ 2U ] = 1.0F;
    d[ 3U ][ 3U ] = 1.0F;
}

[[maybe_unused]] GXVoid GXMat4::Translation ( GXFloat x, GXFloat y, GXFloat z ) noexcept
{
    auto &d = _data;

    d[ 0U ][ 1U ] = 0.0F;
    d[ 0U ][ 2U ] = 0.0F;
    d[ 0U ][ 3U ] = 0.0F;
    d[ 1U ][ 0U ] = 0.0F;
    d[ 1U ][ 2U ] = 0.0F;
    d[ 1U ][ 3U ] = 0.0F;
    d[ 2U ][ 0U ] = 0.0F;
    d[ 2U ][ 1U ] = 0.0F;
    d[ 2U ][ 3U ] = 0.0F;

    d[ 0U ][ 0U ] = 1.0F;
    d[ 1U ][ 1U ] = 1.0F;
    d[ 2U ][ 2U ] = 1.0F;
    d[ 3U ][ 3U ] = 1.0F;

    d[ 3U ][ 0U ] = x;
    d[ 3U ][ 1U ] = y;
    d[ 3U ][ 2U ] = z;
}

[[maybe_unused]] GXVoid GXMat4::Translation ( GXVec3 const &location ) noexcept
{
    auto &d = _data;

    d[ 0U ][ 1U ] = 0.0F;
    d[ 0U ][ 2U ] = 0.0F;
    d[ 0U ][ 3U ] = 0.0F;
    d[ 1U ][ 0U ] = 0.0F;
    d[ 1U ][ 2U ] = 0.0F;
    d[ 1U ][ 3U ] = 0.0F;
    d[ 2U ][ 0U ] = 0.0F;
    d[ 2U ][ 1U ] = 0.0F;
    d[ 2U ][ 3U ] = 0.0F;

    d[ 0U ][ 0U ] = 1.0F;
    d[ 1U ][ 1U ] = 1.0F;
    d[ 2U ][ 2U ] = 1.0F;
    d[ 3U ][ 3U ] = 1.0F;

    std::memcpy ( d[ 3U ], location._data, sizeof ( location ) );
}

[[maybe_unused]] GXVoid GXMat4::TranslateTo ( GXFloat x, GXFloat y, GXFloat z ) noexcept
{
    GXVec3 origin ( x, y, z );
    SetW ( origin );
}

[[maybe_unused]] GXVoid GXMat4::TranslateTo ( GXVec3 const &location ) noexcept
{
    SetW ( location );
}

[[maybe_unused]] GXVoid GXMat4::RotationX ( GXFloat angle ) noexcept
{
    auto &d = _data;

    d[ 0U ][ 1U ] = 0.0F;
    d[ 0U ][ 2U ] = 0.0F;
    d[ 1U ][ 0U ] = 0.0F;
    d[ 2U ][ 0U ] = 0.0F;

    GXFloat const c = std::cos ( angle );
    GXFloat const s = std::sin ( angle );

    d[ 1U ][ 1U ] = c;
    d[ 1U ][ 2U ] = s;
    d[ 2U ][ 1U ] = -s;
    d[ 2U ][ 2U ] = c;

    d[ 0U ][ 0U ] = 1.0F;

    d[ 3U ][ 3U ] = 1.0F;
    d[ 3U ][ 0U ] = 0.0F;
    d[ 3U ][ 1U ] = 0.0F;
    d[ 3U ][ 2U ] = 0.0F;
    d[ 0U ][ 3U ] = 0.0F;
    d[ 1U ][ 3U ] = 0.0F;
    d[ 2U ][ 3U ] = 0.0F;
}

[[maybe_unused]] GXVoid GXMat4::RotationY ( GXFloat angle ) noexcept
{
    auto &d = _data;

    d[ 0U ][ 1U ] = 0.0F;
    d[ 1U ][ 0U ] = 0.0F;
    d[ 1U ][ 2U ] = 0.0F;
    d[ 2U ][ 1U ] = 0.0F;

    GXFloat const c = std::cos ( angle );
    GXFloat const s = std::sin ( angle );

    d[ 0U ][ 0U ] = c;
    d[ 0U ][ 2U ] = -s;
    d[ 2U ][ 0U ] = s;
    d[ 2U ][ 2U ] = c;

    d[ 1U ][ 1U ] = 1.0F;

    d[ 3U ][ 3U ] = 1.0F;
    d[ 3U ][ 0U ] = 0.0F;
    d[ 3U ][ 1U ] = 0.0F;
    d[ 3U ][ 2U ] = 0.0F;
    d[ 0U ][ 3U ] = 0.0F;
    d[ 1U ][ 3U ] = 0.0F;
    d[ 2U ][ 3U ] = 0.0F;
}

[[maybe_unused]] GXVoid GXMat4::RotationZ ( GXFloat angle ) noexcept
{
    auto &d = _data;

    d[ 0U ][ 2U ] = 0.0F;
    d[ 1U ][ 2U ] = 0.0F;
    d[ 2U ][ 0U ] = 0.0F;
    d[ 2U ][ 1U ] = 0.0F;

    GXFloat const c = std::cos ( angle );
    GXFloat const s = std::sin ( angle );

    d[ 0U ][ 0U ] = c;
    d[ 0U ][ 1U ] = s;
    d[ 1U ][ 0U ] = -s;
    d[ 1U ][ 1U ] = c;

    d[ 2U ][ 2U ] = 1.0F;

    d[ 3U ][ 3U ] = 1.0F;
    d[ 3U ][ 0U ] = 0.0F;
    d[ 3U ][ 1U ] = 0.0F;
    d[ 3U ][ 2U ] = 0.0F;
    d[ 0U ][ 3U ] = 0.0F;
    d[ 1U ][ 3U ] = 0.0F;
    d[ 2U ][ 3U ] = 0.0F;
}

[[maybe_unused]] GXVoid GXMat4::RotationXY ( GXFloat pitchRadians, GXFloat yawRadians ) noexcept
{
    GXMat4 x {};
    x.RotationX ( pitchRadians );

    GXMat4 y {};
    y.RotationY ( yawRadians );

    Multiply ( x, y );
}

[[maybe_unused]] GXVoid GXMat4::RotationXYZ ( GXFloat pitchRadians, GXFloat yawRadians, GXFloat rollRadians ) noexcept
{
    GXMat4 x {};
    x.RotationX ( pitchRadians );

    GXMat4 y {};
    y.RotationY ( yawRadians );

    GXMat4 z {};
    z.RotationZ ( rollRadians );

    GXMat4 temp {};
    temp.Multiply ( x, y );

    Multiply ( temp, z );
}

[[maybe_unused]] GXVoid GXMat4::ClearRotation ( GXMat3 const &sourceMatrix ) noexcept
{
    GXVec3 modelX {};
    GXVec3 modelY {};
    GXVec3 modelZ {};

    sourceMatrix.GetX ( modelX );
    sourceMatrix.GetY ( modelY );
    sourceMatrix.GetZ ( modelZ );

    GXVec3 tmp {};
    tmp.Multiply ( modelX, 1.0F / modelX.Length () );
    SetX ( tmp );

    tmp.Multiply ( modelY, 1.0F / modelY.Length () );
    SetY ( tmp );

    tmp.Multiply ( modelZ, 1.0F / modelZ.Length () );
    SetZ ( tmp );

    auto &d = _data;

    d[ 0U ][ 3U ] = 0.0F;
    d[ 1U ][ 3U ] = 0.0F;
    d[ 2U ][ 3U ] = 0.0F;
    d[ 3U ][ 0U ] = 0.0F;
    d[ 3U ][ 1U ] = 0.0F;
    d[ 3U ][ 2U ] = 0.0F;

    d[ 3U ][ 3U ] = 1.0F;
}

[[maybe_unused]] GXVoid GXMat4::ClearRotation ( GXMat4 const &sourceMatrix ) noexcept
{
    GXVec3 modelX {};
    GXVec3 modelY {};
    GXVec3 modelZ {};

    sourceMatrix.GetX ( modelX );
    sourceMatrix.GetY ( modelY );
    sourceMatrix.GetZ ( modelZ );

    GXVec3 tmp {};
    tmp.Multiply ( modelX, 1.0F / modelX.Length () );
    SetX ( tmp );

    tmp.Multiply ( modelY, 1.0F / modelY.Length () );
    SetY ( tmp );

    tmp.Multiply ( modelZ, 1.0F / modelZ.Length () );
    SetZ ( tmp );

    auto &d = _data;

    d[ 0U ][ 3U ] = 0.0F;
    d[ 1U ][ 3U ] = 0.0F;
    d[ 2U ][ 3U ] = 0.0F;
    d[ 3U ][ 0U ] = 0.0F;
    d[ 3U ][ 1U ] = 0.0F;
    d[ 3U ][ 2U ] = 0.0F;

    d[ 3U ][ 3U ] = 1.0F;
}

[[maybe_unused]] GXVoid GXMat4::Scale ( GXFloat x, GXFloat y, GXFloat z ) noexcept
{
    auto &d = _data;

    d[ 0U ][ 0U ] = x;
    d[ 1U ][ 1U ] = y;
    d[ 2U ][ 2U ] = z;

    d[ 3U ][ 3U ] = 1.0F;

    d[ 0U ][ 1U ] = 0.0F;
    d[ 0U ][ 2U ] = 0.0F;
    d[ 1U ][ 0U ] = 0.0F;
    d[ 1U ][ 2U ] = 0.0F;
    d[ 2U ][ 0U ] = 0.0F;
    d[ 2U ][ 1U ] = 0.0F;
    d[ 3U ][ 0U ] = 0.0F;
    d[ 3U ][ 1U ] = 0.0F;
    d[ 3U ][ 2U ] = 0.0F;
    d[ 0U ][ 3U ] = 0.0F;
    d[ 1U ][ 3U ] = 0.0F;
    d[ 2U ][ 3U ] = 0.0F;
}

[[maybe_unused]] GXVoid GXMat4::ClearScale ( GXVec3 &scale ) const noexcept
{
    GXVec3 alpha {};

    GetX ( alpha );
    scale._data[ 0U ] = alpha.Length ();

    GetY ( alpha );
    scale._data[ 1U ] = alpha.Length ();

    GetZ ( alpha );
    scale._data[ 2U ] = alpha.Length ();
}

//----------------------------------------------------------------------------------------------------------------------

[[maybe_unused]] GXVoid GXAABB::Empty () noexcept
{
    _vertices = 0u;

    _min = GXVec3 ( FLT_MAX, FLT_MAX, FLT_MAX );
    _max = GXVec3 ( -FLT_MAX, -FLT_MAX, -FLT_MAX );
}

[[maybe_unused]] GXVoid GXAABB::Transform ( GXAABB &bounds, GXMat4 const &transform ) const noexcept
{
    auto const &minData = _min._data;
    auto const &maxData = _max._data;

    GXVec3 const verticesLocal[] =
    {
        GXVec3 ( minData[ 0U ], minData[ 1U ], minData[ 2U ] ),
        GXVec3 ( maxData[ 0U ], minData[ 1U ], minData[ 2U ] ),
        GXVec3 ( maxData[ 0U ], minData[ 1U ], maxData[ 2U ] ),
        GXVec3 ( minData[ 0U ], minData[ 1U ], maxData[ 2U ] ),
        GXVec3 ( minData[ 0U ], maxData[ 1U ], minData[ 2U ] ),
        GXVec3 ( maxData[ 0U ], maxData[ 1U ], minData[ 2U ] ),
        GXVec3 ( maxData[ 0U ], maxData[ 1U ], maxData[ 2U ] ),
        GXVec3 ( minData[ 0U ], maxData[ 1U ], maxData[ 2U ] )
    };

    bounds.Empty ();

    for ( auto const &v : verticesLocal )
    {
        GXVec3 vertex {};
        transform.MultiplyAsPoint ( vertex, v );
        bounds.AddVertex ( vertex );
    }
}

[[maybe_unused]] GXVoid GXAABB::AddVertex ( GXVec3 const &vertex ) noexcept
{
    auto const &vData = vertex._data;
    AddVertex ( vData[ 0U ], vData[ 1U ], vData[ 2U ] );
}

[[maybe_unused]] GXVoid GXAABB::AddVertex ( GXFloat x, GXFloat y, GXFloat z ) noexcept
{
    auto &minData = _min._data;
    auto &maxData = _max._data;

    if ( _vertices > 1U )
    {
        if ( minData[ 0U ] > x )
            minData[ 0U ] = x;
        else if ( maxData[ 0U ] < x )
            maxData[ 0U ] = x;

        if ( minData[ 1U ] > y )
            minData[ 1U ] = y;
        else if ( maxData[ 1U ] < y )
            maxData[ 1U ] = y;

        if ( minData[ 2U ] > z )
            minData[ 2U ] = z;
        else if ( maxData[ 2U ] < z )
            maxData[ 2U ] = z;

        return;
    }

    if ( _vertices == 0U )
    {
        ++_vertices;

        _min.Init ( x, y, z );
        return;
    }

    ++_vertices;

    if ( minData[ 0U ] > x )
    {
        maxData[ 0U ] = minData[ 0U ];
        minData[ 0U ] = x;
    }
    else
    {
        maxData[ 0U ] = x;
    }

    if ( minData[ 1U ] > y )
    {
        maxData[ 1U ] = minData[ 1U ];
        minData[ 1U ] = y;
    }
    else
    {
        maxData[ 1U ] = y;
    }

    if ( minData[ 2U ] <= z )
    {
        maxData[ 2U ] = z;
        return;
    }

    maxData[ 2U ] = minData[ 2U ];
    minData[ 2U ] = z;
}

[[maybe_unused]] GXVoid GXAABB::GetCenter ( GXVec3 &center ) const noexcept
{
    center.Sum ( _min, _max );
    center.Multiply ( center, 0.5F );
}

[[maybe_unused]] GXFloat GXAABB::GetWidth () const noexcept
{
    if ( _vertices < 2U )
        return -1.0F;

    return _max._data[ 0U ] - _min._data[ 0U ];
}

[[maybe_unused]] GXFloat GXAABB::GetHeight () const noexcept
{
    if ( _vertices < 2U )
        return -1.0F;

    return _max._data[ 1U ] - _min._data[ 1U ];
}

[[maybe_unused]] GXFloat GXAABB::GetDepth () const noexcept
{
    if ( _vertices < 2U )
        return -1.0F;

    return _max._data[ 2U ] - _min._data[ 2U ];
}

[[maybe_unused]] GXFloat GXAABB::GetSphereRadius () const noexcept
{
    GXVec3 center {};
    GetCenter ( center );
    return center.Distance ( _min );
}

//----------------------------------------------------------------------------------------------------------------------

[[maybe_unused]] GXVoid GXPlane::From ( GXVec3 const &pointA, GXVec3 const &pointB, GXVec3 const &pointC ) noexcept
{
    GXVec3 ab {};
    ab.Subtract ( pointB, pointA );

    GXVec3 ac {};
    ac.Subtract ( pointC, pointA );

    GXVec3 normal {};
    normal.CrossProduct ( ab, ac );
    normal.Normalize ();

    _a = normal._data[ 0U ];
    _b = normal._data[ 1U ];
    _c = normal._data[ 2U ];
    _d = -normal.DotProduct ( pointA );
}

[[maybe_unused]] GXVoid GXPlane::FromLineToPoint ( GXVec3 const &lineStart,
    GXVec3 const &lineEnd,
    GXVec3 const &point
) noexcept
{
    GXVec3 startToPoint {};
    startToPoint.Subtract ( point, lineStart );

    GXVec3 startToEnd {};
    startToEnd.Subtract ( lineEnd, lineStart );

    GXVec3 tempCross {};
    tempCross.CrossProduct ( startToEnd, startToPoint );

    GXVec3 normal {};
    normal.CrossProduct ( tempCross, startToEnd );

    _a = normal._data[ 0U ];
    _b = normal._data[ 1U ];
    _c = normal._data[ 2U ];
    _d = -_a * lineStart._data[ 0U ] - _b * lineStart._data[ 1U ] - _c * lineStart._data[ 2U ];

    if ( ClassifyVertex ( point ) != eGXPlaneClassifyVertex::Behind )
        return;

    Flip ();
}

[[maybe_unused]] GXVoid GXPlane::Normalize () noexcept
{
    GXFloat const inverseLength = 1.0F / sqrtf ( _a * _a + _b * _b + _c * _c );

    _a *= inverseLength;
    _b *= inverseLength;
    _c *= inverseLength;
    _d *= inverseLength;
}

[[maybe_unused]] GXVoid GXPlane::Flip () noexcept
{
    _a = -_a;
    _b = -_b;
    _c = -_c;
    _d = -_d;
}

[[maybe_unused]] eGXPlaneClassifyVertex GXPlane::ClassifyVertex ( GXVec3 const &vertex ) const noexcept
{
    return ClassifyVertex ( vertex._data[ 0U ], vertex._data[ 1U ], vertex._data[ 2U ] );
}

[[maybe_unused]] eGXPlaneClassifyVertex GXPlane::ClassifyVertex ( GXFloat x, GXFloat y, GXFloat z ) const noexcept
{
    GXFloat const test = _a * x + _b * y + _c * z + _d;

    if ( test < 0.0F )
        return eGXPlaneClassifyVertex::Behind;

    return test > 0.0F ? eGXPlaneClassifyVertex::InFront : eGXPlaneClassifyVertex::On;
}

//----------------------------------------------------------------------------------------------------------------------

// NOLINTNEXTLINE
[[maybe_unused]] GXProjectionClipPlanes::GXProjectionClipPlanes ( GXMat4 const &src ) noexcept
{
    From ( src );
}

[[maybe_unused]] GXVoid GXProjectionClipPlanes::From ( GXMat4 const &src ) noexcept
{
    auto const &m = src._data;
    auto &planes = _planes;

    // Left clipping plane
    GXPlane &p0 = planes[ 0U ];
    p0._a = m[ 0U ][ 3U ] + m[ 0U ][ 0U ];
    p0._b = m[ 1U ][ 3U ] + m[ 1U ][ 0U ];
    p0._c = m[ 2U ][ 3U ] + m[ 2U ][ 0U ];
    p0._d = m[ 3U ][ 3U ] + m[ 3U ][ 0U ];

    // Right clipping plane
    GXPlane &p1 = planes[ 1U ];
    p1._a = m[ 0U ][ 3U ] - m[ 0U ][ 0U ];
    p1._b = m[ 1U ][ 3U ] - m[ 1U ][ 0U ];
    p1._c = m[ 2U ][ 3U ] - m[ 2U ][ 0U ];
    p1._d = m[ 3U ][ 3U ] - m[ 3U ][ 0U ];

    // Top clipping plane
    GXPlane &p2 = planes[ 2U ];
    p2._a = m[ 0U ][ 3U ] - m[ 0U ][ 1U ];
    p2._b = m[ 1U ][ 3U ] - m[ 1U ][ 1U ];
    p2._c = m[ 2U ][ 3U ] - m[ 2U ][ 1U ];
    p2._d = m[ 3U ][ 3U ] - m[ 3U ][ 1U ];

    // Bottom clipping plane
    GXPlane &p3 = planes[ 3U ];
    p3._a = m[ 0U ][ 3U ] + m[ 0U ][ 1U ];
    p3._b = m[ 1U ][ 3U ] + m[ 1U ][ 1U ];
    p3._c = m[ 2U ][ 3U ] + m[ 2U ][ 1U ];
    p3._d = m[ 3U ][ 3U ] + m[ 3U ][ 1U ];

    // Near clipping plane
    GXPlane &p4 = planes[ 4U ];
    p4._a = m[ 0U ][ 3U ] + m[ 0U ][ 2U ];
    p4._b = m[ 1U ][ 3U ] + m[ 1U ][ 2U ];
    p4._c = m[ 2U ][ 3U ] + m[ 2U ][ 2U ];
    p4._d = m[ 3U ][ 3U ] + m[ 3U ][ 2U ];

    // Far clipping plane
    GXPlane &p5 = planes[ 5U ];
    p5._a = m[ 0U ][ 3U ] - m[ 0U ][ 2U ];
    p5._b = m[ 1U ][ 3U ] - m[ 1U ][ 2U ];
    p5._c = m[ 2U ][ 3U ] - m[ 2U ][ 2U ];
    p5._d = m[ 3U ][ 3U ] - m[ 3U ][ 2U ];
}

[[maybe_unused]] GXBool GXProjectionClipPlanes::IsVisible ( GXAABB const &bounds ) const noexcept
{
    auto const &minData = bounds._min._data;
    auto const &maxData = bounds._max._data;

    GXUByte flags = PlaneTest ( minData[ 0U ], minData[ 1U ], minData[ 2U ] );
    flags &= PlaneTest ( minData[ 0U ], maxData[ 1U ], minData[ 2U ] );
    flags &= PlaneTest ( maxData[ 0U ], maxData[ 1U ], minData[ 2U ] );
    flags &= PlaneTest ( maxData[ 0U ], minData[ 1U ], minData[ 2U ] );

    flags &= PlaneTest ( minData[ 0U ], minData[ 1U ], maxData[ 2U ] );
    flags &= PlaneTest ( minData[ 0U ], maxData[ 1U ], maxData[ 2U ] );
    flags &= PlaneTest ( maxData[ 0U ], maxData[ 1U ], maxData[ 2U ] );
    flags &= PlaneTest ( maxData[ 0U ], minData[ 1U ], maxData[ 2U ] );

    return flags == 0U;
}

[[maybe_unused]] GXUByte GXProjectionClipPlanes::PlaneTest ( GXFloat x, GXFloat y, GXFloat z ) const noexcept
{
    constexpr GXUByte const masks[] =
    {
        0b00000001U,
        0b00000010U,
        0b00000100U,
        0b00001000U,
        0b00010000U,
        0b00100000U
    };

    GXUByte result = 0U;
    auto const &planes = _planes;
    constexpr GXUPointer count = std::size ( masks );

    for ( GXUPointer i = 0U; i < count; ++i )
    {
        if ( planes[ i ].ClassifyVertex ( x, y, z ) != eGXPlaneClassifyVertex::Behind )
            continue;

        result |= masks[ i ];
    }

    return result;
}

//----------------------------------------------------------------------------------------------------------------------

[[maybe_unused]] GXFloat GXCALL GXDegToRad ( GXFloat degrees ) noexcept
{
    return degrees * DEGREES_TO_RADIANS_FACTOR;
}

[[maybe_unused]] GXFloat GXCALL GXRadToDeg ( GXFloat radians ) noexcept
{
    return radians * RADIANS_TO_DEGREES_FACTOR;
}

[[maybe_unused]] GXVoid GXCALL GXRandomize () noexcept
{
    // NOLINTNEXTLINE - do not use std::srand
    std::srand ( static_cast<GXUInt> ( time ( nullptr ) ) );
}

[[maybe_unused]] GXFloat GXCALL GXRandomNormalize () noexcept
{
    // NOLINTNEXTLINE - do not use std::rand
    return static_cast<GXFloat> ( std::rand () ) * INVERSE_RAND_MAX;
}

[[maybe_unused]] GXFloat GXCALL GXRandomBetween ( GXFloat from, GXFloat to ) noexcept
{
    GXFloat const delta = to - from;
    return from + delta * GXRandomNormalize ();
}

[[maybe_unused]] GXVoid GXCALL GXRandomBetween ( GXVec3 &out, const GXVec3 &from, const GXVec3 &to ) noexcept
{
    auto const &fData = from._data;
    auto const &tData = to._data;
    auto &result = out._data;

    result[ 0U ] = GXRandomBetween ( fData[ 0U ], tData[ 0U ] );
    result[ 1U ] = GXRandomBetween ( fData[ 1U ], tData[ 1U ] );
    result[ 2U ] = GXRandomBetween ( fData[ 2U ], tData[ 2U ] );
}

[[maybe_unused]] GXVoid GXCALL GXGetTangentBitangent ( GXVec3 &outTangent,
    GXVec3 &outBitangent,
    GXUByte vertexID,
    GXUByte const* vertices,
    GXUPointer vertexStride,
    GXUByte const* uvs,
    GXUPointer uvStride
) noexcept
{
    GXVec3 const* v0;
    GXVec3 const* v1;
    GXVec3 const* v2;

    GXVec2 const* uv0;
    GXVec2 const* uv1;
    GXVec2 const* uv2;

    switch ( vertexID )
    {
        case 0U:
        {
            v0 = reinterpret_cast<GXVec3 const*> ( vertices );
            v1 = reinterpret_cast<GXVec3 const*> ( vertices + vertexStride );
            v2 = reinterpret_cast<GXVec3 const*> ( vertices + 2U * vertexStride );

            uv0 = reinterpret_cast<GXVec2 const*> ( uvs );
            uv1 = reinterpret_cast<GXVec2 const*> ( uvs + uvStride );
            uv2 = reinterpret_cast<GXVec2 const*> ( uvs + 2U * uvStride );
        }
        break;

        case 1U:
        {
            v0 = reinterpret_cast<GXVec3 const*> ( vertices + vertexStride );
            v1 = reinterpret_cast<GXVec3 const*> ( vertices + 2U * vertexStride );
            v2 = reinterpret_cast<GXVec3 const*> ( vertices );

            uv0 = reinterpret_cast<GXVec2 const*> ( uvs + uvStride );
            uv1 = reinterpret_cast<GXVec2 const*> ( uvs + 2U * uvStride );
            uv2 = reinterpret_cast<GXVec2 const*> ( uvs );
        }
        break;

        case 2U:
        default:
        {
            v0 = reinterpret_cast<GXVec3 const*> ( vertices + 2U * vertexStride );
            v1 = reinterpret_cast<GXVec3 const*> ( vertices );
            v2 = reinterpret_cast<GXVec3 const*> ( vertices + vertexStride );

            uv0 = reinterpret_cast<GXVec2 const*> ( uvs + 2U * uvStride );
            uv1 = reinterpret_cast<GXVec2 const*> ( uvs );
            uv2 = reinterpret_cast<GXVec2 const*> ( uvs + uvStride );
        }
        break;
    }

    GXVec3 a {};
    GXVec3 b {};
    a.Subtract ( *v1, *v0 );
    b.Subtract ( *v2, *v0 );

    GXVec2 dUVa {};
    GXVec2 dUVb {};
    dUVa.Subtract ( *uv1, *uv0 );
    dUVb.Subtract ( *uv2, *uv0 );

    GXFloat const factor = 1.0F / ( dUVa._data[ 0U ] * dUVb._data[ 1U ] - dUVb._data[ 0U ] * dUVa._data[ 1U ] );

    auto &tData = outTangent._data;
    tData[ 0U ] = factor * ( dUVb._data[ 1U ] * a._data[ 0U ] - dUVa._data[ 1U ] * b._data[ 0U ] );
    tData[ 1U ] = factor * ( dUVb._data[ 1U ] * a._data[ 1U ] - dUVa._data[ 1U ] * b._data[ 1U ] );
    tData[ 2U ] = factor * ( dUVb._data[ 1U ] * a._data[ 2U ] - dUVa._data[ 1U ] * b._data[ 2U ] );
    outTangent.Normalize ();

    auto &bData = outBitangent._data;
    bData[ 0U ] = factor * ( -dUVb._data[ 0U ] * a._data[ 0U ] + dUVa._data[ 0U ] * b._data[ 0U ] );
    bData[ 1U ] = factor * ( -dUVb._data[ 0U ] * a._data[ 1U ] + dUVa._data[ 0U ] * b._data[ 1U ] );
    bData[ 2U ] = factor * ( -dUVb._data[ 0U ] * a._data[ 2U ] + dUVa._data[ 0U ] * b._data[ 2U ] );
    outBitangent.Normalize ();
}

[[maybe_unused]] GXFloat GXCALL GXClampf ( GXFloat value, GXFloat minValue, GXFloat maxValue ) noexcept
{
    return ( value < minValue ) ? minValue : ( value > maxValue ) ? maxValue : value;
}

[[maybe_unused]] GXInt GXCALL GXClampi ( GXInt value, GXInt minValue, GXInt maxValue ) noexcept
{
    return ( value < minValue ) ? minValue : ( value > maxValue ) ? maxValue : value;
}

[[maybe_unused]] GXFloat GXCALL GXMinf ( GXFloat a, GXFloat b ) noexcept
{
    return a < b ? a : b;
}

[[maybe_unused]] GXFloat GXCALL GXMaxf ( GXFloat a, GXFloat b ) noexcept
{
    return a > b ? a : b;
}

[[maybe_unused]] GXVoid GXCALL GXGetBarycentricCoords ( GXVec3 &out,
    GXVec3 const &point,
    GXVec3 const &aPivot,
    GXVec3 const &bPivot,
    GXVec3 const &cPivot
) noexcept
{
    GXVec3 v0 {};
    GXVec3 v1 {};
    GXVec3 v2 {};

    v0.Subtract ( bPivot, aPivot );
    v1.Subtract ( cPivot, aPivot );
    v2.Subtract ( point, aPivot );

    GXFloat const d00 = v0.DotProduct ( v0 );
    GXFloat const d01 = v0.DotProduct ( v1 );
    GXFloat const d11 = v1.DotProduct ( v1 );
    GXFloat const d20 = v2.DotProduct ( v0 );
    GXFloat const d21 = v2.DotProduct ( v1 );

    GXFloat const denom = 1.0F / ( d00 * d11 - d01 * d01 );

    auto &result = out._data;

    result[ 1U ] = ( d11 * d20 - d01 * d21 ) * denom;
    result[ 2U ] = ( d00 * d21 - d01 * d20 ) * denom;
    result[ 0U ] = 1.0F - result[ 1U ] - result[ 2U ];
}

[[maybe_unused]] GXVoid GXCALL GXGetRayFromViewer ( GXVec3 &origin,
    GXVec3 &direction,
    GXUShort x,
    GXUShort y,
    GXUShort viewportWidth,
    GXUShort viewportHeight,
    GXVec3 const &viewerLocation,
    GXMat4 const &viewProjectionMatrix
) noexcept
{
    GXFloat const halfWidth = static_cast<GXFloat> ( viewportWidth ) * 0.5F;
    GXFloat const halfHeight = static_cast<GXFloat> ( viewportHeight ) * 0.5F;

    GXVec4 const pointCVV
    {
        ( static_cast<GXFloat> ( x ) - halfWidth ) / halfWidth,
        ( static_cast<GXFloat> ( y ) - halfHeight ) / halfHeight,
        1.0F,
        1.0F
    };

    GXMat4 inverseViewProjectionMatrix {};
    inverseViewProjectionMatrix.Inverse ( viewProjectionMatrix );

    GXVec4 pointWorld {};
    inverseViewProjectionMatrix.MultiplyVectorMatrix ( pointWorld, pointCVV );
    GXFloat alpha = 1.0F / pointWorld._data[ 3U ];

    pointWorld._data[ 0U ] *= alpha;
    pointWorld._data[ 1U ] *= alpha;
    pointWorld._data[ 2U ] *= alpha;

    direction.Subtract ( GXVec3 ( pointWorld._data[ 0U ], pointWorld._data[ 1U ], pointWorld._data[ 2U ] ),
        viewerLocation
    );

    direction.Normalize ();
    origin = viewerLocation;
}
