// version 1.57

#include <GXCommon/GXMath.h>
#include <GXCommon/GXWarning.h>

GX_DISABLE_COMMON_WARNINGS

#include <cassert>
#include <cstring>
#include <cstdlib>
#include <ctime>

GX_RESTORE_WARNING_STATE


constexpr static GXFloat const COLOR_TO_FLOAT_FACTOR = 0.00392157F;

constexpr static GXFloat const HSVA_FACTOR = 0.016666F;
constexpr static GXFloat const HSVA_TO_RGBA_FLOAT = 0.01F;
constexpr static GXFloat const RGBA_TO_UBYTE_FACTOR = 255.0F;

constexpr static GXFloat const DEGREES_TO_RADIANS_FACTOR = 0.0174533F;
constexpr static GXFloat const RADIANS_TO_DEGREES_FACTOR = 57.295779F;

constexpr static GXFloat const FLOAT_EPSILON = 1.0e-4F;
constexpr static GXFloat const INVERSE_RAND_MAX = 3.05185e-5F;

constexpr static GXUByte const SOLUTION_ALPHA = 0U;
constexpr static GXUByte const SOLUTION_BETTA = 1U;
constexpr static GXUByte const SOLUTION_GAMMA = 2U;
constexpr static GXUByte const SOLUTION_YOTTA = 3U;
constexpr static GXUByte const UNKNOWN_SOLUTION = 0xFFU;

//----------------------------------------------------------------------------------------------------------------------

[[maybe_unused]] GXVoid GXVec2::SetX ( GXFloat x )
{
    _data[ 0U ] = x;
}

[[maybe_unused]] GXFloat GXVec2::GetX () const
{
    return _data[ 0U ];
}

[[maybe_unused]] GXVoid GXVec2::SetY ( GXFloat y )
{
    _data[ 1U ] = y;
}

[[maybe_unused]] GXFloat GXVec2::GetY () const
{
    return _data[ 1U ];
}

[[maybe_unused]] GXVoid GXVec2::Init ( GXFloat x, GXFloat y )
{
    _data[ 0U ] = x;
    _data[ 1U ] = y;
}

[[maybe_unused]] GXVoid GXVec2::Normalize ()
{
    Multiply ( *this, 1.0F / Length () );
}

[[maybe_unused]] GXVoid GXVec2::CalculateNormalFast ( GXVec2 const &a, GXVec2 const &b )
{
    _data[ 0U ] = a._data[ 1U ] - b._data[ 1U ];
    _data[ 1U ] = b._data[ 0U ] - a._data[ 0U ];
}

[[maybe_unused]] GXVoid GXVec2::CalculateNormal ( GXVec2 const &a, GXVec2 const &b )
{
    CalculateNormalFast ( a, b );
    Normalize ();
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

[[maybe_unused]] GXVoid GXVec2::Substract ( GXVec2 const &a, GXVec2 const &b )
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

[[maybe_unused]] GXFloat GXVec2::DotProduct ( GXVec2 const &other ) const
{
    return _data[ 0U ] * other._data[ 0U ] + _data[ 1U ] * other._data[ 1U ];
}

[[maybe_unused]] GXFloat GXVec2::Length () const
{
    return sqrtf ( SquaredLength () );
}

[[maybe_unused]] GXFloat GXVec2::SquaredLength () const
{
    return DotProduct ( *this );
}

[[maybe_unused]] GXBool GXVec2::IsEqual ( GXVec2 const &other ) const
{
    if ( _data[ 0U ] != other._data[ 0U ] )
        return GX_FALSE;

    return _data[ 1U ] == other._data[ 1U ];
}

//----------------------------------------------------------------------------------------------------------------------

[[maybe_unused]] eGXLineRelationship GXCALL GXLineIntersection2D ( GXVec2 &intersectionPoint,
    const GXVec2 &a0,
    const GXVec2 &a1,
    const GXVec2 &b0,
    const GXVec2 &b1
)
{
    GXVec2 alpha;
    alpha.Substract ( a1, a0 );

    GXVec2 betta;
    betta.Substract ( b1, b0 );

    GXVec2 const yotta ( -alpha.GetY (), alpha.GetX () );
    GXFloat const omega = yotta.DotProduct ( betta );

    if ( omega == 0.0F )
    {
        GXVec2 const gamma ( a0.IsEqual ( b0 ) ? b1 : b0 );

        GXVec2 zetta;
        zetta.Substract ( gamma, a0 );
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

    GXVec2 phi;
    phi.Substract ( b0, a0 );

    intersectionPoint.Sum ( b0, -phi.DotProduct ( yotta ) / omega, betta );
    return eGXLineRelationship::Intersection;
}

//----------------------------------------------------------------------------------------------------------------------

[[maybe_unused]] GXVoid GXVec3::SetX ( GXFloat x )
{
    _data[ 0U ] = x;
}

[[maybe_unused]] GXFloat GXVec3::GetX () const
{
    return _data[ 0U ];
}

[[maybe_unused]] GXVoid GXVec3::SetY ( GXFloat y )
{
    _data[ 1U ] = y;
}

[[maybe_unused]] GXFloat GXVec3::GetY () const
{
    return _data[ 1U ];
}

[[maybe_unused]] GXVoid GXVec3::SetZ ( GXFloat z )
{
    _data[ 2U ] = z;
}

[[maybe_unused]] GXFloat GXVec3::GetZ () const
{
    return _data[ 2U ];
}

[[maybe_unused]] GXVoid GXVec3::Init ( GXFloat x, GXFloat y, GXFloat z )
{
    _data[ 0U ] = x;
    _data[ 1U ] = y;
    _data[ 2U ] = z;
}

[[maybe_unused]] GXVoid GXVec3::Normalize ()
{
    Multiply ( *this, 1.0F / Length () );
}

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

[[maybe_unused]] GXVoid GXVec3::Substract ( GXVec3 const &a, GXVec3 const &b )
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

[[maybe_unused]] GXFloat GXVec3::Length () const
{
    return sqrtf ( DotProduct ( *this ) );
}

[[maybe_unused]] GXFloat GXVec3::SquaredLength () const
{
    return DotProduct ( *this );
}

[[maybe_unused]] GXFloat GXVec3::Distance ( GXVec3 const &other ) const
{
    GXVec3 difference;
    difference.Substract ( *this, other );
    return difference.Length ();
}

[[maybe_unused]] GXFloat GXVec3::SquaredDistance ( GXVec3 const &other ) const
{
    GXVec3 difference;
    difference.Substract ( *this, other );
    return difference.SquaredLength ();
}

[[maybe_unused]] GXVoid GXVec3::LinearInterpolation ( GXVec3 const &start,
    GXVec3 const &finish,
    GXFloat interpolationFactor
)
{
    GXVec3 difference;
    difference.Substract ( finish, start );
    Sum ( start, interpolationFactor, difference );
}

[[maybe_unused]] GXVoid GXVec3::Project ( GXVec3 const &vector, GXVec3 const &axis )
{
    GXVec3 normalVector ( vector );
    normalVector.Normalize ();
    GXFloat factor = vector.Length () * axis.DotProduct ( normalVector );
    Multiply ( axis, factor );
}

[[maybe_unused]] GXBool GXVec3::IsEqual ( GXVec3 const &other )
{
    if ( _data[ 0U ] != other._data[ 0U ] )
        return GX_FALSE;

    if ( _data[ 1U ] != other._data[ 1U ] )
        return GX_FALSE;

    return _data[ 2U ] == other._data[ 2U ];
}

[[maybe_unused]] GXVec3 const& GXVec3::GetAbsoluteX ()
{
    constexpr static GXVec3 const absoluteX ( 1.0F, 0.0F, 0.0F );
    return absoluteX;
}

[[maybe_unused]] GXVec3 const& GXVec3::GetAbsoluteY ()
{
    constexpr static GXVec3 const absoluteY ( 0.0F, 1.0F, 0.0F );
    return absoluteY;
}

[[maybe_unused]] GXVec3 const& GXVec3::GetAbsoluteZ ()
{
    constexpr static GXVec3 const absoluteZ ( 0.0F, 0.0F, 1.0F );
    return absoluteZ;
}

[[maybe_unused]] GXVoid GXCALL GXVec3::MakeOrthonormalBasis ( GXVec3 &baseX, GXVec3 &adjustedY, GXVec3 &adjustedZ )
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
)
{
    // Implementation https://graphics.stanford.edu/courses/cs348b-98/gg/intersect.html

    GXPlane plane;
    plane.From ( a, b, c );
    GXVec3 const normal ( plane._a, plane._b, plane._c );

    GXFloat const t = ( plane._d + normal.DotProduct ( origin ) ) / ( -normal.DotProduct ( direction ) );

    if ( t < 0.0F )
        return GX_FALSE;

    if ( t > length )
        return GX_FALSE;

    GXVec3 const triangle[ 3U ] = { a, b, c };

    GXVec3 point;
    point.Sum ( origin, t, direction );

    GXUByte selector = 0U;

    GXFloat gamma = fabsf ( plane._a );
    GXFloat const omega = fabsf ( plane._b );
    GXFloat const yotta = fabsf ( plane._c );

    if ( gamma > omega )
        selector = gamma > yotta ? 0U : 2U;
    else
        selector = omega > yotta ? 1U : 2U;

    GXUByte i1 = 0xFFU;
    GXUByte i2 = 0xFFU;

    switch ( selector )
    {
        case 0u:
            i1 = 1u;
            i2 = 2u;
        break;

        case 1u:
            i1 = 2u;
            i2 = 0u;
        break;

        case 2u:
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
    GXFloat alpha = ( u0 * v2 - v0 * u2 ) * gamma;

    if ( alpha < 0.0F || alpha > 1.0F )
        return GX_FALSE;

    GXFloat const betta = ( u1 * v0 - v1 * u0 ) * gamma;

    if ( betta < 0.0F || betta > 1.0F || ( alpha + betta ) > 1.0F )
        return GX_FALSE;

    outT = t;
    return GX_TRUE;
}

//----------------------------------------------------------------------------------------------------------------------

[[maybe_unused]] GXVec4::GXVec4 ( GXVec3 const &vector, GXFloat w ):
    _data { vector._data[ 0U ], vector._data[ 1U ], vector._data[ 2U ], w }
{
    // NOTHING
}

[[maybe_unused]] GXVoid GXVec4::Init ( GXFloat x, GXFloat y, GXFloat z, GXFloat w )
{
    _data[ 0U ] = x;
    _data[ 1U ] = y;
    _data[ 2U ] = z;
    _data[ 3U ] = w;
}

[[maybe_unused]] GXVoid GXVec4::SetX ( GXFloat x )
{
    _data[ 0U ] = x;
}

[[maybe_unused]] GXFloat GXVec4::GetX () const
{
    return _data[ 0U ];
}

[[maybe_unused]] GXVoid GXVec4::SetY ( GXFloat y )
{
    _data[ 1U ] = y;
}

[[maybe_unused]] GXFloat GXVec4::GetY () const
{
    return _data[ 1U ];
}

[[maybe_unused]] GXVoid GXVec4::SetZ ( GXFloat z )
{
    _data[ 2U ] = z;
}

[[maybe_unused]] GXFloat GXVec4::GetZ () const
{
    return _data[ 2U ];
}

[[maybe_unused]] GXVoid GXVec4::SetW ( GXFloat w )
{
    _data[ 3U ] = w;
}

[[maybe_unused]] GXFloat GXVec4::GetW () const
{
    return _data[ 3U ];
}

[[maybe_unused]] GXVoid GXVec4::Sum ( GXVec4 const &a, GXVec4 const &b )
{
    _data[ 0U ] = a._data[ 0U ] + b._data[ 0U ];
    _data[ 1U ] = a._data[ 1U ] + b._data[ 1U ];
    _data[ 2U ] = a._data[ 2U ] + b._data[ 2U ];
    _data[ 3U ] = a._data[ 3U ] + b._data[ 3U ];
}

[[maybe_unused]] GXVoid GXVec4::Sum ( GXVec4 const &a, GXFloat bScale, GXVec4 const &b )
{
    _data[ 0U ] = a._data[ 0U ] + bScale * b._data[ 0U ];
    _data[ 1U ] = a._data[ 1U ] + bScale * b._data[ 1U ];
    _data[ 2U ] = a._data[ 2U ] + bScale * b._data[ 2U ];
    _data[ 3U ] = a._data[ 3U ] + bScale * b._data[ 3U ];
}

[[maybe_unused]] GXVoid GXVec4::Substract ( GXVec4 const &a, GXVec4 const &b )
{
    _data[ 0U ] = a._data[ 0U ] - b._data[ 0U ];
    _data[ 1U ] = a._data[ 1U ] - b._data[ 1U ];
    _data[ 2U ] = a._data[ 2U ] - b._data[ 2U ];
    _data[ 3U ] = a._data[ 3U ] - b._data[ 3U ];
}

[[maybe_unused]] GXFloat GXVec4::DotProduct ( GXVec4 const &other ) const
{
    return _data[ 0U ] * other._data[ 0U ] +
        _data[ 1U ] * other._data[ 1U ] +
        _data[ 2U ] * other._data[ 2U ] +
        _data[ 3U ] * other._data[ 3U ];
}

[[maybe_unused]] GXFloat GXVec4::Length () const
{
    return sqrtf ( DotProduct ( *this ) );
}

[[maybe_unused]] GXFloat GXVec4::SquaredLength () const
{
    return DotProduct ( *this );
}

//----------------------------------------------------------------------------------------------------------------------

[[maybe_unused]] GXVoid GXVec6::Init ( GXFloat a1, GXFloat a2, GXFloat a3, GXFloat a4, GXFloat a5, GXFloat a6 )
{
    _data[ 0U ] = a1;
    _data[ 1U ] = a2;
    _data[ 2U ] = a3;
    _data[ 3U ] = a4;
    _data[ 4U ] = a5;
    _data[ 5U ] = a6;
}

[[maybe_unused]] GXVoid GXVec6::From ( GXVec3 const &v1, GXVec3 const &v2 )
{
    memcpy ( _data, &v1, sizeof ( GXVec3 ) );
    memcpy ( _data + 3U, &v2, sizeof ( GXVec3 ) );
}

[[maybe_unused]] GXFloat GXVec6::DotProduct ( GXVec6 const &other ) const
{
    return _data[ 0U ] * other._data[ 0U ] +
        _data[ 1U ] * other._data[ 1U ] +
        _data[ 2U ] * other._data[ 2U ] +
        _data[ 3U ] * other._data[ 3U ] +
        _data[ 4U ] * other._data[ 4U ] +
        _data[ 5U ] * other._data[ 5U ];
}

[[maybe_unused]] GXVoid GXVec6::Sum ( GXVec6 const &a, GXVec6 const &b )
{
    _data[ 0U ] = a._data[ 0U ] + b._data[ 0U ];
    _data[ 1U ] = a._data[ 1U ] + b._data[ 1U ];
    _data[ 2U ] = a._data[ 2U ] + b._data[ 2U ];
    _data[ 3U ] = a._data[ 3U ] + b._data[ 3U ];
    _data[ 4U ] = a._data[ 4U ] + b._data[ 4U ];
    _data[ 5U ] = a._data[ 5U ] + b._data[ 5U ];
}

[[maybe_unused]] GXVoid GXVec6::Sum ( GXVec6 const &a, GXFloat bScale, GXVec6 const &b )
{
    _data[ 0U ] = a._data[ 0U ] + bScale * b._data[ 0U ];
    _data[ 1U ] = a._data[ 1U ] + bScale * b._data[ 1U ];
    _data[ 2U ] = a._data[ 2U ] + bScale * b._data[ 2U ];
    _data[ 3U ] = a._data[ 3U ] + bScale * b._data[ 3U ];
    _data[ 4U ] = a._data[ 4U ] + bScale * b._data[ 4U ];
    _data[ 5U ] = a._data[ 5U ] + bScale * b._data[ 5U ];
}

[[maybe_unused]] GXVoid GXVec6::Multiply ( GXVec6 const &a, GXFloat factor )
{
    _data[ 0U ] = a._data[ 0U ] * factor;
    _data[ 1U ] = a._data[ 1U ] * factor;
    _data[ 2U ] = a._data[ 2U ] * factor;
    _data[ 3U ] = a._data[ 3U ] * factor;
    _data[ 4U ] = a._data[ 4U ] * factor;
    _data[ 5U ] = a._data[ 5U ] * factor;
}

//----------------------------------------------------------------------------------------------------------------------

[[maybe_unused]] GXColorRGB::GXColorRGB ( GXUByte red, GXUByte green, GXUByte blue, GXFloat alpha )
{
    From ( red, green, blue, alpha );
}

[[maybe_unused]] GXColorRGB::GXColorRGB ( const GXColorHSV &color )
{
    From ( color );
}

[[maybe_unused]] GXVoid GXColorRGB::Init ( GXFloat red, GXFloat green, GXFloat blue, GXFloat alpha )
{
    _data[ 0U ] = red;
    _data[ 1U ] = green;
    _data[ 2U ] = blue;
    _data[ 3U ] = alpha;
}

[[maybe_unused]] GXVoid GXColorRGB::SetRed ( GXFloat red )
{
    _data[ 0U ] = red;
}

[[maybe_unused]] GXFloat GXColorRGB::GetRed () const
{
    return _data[ 0U ];
}

[[maybe_unused]] GXVoid GXColorRGB::SetGreen ( GXFloat green )
{
    _data[ 1U ] = green;
}

[[maybe_unused]] GXFloat GXColorRGB::GetGreen () const
{
    return _data[ 1U ];
}

[[maybe_unused]] GXVoid GXColorRGB::SetBlue ( GXFloat blue )
{
    _data[ 2U ] = blue;
}

[[maybe_unused]] GXFloat GXColorRGB::GetBlue () const
{
    return _data[ 2U ];
}

[[maybe_unused]] GXVoid GXColorRGB::SetAlpha ( GXFloat alpha )
{
    _data[ 3U ] = alpha;
}

[[maybe_unused]] GXFloat GXColorRGB::GetAlpha () const
{
    return _data[ 3U ];
}

[[maybe_unused]] GXVoid GXColorRGB::From ( GXUByte red, GXUByte green, GXUByte blue, GXFloat alpha )
{
    _data[ 0U ] = static_cast<GXFloat> ( red ) * COLOR_TO_FLOAT_FACTOR;
    _data[ 1U ] = static_cast<GXFloat> ( green ) * COLOR_TO_FLOAT_FACTOR;
    _data[ 2U ] = static_cast<GXFloat> ( blue ) * COLOR_TO_FLOAT_FACTOR;
    _data[ 3U ] = alpha * COLOR_TO_FLOAT_FACTOR;
}

[[maybe_unused]] GXVoid GXColorRGB::From ( const GXColorHSV &color )
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

    _data[ 3 ] = color.GetAlpha () * HSVA_TO_RGBA_FLOAT;

    switch ( selector )
    {
        case 0U:
            _data[ 0U ] = value * HSVA_TO_RGBA_FLOAT;
            _data[ 1U ] = increment * HSVA_TO_RGBA_FLOAT;
            _data[ 2U ] = minValue * HSVA_TO_RGBA_FLOAT;
        break;

        case 1U:
            _data[ 0U ] = decrement * HSVA_TO_RGBA_FLOAT;
            _data[ 1U ] = value * HSVA_TO_RGBA_FLOAT;
            _data[ 2U ] = minValue * HSVA_TO_RGBA_FLOAT;
        break;

        case 2U:
            _data[ 0U ] = minValue * HSVA_TO_RGBA_FLOAT;
            _data[ 1U ] = value * HSVA_TO_RGBA_FLOAT;
            _data[ 2U ] = increment * HSVA_TO_RGBA_FLOAT;
        break;

        case 3U:
            _data[ 0U ] = minValue * HSVA_TO_RGBA_FLOAT;
            _data[ 1U ] = decrement * HSVA_TO_RGBA_FLOAT;
            _data[ 2U ] = value * HSVA_TO_RGBA_FLOAT;
        break;

        case 4U:
            _data[ 0U ] = increment * HSVA_TO_RGBA_FLOAT;
            _data[ 1U ] = minValue * HSVA_TO_RGBA_FLOAT;
            _data[ 2U ] = value * HSVA_TO_RGBA_FLOAT;
        break;

        case 5U:
            _data[ 0U ] = value * HSVA_TO_RGBA_FLOAT;
            _data[ 1U ] = minValue * HSVA_TO_RGBA_FLOAT;
            _data[ 2U ] = decrement * HSVA_TO_RGBA_FLOAT;
        break;

        default:
            assert ( !"GXColorRGB::From - Something wrong!" );
        break;
    }
}

[[maybe_unused]] GXVoid GXColorRGB::ConvertToUByte ( GXUByte &red, GXUByte &green, GXUByte &blue, GXUByte &alpha ) const
{
    red = static_cast<GXUByte> ( std::lround ( _data[ 0U ] * RGBA_TO_UBYTE_FACTOR ) );
    green = static_cast<GXUByte> ( std::lround ( _data[ 1U ] * RGBA_TO_UBYTE_FACTOR ) );
    blue = static_cast<GXUByte> ( std::lround ( _data[ 2U ] * RGBA_TO_UBYTE_FACTOR ) );
    alpha = static_cast<GXUByte> ( std::lround ( _data[ 3U ] * RGBA_TO_UBYTE_FACTOR ) );
}

//----------------------------------------------------------------------------------------------------------------------

[[maybe_unused]] GXColorHSV::GXColorHSV ( GXColorRGB const &color )
{
    From ( color );
}

[[maybe_unused]] GXVoid GXColorHSV::SetHue ( GXFloat hue )
{
    _data[ 0U ] = hue;
}

[[maybe_unused]] GXFloat GXColorHSV::GetHue () const
{
    return _data[ 0U ];
}

[[maybe_unused]] GXVoid GXColorHSV::SetSaturation ( GXFloat saturation )
{
    _data[ 1U ] = saturation;
}

[[maybe_unused]] GXFloat GXColorHSV::GetSaturation () const
{
    return _data[ 1U ];
}

[[maybe_unused]] GXVoid GXColorHSV::SetValue ( GXFloat value )
{
    _data[ 2U ] = value;
}

[[maybe_unused]] GXFloat GXColorHSV::GetValue () const
{
    return _data[ 2U ];
}

[[maybe_unused]] GXVoid GXColorHSV::SetAlpha ( GXFloat alpha )
{
    _data[ 3U ] = alpha;
}

[[maybe_unused]] GXFloat GXColorHSV::GetAlpha () const
{
    return _data[ 3U ];
}

[[maybe_unused]] GXVoid GXColorHSV::From ( GXColorRGB const &color )
{
    GXFloat const maxValue = GXMaxf ( GXMaxf ( color.GetRed (), color.GetGreen () ), color.GetBlue () );
    GXFloat const minValue = GXMinf ( GXMinf ( color.GetRed (), color.GetGreen () ), color.GetBlue () );

    if ( maxValue == minValue )
    {
        _data[ 0U ] = 0.0F;
    }
    else if ( maxValue == color.GetRed () && color.GetGreen () >= color.GetBlue () )
    {
        _data[ 0U ] = 60.0F * ( ( color.GetGreen () - color.GetBlue () ) / ( maxValue - minValue ) );
    }
    else if ( maxValue == color.GetRed () && color.GetGreen () < color.GetBlue () )
    {
        _data[ 0U ] = 60.0F * ( ( color.GetGreen () - color.GetBlue () ) / ( maxValue - minValue ) ) + 360.0F;
    }
    else if ( maxValue == color.GetGreen () )
    {
        _data[ 0U ] = 60.0F * ( ( color.GetBlue () - color.GetRed () ) / ( maxValue - minValue ) ) + 120.0F;
    }
    else if ( maxValue == color.GetBlue () )
    {
        _data[ 0U ] = 60.0F * ( ( color.GetRed () - color.GetGreen () ) / ( maxValue - minValue ) ) + 240.0F;
    }
    else
    {
        assert ( !"GXColorHSV::From - Something wrong!" );
    }

    if ( maxValue == 0.0F )
        _data[ 1U ] = 0.0F;
    else
        _data[ 1U ] = 100.0F * ( 1.0F - minValue / maxValue );

    _data[ 2U ] = 100.0F * maxValue;
    _data[ 3U ] = 100.0F * color.GetAlpha ();
}

//---------------------------------------------------------------------------------------------------------------------

[[maybe_unused]] GXVoid GXPreciseComplex::Init ( GXDouble real, GXDouble imaginary )
{
    _r = real;
    _i = imaginary;
}

[[maybe_unused]] GXDouble GXPreciseComplex::Length () const
{
    return sqrt ( SquaredLength () );
}

[[maybe_unused]] GXDouble GXPreciseComplex::SquaredLength () const
{
    return _r * _r + _i * _i;
}

[[maybe_unused]] GXBool GXPreciseComplex::Power ( GXUInt power )
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

[[maybe_unused]] GXPreciseComplex GXPreciseComplex::operator + ( GXPreciseComplex const &other ) const
{
    return GXPreciseComplex ( _r + other._r, _i + other._i );
}

[[maybe_unused]] GXPreciseComplex GXPreciseComplex::operator - ( GXPreciseComplex const &other ) const
{
    return GXPreciseComplex ( _r - other._r, _i - other._i );
}

[[maybe_unused]] GXPreciseComplex GXPreciseComplex::operator * ( GXPreciseComplex const &other ) const
{
    return GXPreciseComplex ( _r * other._r - _i * other._i, _r * other._i + _i * other._r );
}

[[maybe_unused]] GXPreciseComplex GXPreciseComplex::operator * ( GXDouble a ) const
{
    return GXPreciseComplex ( _r * a, _i * a );
}

[[maybe_unused]] GXPreciseComplex GXPreciseComplex::operator / ( GXDouble a ) const
{
    GXDouble const invA = 1.0 / a;
    return GXPreciseComplex ( _r * invA, _i * invA );
}

//----------------------------------------------------------------------------------------------------------------------

[[maybe_unused]] GXQuat::GXQuat ( GXMat3 const &rotationMatrix )
{
    From ( rotationMatrix );
}

[[maybe_unused]] GXQuat::GXQuat ( GXMat4 const &rotationMatrix )
{
    From ( rotationMatrix );
}

[[maybe_unused]] GXVoid GXQuat::Init ( GXFloat r, GXFloat a, GXFloat b, GXFloat c )
{
    _data[ 0U ] = r;
    _data[ 1U ] = a;
    _data[ 2U ] = b;
    _data[ 3U ] = c;
}

[[maybe_unused]] GXVoid GXQuat::SetR ( GXFloat r )
{
    _data[ 0U ] = r;
}

[[maybe_unused]] GXFloat GXQuat::GetR () const
{
    return _data[ 0U ];
}

[[maybe_unused]] GXVoid GXQuat::SetA ( GXFloat a )
{
    _data[ 1U ] = a;
}

[[maybe_unused]] GXFloat GXQuat::GetA () const
{
    return _data[ 1U ];
}

[[maybe_unused]] GXVoid GXQuat::SetB ( GXFloat b )
{
    _data[ 2U ] = b;
}

[[maybe_unused]] GXFloat GXQuat::GetB () const
{
    return _data[ 2U ];
}

[[maybe_unused]] GXVoid GXQuat::SetC ( GXFloat c )
{
    _data[ 3U ] = c;
}

[[maybe_unused]] GXFloat GXQuat::GetC () const
{
    return _data[ 3U ];
}

[[maybe_unused]] GXVoid GXQuat::Identity ()
{
    _data[ 0U ] = 1.0F;
    _data[ 1U ] = _data[ 2U ] = _data[ 3U ] = 0.0F;
}

[[maybe_unused]] GXVoid GXQuat::Normalize ()
{
    GXFloat const squaredLength = _data[ 0U ] * _data[ 0U ] +
        _data[ 1U ] * _data[ 1U ] +
        _data[ 2U ] * _data[ 2U ] +
        _data[ 3U ] * _data[ 3U ];

    if ( std::fabsf ( squaredLength ) < FLOAT_EPSILON )
    {
        assert ( !"GXQuat::Normalize - Error." );
        return;
    }

    Multiply ( *this, 1.0F / sqrtf ( squaredLength ) );
}

[[maybe_unused]] GXVoid GXQuat::Inverse ( GXQuat const &q )
{
    GXFloat const squaredLength = q._data[ 0U ] * q._data[ 0U ] +
        q._data[ 1U ] * q._data[ 1U ] +
        q._data[ 2U ] * q._data[ 2U ] +
        q._data[ 3U ] * q._data[ 3U ];

    if ( std::fabsf ( squaredLength ) <= FLOAT_EPSILON )
    {
        assert ( !"GXQuat::Inverse - Error." );
        Identity ();
        return;
    }

    GXFloat const inverseSquaredLength = 1.0F / squaredLength;

    _data[ 0U ] = q._data[ 0U ] * inverseSquaredLength;
    _data[ 1U ] = -q._data[ 1U ] * inverseSquaredLength;
    _data[ 2U ] = -q._data[ 2U ] * inverseSquaredLength;
    _data[ 3U ] = -q._data[ 3U ] * inverseSquaredLength;
}

[[maybe_unused]] GXVoid GXQuat::FromAxisAngle ( GXFloat x, GXFloat y, GXFloat z, GXFloat angle )
{
    GXFloat const halfAngle = 0.5F * angle;
    GXFloat const sinom = std::sinf ( halfAngle );

    _data[ 0U ] = std::cosf ( halfAngle );
    _data[ 1U ] = x * sinom;
    _data[ 2U ] = y * sinom;
    _data[ 3U ] = z * sinom;
}

[[maybe_unused]] GXVoid GXQuat::FromAxisAngle ( const GXVec3 &axis, GXFloat angle )
{
    FromAxisAngle ( axis._data[ 0U ], axis._data[ 1U ], axis._data[ 2U ], angle );
}

[[maybe_unused]] GXVoid GXQuat::From ( GXMat3 const &rotationMatrix )
{
    GXMat3 pureRotationMatrix;
    pureRotationMatrix.ClearRotation ( rotationMatrix );
    FromFast ( pureRotationMatrix );
}

[[maybe_unused]] GXVoid GXQuat::From ( GXMat4 const &rotationMatrix )
{
    GXMat4 pureRotationMatrix;
    pureRotationMatrix.ClearRotation ( rotationMatrix );
    FromFast ( pureRotationMatrix );
}

[[maybe_unused]] GXVoid GXQuat::FromFast ( GXMat3 const &pureRotationMatrix )
{
    // In ideal mathematics world all solutions are right.
    // But in practice more precise solution is the biggest "solutionFactorXXX" because of square root operation.

    GXFloat const solutionFactorAlpha = pureRotationMatrix._m[ 0U ][ 0U ] +
        pureRotationMatrix._m[ 1U ][ 1U ] +
        pureRotationMatrix._m[ 2U ][ 2U ] + 1.0F;

    GXFloat const solutionFactorBetta = pureRotationMatrix._m[ 0U ][ 0U ] -
        pureRotationMatrix._m[ 1U ][ 1U ] -
        pureRotationMatrix._m[ 2U ][ 2U ] + 1.0F;

    GXFloat const solutionFactorGamma = -pureRotationMatrix._m[ 0U ][ 0U ] +
        pureRotationMatrix._m[ 1U ][ 1U ] -
        pureRotationMatrix._m[ 2U ][ 2U ] + 1.0F;

    GXFloat const solutionFactorYotta = -pureRotationMatrix._m[ 0U ][ 0U ] -
        pureRotationMatrix._m[ 1U ][ 1U ] +
        pureRotationMatrix._m[ 2U ][ 2U ] + 1.0F;

    auto solution = static_cast<GXUByte> ( UNKNOWN_SOLUTION );

    if ( solutionFactorAlpha > solutionFactorBetta )
    {
        if ( solutionFactorAlpha > solutionFactorGamma )
        {
            solution = solutionFactorAlpha > solutionFactorYotta ?
                static_cast<GXUByte> ( SOLUTION_ALPHA ) :
                solution = static_cast<GXUByte> ( SOLUTION_YOTTA );
        }
        else if ( solutionFactorGamma > solutionFactorYotta )
        {
            solution = static_cast<GXUByte> ( SOLUTION_GAMMA );
        }
        else
        {
            solution = static_cast<GXUByte> ( SOLUTION_YOTTA );
        }
    }
    else if ( solutionFactorBetta > solutionFactorGamma )
    {
        solution = solutionFactorBetta > solutionFactorYotta ?
            static_cast<GXUByte> ( SOLUTION_BETTA ) :
            static_cast<GXUByte> ( SOLUTION_YOTTA );
    }
    else if ( solutionFactorGamma > solutionFactorYotta )
    {
        solution = static_cast<GXUByte> ( SOLUTION_GAMMA );
    }
    else
    {
        solution = static_cast<GXUByte> ( SOLUTION_YOTTA );
    }

    switch ( solution )
    {
        case SOLUTION_ALPHA:
        {
            GXFloat const phi = 0.5F * std::sqrtf ( solutionFactorAlpha );
            GXFloat const omega = 1.0F / ( 4.0F * phi );

            _data[ 0U ] = phi;
            _data[ 1U ] = omega * ( pureRotationMatrix._m[ 1U ][ 2U ] - pureRotationMatrix._m[ 2U ][ 1U ] );
            _data[ 2U ] = omega * ( pureRotationMatrix._m[ 2U ][ 0U ] - pureRotationMatrix._m[ 0U ][ 2U ] );
            _data[ 3U ] = omega * ( pureRotationMatrix._m[ 0U ][ 1U ] - pureRotationMatrix._m[ 1U ][ 0U ] );
        }
        break;

        case SOLUTION_BETTA:
        {
            GXFloat const phi = 0.5F * std::sqrtf ( solutionFactorBetta );
            GXFloat const omega = 1.0F / ( 4.0F * phi );

            _data[ 0U ] = omega * ( pureRotationMatrix._m[ 1U ][ 2U ] - pureRotationMatrix._m[ 2U ][ 1U ] );
            _data[ 1U ] = phi;
            _data[ 2U ] = omega * ( pureRotationMatrix._m[ 0U ][ 1U ] + pureRotationMatrix._m[ 1U ][ 0U ] );
            _data[ 3U ] = omega * ( pureRotationMatrix._m[ 0U ][ 2U ] + pureRotationMatrix._m[ 2U ][ 0U ] );
        }
        break;

        case SOLUTION_GAMMA:
        {
            GXFloat const phi = 0.5F * std::sqrtf ( solutionFactorGamma );
            GXFloat const omega = 1.0F / ( 4.0F * phi );

            _data[ 0U ] = omega * ( pureRotationMatrix._m[ 2U ][ 0U ] - pureRotationMatrix._m[ 0U ][ 2U ] );
            _data[ 1U ] = omega * ( pureRotationMatrix._m[ 0U ][ 1U ] + pureRotationMatrix._m[ 1U ][ 0U ] );
            _data[ 2U ] = phi;
            _data[ 3U ] = omega * ( pureRotationMatrix._m[ 1U ][ 2U ] + pureRotationMatrix._m[ 2U ][ 1U ] );
        }
        break;

        case SOLUTION_YOTTA:
        {
            GXFloat const phi = 0.5F * std::sqrtf ( solutionFactorYotta );
            GXFloat const omega = 1.0F / ( 4.0F * phi );

            _data[ 0U ] = omega * ( pureRotationMatrix._m[ 0U ][ 1U ] - pureRotationMatrix._m[ 1U ][ 0U ] );
            _data[ 1U ] = omega * ( pureRotationMatrix._m[ 0U ][ 2U ] + pureRotationMatrix._m[ 2U ][ 0U ] );
            _data[ 2U ] = omega * ( pureRotationMatrix._m[ 1U ][ 2U ] + pureRotationMatrix._m[ 2U ][ 1U ] );
            _data[ 3U ] = phi;
        }
        break;

        default:
            // NOTHING
        break;
    }
}

[[maybe_unused]] GXVoid GXQuat::FromFast ( GXMat4 const &pureRotationMatrix )
{
    // In ideal mathematics world all solutions are right.
    // But in practice more precise solution is the biggest "solutionFactorXXX" because of square root operation.

    GXFloat const solutionFactorAlpha = pureRotationMatrix._m[ 0U ][ 0U ] +
        pureRotationMatrix._m[ 1U ][ 1U ] +
        pureRotationMatrix._m[ 2U ][ 2U ] + 1.0F;

    GXFloat const solutionFactorBetta = pureRotationMatrix._m[ 0U ][ 0U ] -
        pureRotationMatrix._m[ 1U ][ 1U ] -
        pureRotationMatrix._m[ 2U ][ 2U ] + 1.0F;

    GXFloat const solutionFactorGamma = -pureRotationMatrix._m[ 0U ][ 0U ] +
        pureRotationMatrix._m[ 1U ][ 1U ] -
        pureRotationMatrix._m[ 2U ][ 2U ] + 1.0F;

    GXFloat const solutionFactorYotta = -pureRotationMatrix._m[ 0U ][ 0U ] -
        pureRotationMatrix._m[ 1U ][ 1U ] +
        pureRotationMatrix._m[ 2U ][ 2U ] + 1.0F;

    auto solution = static_cast<GXUByte> ( UNKNOWN_SOLUTION );

    if ( solutionFactorAlpha > solutionFactorBetta )
    {
        if ( solutionFactorAlpha > solutionFactorGamma )
        {
            solution = solutionFactorAlpha > solutionFactorYotta ?
                static_cast<GXUByte> ( SOLUTION_ALPHA ) :
                solution = static_cast<GXUByte> ( SOLUTION_YOTTA );
        }
        else if ( solutionFactorGamma > solutionFactorYotta )
        {
            solution = static_cast<GXUByte> ( SOLUTION_GAMMA );
        }
        else
        {
            solution = static_cast<GXUByte> ( SOLUTION_YOTTA );
        }
    }
    else if ( solutionFactorBetta > solutionFactorGamma )
    {
        solution = solutionFactorBetta > solutionFactorYotta ?
            static_cast<GXUByte> ( SOLUTION_BETTA ) :
            static_cast<GXUByte> ( SOLUTION_YOTTA );
    }
    else if ( solutionFactorGamma > solutionFactorYotta )
    {
        solution = static_cast<GXUByte> ( SOLUTION_GAMMA );
    }
    else
    {
        solution = static_cast<GXUByte> ( SOLUTION_YOTTA );
    }

    switch ( solution )
    {
        case SOLUTION_ALPHA:
        {
            GXFloat const phi = 0.5F * sqrtf ( solutionFactorAlpha );
            GXFloat const omega = 1.0F / ( 4.0F * phi );

            _data[ 0U ] = phi;
            _data[ 1U ] = omega * ( pureRotationMatrix._m[ 1U ][ 2U ] - pureRotationMatrix._m[ 2U ][ 1U ] );
            _data[ 2U ] = omega * ( pureRotationMatrix._m[ 2U ][ 0U ] - pureRotationMatrix._m[ 0U ][ 2U ] );
            _data[ 3U ] = omega * ( pureRotationMatrix._m[ 0U ][ 1U ] - pureRotationMatrix._m[ 1U ][ 0U ] );
        }
        break;

        case SOLUTION_BETTA:
        {
            GXFloat const phi = 0.5F * sqrtf ( solutionFactorBetta );
            GXFloat const omega = 1.0F / ( 4.0F * phi );

            _data[ 0U ] = omega * ( pureRotationMatrix._m[ 1U ][ 2U ] - pureRotationMatrix._m[ 2U ][ 1U ] );
            _data[ 1U ] = phi;
            _data[ 2U ] = omega * ( pureRotationMatrix._m[ 0U ][ 1U ] + pureRotationMatrix._m[ 1U ][ 0U ] );
            _data[ 3U ] = omega * ( pureRotationMatrix._m[ 0U ][ 2U ] + pureRotationMatrix._m[ 2U ][ 0U ] );
        }
        break;

        case SOLUTION_GAMMA:
        {
            GXFloat const phi = 0.5F * sqrtf ( solutionFactorGamma );
            GXFloat const omega = 1.0F / ( 4.0F * phi );

            _data[ 0U ] = omega * ( pureRotationMatrix._m[ 2U ][ 0U ] - pureRotationMatrix._m[ 0U ][ 2U ] );
            _data[ 1U ] = omega * ( pureRotationMatrix._m[ 0U ][ 1U ] + pureRotationMatrix._m[ 1U ][ 0U ] );
            _data[ 2U ] = phi;
            _data[ 3U ] = omega * ( pureRotationMatrix._m[ 1U ][ 2U ] + pureRotationMatrix._m[ 2U ][ 1U ] );
        }
        break;

        case SOLUTION_YOTTA:
        {
            GXFloat const phi = 0.5F * sqrtf ( solutionFactorYotta );
            GXFloat const omega = 1.0F / ( 4.0F * phi );

            _data[ 0U ] = omega * ( pureRotationMatrix._m[ 0U ][ 1U ] - pureRotationMatrix._m[ 1U ][ 0U ] );
            _data[ 1U ] = omega * ( pureRotationMatrix._m[ 0U ][ 2U ] + pureRotationMatrix._m[ 2U ][ 0U ] );
            _data[ 2U ] = omega * ( pureRotationMatrix._m[ 1U ][ 2U ] + pureRotationMatrix._m[ 2U ][ 1U ] );
            _data[ 3U ] = phi;
        }
        break;

        default:
            // NOTHING
        break;
    }
}

[[maybe_unused]] GXVoid GXQuat::Multiply ( GXQuat const &a, GXQuat const &b )
{
    _data[ 0U ] = a._data[ 0U ] * b._data[ 0U ] -
        a._data[ 1U ] * b._data[ 1U ] -
        a._data[ 2U ] * b._data[ 2U ] -
        a._data[ 3U ] * b._data[ 3U ];

    _data[ 1U ] = a._data[ 0U ] * b._data[ 1U ] +
        a._data[ 1U ] * b._data[ 0U ] +
        a._data[ 2U ] * b._data[ 3U ] -
        a._data[ 3U ] * b._data[ 2U ];

    _data[ 2U ] = a._data[ 0U ] * b._data[ 2U ] -
        a._data[ 1U ] * b._data[ 3U ] +
        a._data[ 2U ] * b._data[ 0U ] +
        a._data[ 3U ] * b._data[ 1U ];

    _data[ 3U ] = a._data[ 0U ] * b._data[ 3U ] +
        a._data[ 1U ] * b._data[ 2U ] -
        a._data[ 2U ] * b._data[ 1U ] +
        a._data[ 3U ] * b._data[ 0U ];
}

[[maybe_unused]] GXVoid GXQuat::Multiply ( GXQuat const &q, GXFloat scale )
{
    _data[ 0U ] = q._data[ 0U ] * scale;
    _data[ 1U ] = q._data[ 1U ] * scale;
    _data[ 2U ] = q._data[ 2U ] * scale;
    _data[ 3U ] = q._data[ 3U ] * scale;
}

[[maybe_unused]] GXVoid GXQuat::Sum ( GXQuat const &a, GXQuat const &b )
{
    _data[ 0U ] = a._data[ 0U ] + b._data[ 0U ];
    _data[ 1U ] = a._data[ 1U ] + b._data[ 1U ];
    _data[ 2U ] = a._data[ 2U ] + b._data[ 2U ];
    _data[ 3U ] = a._data[ 3U ] + b._data[ 3U ];
}

[[maybe_unused]] GXVoid GXQuat::Substract ( GXQuat const &a, GXQuat const &b )
{
    _data[ 0U ] = a._data[ 0U ] - b._data[ 0U ];
    _data[ 1U ] = a._data[ 1U ] - b._data[ 1U ];
    _data[ 2U ] = a._data[ 2U ] - b._data[ 2U ];
    _data[ 3U ] = a._data[ 3U ] - b._data[ 3U ];
}

[[maybe_unused]] GXVoid GXQuat::SphericalLinearInterpolation ( GXQuat const &start,
    GXQuat const &finish,
    GXFloat interpolationFactor )
{
    if ( interpolationFactor <= 0.0F )
    {
        *this = start;
        return;
    }

    if ( interpolationFactor >= 1.0F )
    {
        *this = finish;
        return;
    }

    GXFloat cosom = start._data[ 0U ] * finish._data[ 0U ] +
        start._data[ 1U ] * finish._data[ 1U ] +
        start._data[ 2U ] * finish._data[ 2U ] +
        start._data[ 3U ] * finish._data[ 3U ];

    GXQuat temp;

    if ( cosom < 0.0f )
    {
        temp._data[ 0U ] = -finish._data[ 0U ];
        temp._data[ 1U ] = -finish._data[ 1U ];
        temp._data[ 2U ] = -finish._data[ 2U ];
        temp._data[ 3U ] = -finish._data[ 3U ];
        cosom = -cosom;
    }
    else
    {
        temp = finish;
    }

    GXFloat scale0;
    GXFloat scale1;

    if ( ( 1.0F - cosom ) > FLOAT_EPSILON )
    {
        GXFloat const omega = std::acosf ( cosom );
        GXFloat const sinom = 1.0F / std::sinf ( omega );
        scale0 = std::sinf ( ( 1.0F - interpolationFactor ) * omega ) * sinom;
        scale1 = std::sinf ( interpolationFactor * omega ) * sinom;
    }
    else
    {
        scale0 = 1.0F - interpolationFactor;
        scale1 = interpolationFactor;
    }

    _data[ 0U ] = start._data[ 0U ] * scale0 + temp._data[ 0U ] * scale1;
    _data[ 1U ] = start._data[ 1U ] * scale0 + temp._data[ 1U ] * scale1;
    _data[ 2U ] = start._data[ 2U ] * scale0 + temp._data[ 2U ] * scale1;
    _data[ 3U ] = start._data[ 3U ] * scale0 + temp._data[ 3U ] * scale1;
}

[[maybe_unused]] GXVoid GXQuat::GetAxisAngle ( GXVec3 &axis, GXFloat &angle ) const
{
    GXQuat q ( *this );

    if ( std::fabsf ( q._data[ 0U ] ) > 1.0F )
        q.Normalize ();

    angle = 2.0F * std::acosf ( q._data[ 0U ] );

    axis._data[ 0U ] = q._data[ 1U ];
    axis._data[ 1U ] = q._data[ 2U ];
    axis._data[ 2U ] = q._data[ 3U ];

    GXFloat const s = std::sqrtf ( 1.0F - q._data[ 0U ] * q._data[ 0U ] );

    if ( s < FLOAT_EPSILON )
        return;

    axis.Multiply ( axis, 1.0F / s );
}

[[maybe_unused]] GXVoid GXQuat::Transform ( GXVec3 &out, GXVec3 const &v ) const
{
    GXFloat const rr = _data[ 0U ] * _data[ 0U ];
    GXFloat const ra2 = _data[ 0U ] * _data[ 1U ] * 2.0F;
    GXFloat const rb2 = _data[ 0U ] * _data[ 2U ] * 2.0F;
    GXFloat const rc2 = _data[ 0U ] * _data[ 3U ] * 2.0F;

    GXFloat const aa = _data[ 1U ] * _data[ 1U ];
    GXFloat const ab2 = _data[ 1U ] * _data[ 2U ] * 2.0F;
    GXFloat const ac2 = _data[ 1U ] * _data[ 3U ] * 2.0F;

    GXFloat const bb = _data[ 2U ] * _data[ 2U ];
    GXFloat const bc2 = _data[ 2U ] * _data[ 3U ] * 2.0F;

    GXFloat const cc = _data[ 3U ] * _data[ 3U ];

    GXFloat const inverseSquaredLength = 1.0F / ( rr + aa + bb + cc );

    out._data[ 0U ] = inverseSquaredLength *
        ( v._data[ 0U ] * ( rr + aa - bb - cc ) + v._data[ 1U ] * ( ab2 - rc2 ) + v._data[ 2U ] * ( rb2 + ac2 ) );

    out._data[ 1U ] = inverseSquaredLength *
        ( v._data[ 0U ] * ( rc2 + ab2 ) + v._data[ 1U ] * ( rr - aa + bb - cc ) + v._data[ 2U ] * ( bc2 - ra2 ) );

    out._data[ 2U ] = inverseSquaredLength *
        ( v._data[ 0U ] * ( ac2 - rb2 ) + v._data[ 1U ] * ( ra2 + bc2 ) + v._data[ 2U ] * ( rr - aa - bb + cc ) );
}

[[maybe_unused]] GXVoid GXQuat::TransformFast ( GXVec3 &out, GXVec3 const &v ) const
{
    GXFloat const rr = _data[ 0U ] * _data[ 0U ];
    GXFloat const ra2 = _data[ 0U ] * _data[ 1U ] * 2.0F;
    GXFloat const rb2 = _data[ 0U ] * _data[ 2U ] * 2.0F;
    GXFloat const rc2 = _data[ 0U ] * _data[ 3U ] * 2.0F;

    GXFloat const aa = _data[ 1U ] * _data[ 1U ];
    GXFloat const ab2 = _data[ 1U ] * _data[ 2U ] * 2.0F;
    GXFloat const ac2 = _data[ 1U ] * _data[ 3U ] * 2.0F;

    GXFloat const bb = _data[ 2U ] * _data[ 2U ];
    GXFloat const bc2 = _data[ 2U ] * _data[ 3U ] * 2.0F;

    GXFloat const cc = _data[ 3U ] * _data[ 3U ];

    out._data[ 0U ] = v._data[ 0U ] * ( rr + aa - bb - cc ) +
        v._data[ 1U ] * ( ab2 - rc2 ) +
        v._data[ 2U ] * ( rb2 + ac2 );

    out._data[ 1U ] = v._data[ 0U ] * ( rc2 + ab2 ) +
        v._data[ 1U ] * ( rr - aa + bb - cc ) +
        v._data[ 2U ] * ( bc2 - ra2 );

    out._data[ 2U ] = v._data[ 0U ] * ( ac2 - rb2 ) +
        v._data[ 1U ] * ( ra2 + bc2 ) +
        v._data[ 2U ] * ( rr - aa - bb + cc );
}

//----------------------------------------------------------------------------------------------------------------------

[[maybe_unused]] GXMat3::GXMat3 ( GXMat4 const &matrix )
{
    From ( matrix );
}

[[maybe_unused]] GXVoid GXMat3::From ( GXQuat const &quaternion )
{
    GXFloat rr = quaternion._data[ 0U ] * quaternion._data[ 0U ];
    GXFloat ra2 = quaternion._data[ 0U ] * quaternion._data[ 1U ] * 2.0F;
    GXFloat rb2 = quaternion._data[ 0U ] * quaternion._data[ 2U ] * 2.0F;
    GXFloat rc2 = quaternion._data[ 0U ] * quaternion._data[ 3u ] * 2.0F;

    GXFloat aa = quaternion._data[ 1U ] * quaternion._data[ 1U ];
    GXFloat ab2 = quaternion._data[ 1U ] * quaternion._data[ 2U ] * 2.0F;
    GXFloat ac2 = quaternion._data[ 1U ] * quaternion._data[ 3u ] * 2.0F;

    GXFloat bb = quaternion._data[ 2U ] * quaternion._data[ 2U ];
    GXFloat bc2 = quaternion._data[ 2U ] * quaternion._data[ 3u ] * 2.0F;

    GXFloat cc = quaternion._data[ 3u ] * quaternion._data[ 3u ];

    GXFloat inverseSquaredLength = 1.0f / ( rr + aa + bb + cc );

    _m[ 0U ][ 0U ] = inverseSquaredLength * ( rr + aa - bb - cc );
    _m[ 0U ][ 1U ] = inverseSquaredLength * ( rc2 + ab2 );
    _m[ 0U ][ 2U ] = inverseSquaredLength * ( ac2 - rb2 );

    _m[ 1U ][ 0U ] = inverseSquaredLength * ( ab2 - rc2 );
    _m[ 1U ][ 1U ] = inverseSquaredLength * ( rr - aa + bb - cc );
    _m[ 1U ][ 2U ] = inverseSquaredLength * ( ra2 + bc2 );

    _m[ 2U ][ 0U ] = inverseSquaredLength * ( rb2 + ac2 );
    _m[ 2U ][ 1U ] = inverseSquaredLength * ( bc2 - ra2 );
    _m[ 2U ][ 2U ] = inverseSquaredLength * ( rr - aa - bb + cc );
}

[[maybe_unused]] GXVoid GXMat3::From ( GXMat4 const &matrix )
{
    constexpr GXUPointer const lineSize = 3U * sizeof ( GXFloat );

    memcpy ( _data, matrix._data, lineSize );
    memcpy ( _data + 3U, matrix._data + 4U, lineSize );
    memcpy ( _data + 6U, matrix._data + 8U, lineSize );
}

[[maybe_unused]] GXVoid GXMat3::From ( const GXVec3 &zDirection )
{
    GXVec3 xAxis;
    GXVec3 yAxis;

    if ( zDirection.DotProduct ( GXVec3::GetAbsoluteX () ) < 0.5F )
    {
        GXVec3 tmp;
        tmp.CrossProduct ( zDirection, GXVec3::GetAbsoluteX () );
        xAxis.CrossProduct ( tmp, zDirection );
        xAxis.Normalize ();
        yAxis.CrossProduct ( zDirection, xAxis );
    }
    else
    {
        GXVec3 tmp;
        tmp.CrossProduct ( zDirection, GXVec3::GetAbsoluteY () );
        yAxis.CrossProduct ( zDirection, tmp );
        yAxis.Normalize ();
        xAxis.CrossProduct ( yAxis, zDirection );
    }

    SetX ( xAxis );
    SetY ( yAxis );
    SetZ ( zDirection );
}

[[maybe_unused]] GXVoid GXMat3::FromFast ( GXQuat const &quaternion )
{
    GXFloat rr = quaternion._data[ 0U ] * quaternion._data[ 0U ];
    GXFloat ra2 = quaternion._data[ 0U ] * quaternion._data[ 1U ] * 2.0F;
    GXFloat rb2 = quaternion._data[ 0U ] * quaternion._data[ 2U ] * 2.0F;
    GXFloat rc2 = quaternion._data[ 0U ] * quaternion._data[ 3u ] * 2.0F;

    GXFloat aa = quaternion._data[ 1U ] * quaternion._data[ 1U ];
    GXFloat ab2 = quaternion._data[ 1U ] * quaternion._data[ 2U ] * 2.0F;
    GXFloat ac2 = quaternion._data[ 1U ] * quaternion._data[ 3u ] * 2.0F;

    GXFloat bb = quaternion._data[ 2U ] * quaternion._data[ 2U ];
    GXFloat bc2 = quaternion._data[ 2U ] * quaternion._data[ 3u ] * 2.0F;

    GXFloat cc = quaternion._data[ 3u ] * quaternion._data[ 3u ];

    _m[ 0U ][ 0U ] = rr + aa - bb - cc;
    _m[ 0U ][ 1U ] = rc2 + ab2;
    _m[ 0U ][ 2U ] = ac2 - rb2;

    _m[ 1U ][ 0U ] = ab2 - rc2;
    _m[ 1U ][ 1U ] = rr - aa + bb - cc;
    _m[ 1U ][ 2U ] = ra2 + bc2;

    _m[ 2U ][ 0U ] = rb2 + ac2;
    _m[ 2U ][ 1U ] = bc2 - ra2;
    _m[ 2U ][ 2U ] = rr - aa - bb + cc;
}

[[maybe_unused]] GXVoid GXMat3::SetX ( GXVec3 const &x )
{
    memcpy ( _data, &x, sizeof ( GXVec3 ) );
}

[[maybe_unused]] GXVoid GXMat3::GetX ( GXVec3 &x ) const
{
    memcpy ( &x, _data, sizeof ( GXVec3 ) );
}

[[maybe_unused]] GXVoid GXMat3::SetY ( GXVec3 const &y )
{
    memcpy ( _data + 3U, &y, sizeof ( GXVec3 ) );
}

[[maybe_unused]] GXVoid GXMat3::GetY ( GXVec3 &y ) const
{
    memcpy ( &y, _data + 3U, sizeof ( GXVec3 ) );
}

[[maybe_unused]] GXVoid GXMat3::SetZ ( GXVec3 const &z )
{
    memcpy ( _data + 6U, &z, sizeof ( GXVec3 ) );
}

[[maybe_unused]] GXVoid GXMat3::GetZ ( GXVec3 &z ) const
{
    memcpy ( &z, _data + 6U, sizeof ( GXVec3 ) );
}

[[maybe_unused]] GXVoid GXMat3::Identity ()
{
    _m[ 0U ][ 0U ] = _m[ 1U ][ 1U ] = _m[ 2U ][ 2U ] = 1.0F;
    _m[ 0U ][ 1U ] = _m[ 0U ][ 2U ] = 0.0F;
    _m[ 1U ][ 0U ] = _m[ 1U ][ 2U ] = 0.0F;
    _m[ 2U ][ 0U ] = _m[ 2U ][ 1U ] = 0.0F;
}

[[maybe_unused]] GXVoid GXMat3::Zeros ()
{
    memset ( _data, 0, sizeof ( _data ) );
}

[[maybe_unused]] GXVoid GXMat3::Inverse ( GXMat3 const &sourceMatrix )
{
    GXMat3 const& m = sourceMatrix;

    GXFloat determinant = m._m[ 0U ][ 0U ] *
        ( m._m[ 1U ][ 1U ] * m._m[ 2U ][ 2U ] - m._m[ 2U ][ 1U ] * m._m[ 1U ][ 2U ] );

    determinant -= m._m[ 0U ][ 1U ] *
        ( m._m[ 1U ][ 0U ] * m._m[ 2U ][ 2U ] - m._m[ 2U ][ 0U ] * m._m[ 1U ][ 2U ] );

    determinant += m._m[ 0U ][ 2U ] *
        ( m._m[ 1U ][ 0U ] * m._m[ 2U ][ 1U ] - m._m[ 2U ][ 0U ] * m._m[ 1U ][ 1U ] );

    GXFloat const invDeterminant = 1.0F / determinant;

    _m[ 0U ][ 0U ] = invDeterminant * ( m._m[ 1U ][ 1U ] * m._m[ 2U ][ 2U ] - m._m[ 2U ][ 1U ] * m._m[ 1U ][ 2U ] );
    _m[ 0U ][ 1U ] = invDeterminant * ( m._m[ 0U ][ 2U ] * m._m[ 2U ][ 1U ] - m._m[ 2U ][ 2U ] * m._m[ 0U ][ 1U ] );
    _m[ 0U ][ 2U ] = invDeterminant * ( m._m[ 0U ][ 1U ] * m._m[ 1U ][ 2U ] - m._m[ 1U ][ 1U ] * m._m[ 0U ][ 2U ] );

    _m[ 1U ][ 0U ] = invDeterminant * ( m._m[ 1U ][ 2U ] * m._m[ 2U ][ 0U ] - m._m[ 2U ][ 2U ] * m._m[ 1U ][ 0U ] );
    _m[ 1U ][ 1U ] = invDeterminant * ( m._m[ 0U ][ 0U ] * m._m[ 2U ][ 2U ] - m._m[ 2U ][ 0U ] * m._m[ 0U ][ 2U ] );
    _m[ 1U ][ 2U ] = invDeterminant * ( m._m[ 0U ][ 2U ] * m._m[ 1U ][ 0U ] - m._m[ 1U ][ 2U ] * m._m[ 0U ][ 0U ] );

    _m[ 2U ][ 0U ] = invDeterminant * ( m._m[ 1U ][ 0U ] * m._m[ 2U ][ 1U ] - m._m[ 2U ][ 0U ] * m._m[ 1U ][ 1U ] );
    _m[ 2U ][ 1U ] = invDeterminant * ( m._m[ 0U ][ 1U ] * m._m[ 2U ][ 0U ] - m._m[ 2U ][ 1U ] * m._m[ 0U ][ 0U ] );
    _m[ 2U ][ 2U ] = invDeterminant * ( m._m[ 0U ][ 0U ] * m._m[ 1U ][ 1U ] - m._m[ 1U ][ 0U ] * m._m[ 0U ][ 1U ] );
}

[[maybe_unused]] GXVoid GXMat3::Transponse ( GXMat3 const &sourceMatrix )
{
    _m[ 0U ][ 0U ] = sourceMatrix._m[ 0U ][ 0U ];
    _m[ 0U ][ 1U ] = sourceMatrix._m[ 1U ][ 0U ];
    _m[ 0U ][ 2U ] = sourceMatrix._m[ 2U ][ 0U ];

    _m[ 1U ][ 0U ] = sourceMatrix._m[ 0U ][ 1U ];
    _m[ 1U ][ 1U ] = sourceMatrix._m[ 1U ][ 1U ];
    _m[ 1U ][ 2U ] = sourceMatrix._m[ 2U ][ 1U ];

    _m[ 2U ][ 0U ] = sourceMatrix._m[ 0U ][ 2U ];
    _m[ 2U ][ 1U ] = sourceMatrix._m[ 1U ][ 2U ];
    _m[ 2U ][ 2U ] = sourceMatrix._m[ 2U ][ 2U ];
}

[[maybe_unused]] GXVoid GXMat3::ClearRotation ( GXMat3 const &sourceMatrix )
{
    GXVec3 modelX;
    GXVec3 modelY;
    GXVec3 modelZ;

    sourceMatrix.GetX ( modelX );
    sourceMatrix.GetY ( modelY );
    sourceMatrix.GetZ ( modelZ );

    GXVec3 tmp;
    tmp.Multiply ( modelX, 1.0F / modelX.Length () );
    SetX ( tmp );

    tmp.Multiply ( modelY, 1.0F / modelY.Length () );
    SetY ( tmp );

    tmp.Multiply ( modelZ, 1.0F / modelZ.Length () );
    SetZ ( tmp );
}

[[maybe_unused]] GXVoid GXMat3::ClearRotation ( GXMat4 const &sourceMatrix )
{
    GXVec3 modelX;
    GXVec3 modelY;
    GXVec3 modelZ;

    sourceMatrix.GetX ( modelX );
    sourceMatrix.GetY ( modelY );
    sourceMatrix.GetZ ( modelZ );

    GXVec3 tmp;
    tmp.Multiply ( modelX, 1.0F / modelX.Length () );
    SetX ( tmp );

    tmp.Multiply ( modelY, 1.0F / modelY.Length () );
    SetY ( tmp );

    tmp.Multiply ( modelZ, 1.0F / modelZ.Length () );
    SetZ ( tmp );
}

[[maybe_unused]] GXVoid GXMat3::SkewSymmetric ( GXVec3 const &base )
{
    _m[ 0U ][ 0U ] = _m[ 1U ][ 1U ] = _m[ 2U ][ 2U ] = 0.0F;

    _m[ 0U ][ 1U ] = -base._data[ 2U ];
    _m[ 0U ][ 2U ] = base._data[ 1U ];

    _m[ 1U ][ 0U ] = base._data[ 2U ];
    _m[ 1U ][ 2U ] = -base._data[ 0U ];

    _m[ 2U ][ 0U ] = -base._data[ 1U ];
    _m[ 2U ][ 1U ] = base._data[ 0U ];
}

[[maybe_unused]] GXVoid GXMat3::Sum ( GXMat3 const &a, GXMat3 const &b )
{
    _m[ 0U ][ 0U ] = a._m[ 0U ][ 0U ] + b._m[ 0U ][ 0U ];
    _m[ 0U ][ 1U ] = a._m[ 0U ][ 1U ] + b._m[ 0U ][ 1U ];
    _m[ 0U ][ 2U ] = a._m[ 0U ][ 2U ] + b._m[ 0U ][ 2U ];

    _m[ 1U ][ 0U ] = a._m[ 1U ][ 0U ] + b._m[ 1U ][ 0U ];
    _m[ 1U ][ 1U ] = a._m[ 1U ][ 1U ] + b._m[ 1U ][ 1U ];
    _m[ 1U ][ 2U ] = a._m[ 1U ][ 2U ] + b._m[ 1U ][ 2U ];

    _m[ 2U ][ 0U ] = a._m[ 2U ][ 0U ] + b._m[ 2U ][ 0U ];
    _m[ 2U ][ 1U ] = a._m[ 2U ][ 1U ] + b._m[ 2U ][ 1U ];
    _m[ 2U ][ 2U ] = a._m[ 2U ][ 2U ] + b._m[ 2U ][ 2U ];
}

[[maybe_unused]] GXVoid GXMat3::Substract ( GXMat3 const &a, GXMat3 const &b )
{
    _m[ 0U ][ 0U ] = a._m[ 0U ][ 0U ] - b._m[ 0U ][ 0U ];
    _m[ 0U ][ 1U ] = a._m[ 0U ][ 1U ] - b._m[ 0U ][ 1U ];
    _m[ 0U ][ 2U ] = a._m[ 0U ][ 2U ] - b._m[ 0U ][ 2U ];

    _m[ 1U ][ 0U ] = a._m[ 1U ][ 0U ] - b._m[ 1U ][ 0U ];
    _m[ 1U ][ 1U ] = a._m[ 1U ][ 1U ] - b._m[ 1U ][ 1U ];
    _m[ 1U ][ 2U ] = a._m[ 1U ][ 2U ] - b._m[ 1U ][ 2U ];

    _m[ 2U ][ 0U ] = a._m[ 2U ][ 0U ] - b._m[ 2U ][ 0U ];
    _m[ 2U ][ 1U ] = a._m[ 2U ][ 1U ] - b._m[ 2U ][ 1U ];
    _m[ 2U ][ 2U ] = a._m[ 2U ][ 2U ] - b._m[ 2U ][ 2U ];
}

[[maybe_unused]] GXVoid GXMat3::Multiply ( GXMat3 const &a, GXMat3 const &b )
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

[[maybe_unused]] GXVoid GXMat3::MultiplyVectorMatrix ( GXVec3 &out, GXVec3 const &v ) const
{
    out._data[ 0U ] = v._data[ 0U ] * _m[ 0U ][ 0U ] + v._data[ 1U ] * _m[ 1U ][ 0U ] + v._data[ 2U ] * _m[ 2U ][ 0U ];
    out._data[ 1U ] = v._data[ 0U ] * _m[ 0U ][ 1U ] + v._data[ 1U ] * _m[ 1U ][ 1U ] + v._data[ 2U ] * _m[ 2U ][ 1U ];
    out._data[ 2U ] = v._data[ 0U ] * _m[ 0U ][ 2U ] + v._data[ 1U ] * _m[ 1U ][ 2U ] + v._data[ 2U ] * _m[ 2U ][ 2U ];
}

[[maybe_unused]] GXVoid GXMat3::MultiplyMatrixVector ( GXVec3 &out, GXVec3 const &v ) const
{
    out._data[ 0U ] = _m[ 0U ][ 0U ] * v._data[ 0U ] + _m[ 0U ][ 1U ] * v._data[ 1U ] + _m[ 0U ][ 2U ] * v._data[ 2U ];
    out._data[ 1U ] = _m[ 1U ][ 0U ] * v._data[ 0U ] + _m[ 1U ][ 1U ] * v._data[ 1U ] + _m[ 1U ][ 2U ] * v._data[ 2U ];
    out._data[ 2U ] = _m[ 2U ][ 0U ] * v._data[ 0U ] + _m[ 2U ][ 1U ] * v._data[ 1U ] + _m[ 2U ][ 2U ] * v._data[ 2U ];
}

[[maybe_unused]] GXVoid GXMat3::Multiply ( GXMat3 const &a, GXFloat factor )
{
    _m[ 0U ][ 0U ] = a._m[ 0U ][ 0U ] * factor;
    _m[ 0U ][ 1U ] = a._m[ 0U ][ 1U ] * factor;
    _m[ 0U ][ 2U ] = a._m[ 0U ][ 2U ] * factor;

    _m[ 1U ][ 0U ] = a._m[ 1U ][ 0U ] * factor;
    _m[ 1U ][ 1U ] = a._m[ 1U ][ 1U ] * factor;
    _m[ 1U ][ 2U ] = a._m[ 1U ][ 2U ] * factor;

    _m[ 2U ][ 0U ] = a._m[ 2U ][ 0U ] * factor;
    _m[ 2U ][ 1U ] = a._m[ 2U ][ 1U ] * factor;
    _m[ 2U ][ 2U ] = a._m[ 2U ][ 2U ] * factor;
}

//----------------------------------------------------------------------------------------------------------------------

GXMat4::GXMat4 ()
{
    memset ( _data, 0, 16 * sizeof ( GXFloat ) );
}

GXMat4::GXMat4 ( const GXMat4 &other )
{
    memcpy ( this, &other, sizeof ( GXMat4 ) );
} 

GXVoid GXMat4::SetRotation ( const GXQuat &quaternion )
{
    GXFloat rr = quaternion._data[ 0u ] * quaternion._data[ 0u ];
    GXFloat ra2 = quaternion._data[ 0u ] * quaternion._data[ 1u ] * 2.0f;
    GXFloat rb2 = quaternion._data[ 0u ] * quaternion._data[ 2u ] * 2.0f;
    GXFloat rc2 = quaternion._data[ 0u ] * quaternion._data[ 3u ] * 2.0f;

    GXFloat aa = quaternion._data[ 1u ] * quaternion._data[ 1u ];
    GXFloat ab2 = quaternion._data[ 1u ] * quaternion._data[ 2u ] * 2.0f;
    GXFloat ac2 = quaternion._data[ 1u ] * quaternion._data[ 3u ] * 2.0f;

    GXFloat bb = quaternion._data[ 2u ] * quaternion._data[ 2u ];
    GXFloat bc2 = quaternion._data[ 2u ] * quaternion._data[ 3u ] * 2.0f;

    GXFloat cc = quaternion._data[ 3u ] * quaternion._data[ 3u ];

    GXFloat inverseSquaredLength = 1.0f / ( rr + aa + bb + cc );

    _m[ 0u ][ 0u ] = inverseSquaredLength * ( rr + aa - bb - cc );
    _m[ 0u ][ 1u ] = inverseSquaredLength * ( rc2 + ab2 );
    _m[ 0u ][ 2u ] = inverseSquaredLength * ( ac2 - rb2 );

    _m[ 1u ][ 0u ] = inverseSquaredLength * ( ab2 - rc2 );
    _m[ 1u ][ 1u ] = inverseSquaredLength * ( rr - aa + bb - cc );
    _m[ 1u ][ 2u ] = inverseSquaredLength * ( ra2 + bc2 );

    _m[ 2u ][ 0u ] = inverseSquaredLength * ( rb2 + ac2 );
    _m[ 2u ][ 1u ] = inverseSquaredLength * ( bc2 - ra2 );
    _m[ 2u ][ 2u ] = inverseSquaredLength * ( rr - aa - bb + cc );
}

GXVoid GXMat4::SetRotationFast ( const GXQuat &quaternion )
{
    GXFloat rr = quaternion._data[ 0u ] * quaternion._data[ 0u ];
    GXFloat ra2 = quaternion._data[ 0u ] * quaternion._data[ 1u ] * 2.0f;
    GXFloat rb2 = quaternion._data[ 0u ] * quaternion._data[ 2u ] * 2.0f;
    GXFloat rc2 = quaternion._data[ 0u ] * quaternion._data[ 3u ] * 2.0f;

    GXFloat aa = quaternion._data[ 1u ] * quaternion._data[ 1u ];
    GXFloat ab2 = quaternion._data[ 1u ] * quaternion._data[ 2u ] * 2.0f;
    GXFloat ac2 = quaternion._data[ 1u ] * quaternion._data[ 3u ] * 2.0f;

    GXFloat bb = quaternion._data[ 2u ] * quaternion._data[ 2u ];
    GXFloat bc2 = quaternion._data[ 2u ] * quaternion._data[ 3u ] * 2.0f;

    GXFloat cc = quaternion._data[ 3u ] * quaternion._data[ 3u ];

    _m[ 0u ][ 0u ] = rr + aa - bb - cc;
    _m[ 0u ][ 1u ] = rc2 + ab2;
    _m[ 0u ][ 2u ] = ac2 - rb2;

    _m[ 1u ][ 0u ] = ab2 - rc2;
    _m[ 1u ][ 1u ] = rr - aa + bb - cc;
    _m[ 1u ][ 2u ] = ra2 + bc2;

    _m[ 2u ][ 0u ] = rb2 + ac2;
    _m[ 2u ][ 1u ] = bc2 - ra2;
    _m[ 2u ][ 2u ] = rr - aa - bb + cc;
}

GXVoid GXMat4::SetOrigin ( const GXVec3 &origin )
{ 
    SetW ( origin );
}

GXVoid GXMat4::From ( const GXQuat &quaternion, const GXVec3 &origin )
{
    SetRotation ( quaternion );
    SetOrigin ( origin );

    _m[ 0u ][ 3u ] = _m[ 1u ][ 3u ] = _m[ 2u ][ 3u ] = 0.0f;
    _m[ 3u ][ 3u ] = 1.0f;
}

GXVoid GXMat4::From ( const GXMat3 &rotation, const GXVec3 &origin )
{
    GXVec3 tmp;
    rotation.GetX ( tmp );
    SetX ( tmp );

    rotation.GetY ( tmp );
    SetY ( tmp );

    rotation.GetZ ( tmp );
    SetZ ( tmp );

    SetW ( origin );

    _m[ 0u ][ 3u ] = _m[ 1u ][ 3u ] = _m[ 2u ][ 3u ] = 0.0f;
    _m[ 3u ][ 3u ] = 1.0f;
}

GXVoid GXMat4::From ( const GXVec3 &zDirection, const GXVec3 &origin )
{
    GXVec3 xAxis;
    GXVec3 yAxis;

    if ( zDirection.DotProduct ( GXVec3::GetAbsoluteX () ) < 0.5f )
    {
        GXVec3 tmp;
        tmp.CrossProduct ( zDirection, GXVec3::GetAbsoluteX () );
        xAxis.CrossProduct ( tmp, zDirection );
        xAxis.Normalize ();
        yAxis.CrossProduct ( zDirection, xAxis );
    }
    else
    {
        GXVec3 tmp;
        tmp.CrossProduct ( zDirection, GXVec3::GetAbsoluteY () );
        yAxis.CrossProduct ( zDirection, tmp );
        yAxis.Normalize ();
        xAxis.CrossProduct ( yAxis, zDirection );
    }

    SetX ( xAxis );
    SetY ( yAxis );
    SetZ ( zDirection );
    SetW ( origin );

    _m[ 3u ][ 3u ] = 1.0f;
}

GXVoid GXMat4::FromFast ( const GXQuat &quaternion, const GXVec3 &origin )
{
    SetRotationFast ( quaternion );
    SetOrigin ( origin );

    _m[ 0u ][ 3u ] = _m[ 1u ][ 3u ] = _m[ 2u ][ 3u ] = 0.0f;
    _m[ 3u ][ 3u ] = 1.0f;
}

GXVoid GXMat4::SetX ( const GXVec3 &x )
{
    memcpy ( _data, &x, sizeof ( GXVec3 ) );
}

GXVoid GXMat4::GetX ( GXVec3 &x ) const
{
    memcpy ( &x, _data, sizeof ( GXVec3 ) );
}

GXVoid GXMat4::SetY ( const GXVec3 &y )
{
    memcpy ( _data + 4u, &y, sizeof ( GXVec3 ) );
}

GXVoid GXMat4::GetY ( GXVec3 &y ) const
{
    memcpy ( &y, _data + 4u, sizeof ( GXVec3 ) );
}

GXVoid GXMat4::SetZ ( const GXVec3 &z )
{
    memcpy ( _data + 8u, &z, sizeof ( GXVec3 ) );
}

GXVoid GXMat4::GetZ ( GXVec3 &z ) const
{
    memcpy ( &z, _data + 8u, sizeof ( GXVec3 ) );
}

GXVoid GXMat4::SetW ( const GXVec3 &w )
{
    memcpy ( _data + 12u, &w, sizeof ( GXVec3 ) );
}

GXVoid GXMat4::GetW ( GXVec3 &w ) const
{
    memcpy ( &w, _data + 12u, sizeof ( GXVec3 ) );
}

GXVoid GXMat4::Identity ()
{
    _m[ 0u ][ 1u ] = _m[ 0u ][ 2u ] = _m[ 0u ][ 3u ] = 0.0f;
    _m[ 1u ][ 0u ] = _m[ 1u ][ 2u ] = _m[ 1u ][ 3u ] = 0.0f;
    _m[ 2u ][ 0u ] = _m[ 2u ][ 1u ] = _m[ 2u ][ 3u ] = 0.0f;
    _m[ 3u ][ 0u ] = _m[ 3u ][ 1u ] = _m[ 3u ][ 2u ] = 0.0f;

    _m[ 0u ][ 0u ] = _m[ 1u ][ 1u ] = _m[ 2u ][ 2u ] = _m[ 3u ][ 3u ] = 1.0f;
}

GXVoid GXMat4::Translation ( GXFloat x, GXFloat y, GXFloat z )
{
    _m[ 0u ][ 1u ] = _m[ 0u ][ 2u ] = _m[ 0u ][ 3u ] = 0.0f;
    _m[ 1u ][ 0u ] = _m[ 1u ][ 2u ] = _m[ 1u ][ 3u ] = 0.0f;
    _m[ 2u ][ 0u ] = _m[ 2u ][ 1u ] = _m[ 2u ][ 3u ] = 0.0f;

    _m[ 0u ][ 0u ] = _m[ 1u ][ 1u ] = _m[ 2u ][ 2u ] = _m[ 3u ][ 3u ] = 1.0f;

    _m[ 3u ][ 0u ] = x;
    _m[ 3u ][ 1u ] = y;
    _m[ 3u ][ 2u ] = z;
}

GXVoid GXMat4::TranslateTo ( GXFloat x, GXFloat y, GXFloat z )
{
    GXVec3 origin ( x, y, z );
    SetW ( origin );
}

GXVoid GXMat4::TranslateTo ( const GXVec3 &location )
{
    SetW ( location );
}

GXVoid GXMat4::RotationX ( GXFloat angle )
{
    _m[ 0u ][ 1u ] = _m[ 0u ][ 2u ] = 0.0f;
    _m[ 1u ][ 0u ] = 0.0f;
    _m[ 2u ][ 0u ] = 0.0f;

    GXFloat c;
    GXFloat s;

    c = cosf ( angle );
    s = sinf ( angle );

    _m[ 1u ][ 1u ] = c;      _m[ 1u ][ 2u ] = s;
    _m[ 2u ][ 1u ] = -s;     _m[ 2u ][ 2u ] = c;

    _m[ 0u ][ 0u ] = 1.0f;

    _m[ 3u ][ 3u ] = 1.0f;
    _m[ 3u ][ 0u ] = _m[ 3u ][ 1u ] = _m[ 3u ][ 2u ] = _m[ 0u ][ 3u ] = _m[ 1u ][ 3u ] = _m[ 2u ][ 3u ] = 0.0f;
}

GXVoid GXMat4::RotationY ( GXFloat angle )
{
    _m[ 0u ][ 1u ] = 0.0f;
    _m[ 1u ][ 0u ] = _m[ 1u ][ 2u ] = 0.0f;
    _m[ 2u ][ 1u ] = 0.0f;

    GXFloat c;
    GXFloat s;

    c = cosf ( angle );
    s = sinf ( angle );

    _m[ 0u ][ 0u ] = c;      _m[ 0u ][ 2u ] = -s;
    _m[ 2u ][ 0u ] = s;      _m[ 2u ][ 2u ] = c;

    _m[ 1u ][ 1u ] = 1.0f;

    _m[ 3u ][ 3u ] = 1.0f;
    _m[ 3u ][ 0u ] = _m[ 3u ][ 1u ] = _m[ 3u ][ 2u ] = _m[ 0u ][ 3u ] = _m[ 1u ][ 3u ] = _m[ 2u ][ 3u ] = 0.0f;
}

GXVoid GXMat4::RotationZ ( GXFloat angle )
{
    _m[ 0u ][ 2u ] = 0.0f;
    _m[ 1u ][ 2u ] = 0.0f;
    _m[ 2u ][ 0u ] = _m[ 2u ][ 1u ] = 0.0f;

    GXFloat c;
    GXFloat s;

    c = cosf ( angle );
    s = sinf ( angle );

    _m[ 0u ][ 0u ] = c;      _m[ 0u ][ 1u ] = s;
    _m[ 1u ][ 0u ] = -s;     _m[ 1u ][ 1u ] = c;

    _m[ 2u ][ 2u ] = 1.0f;

    _m[ 3u ][ 3u ] = 1.0f;
    _m[ 3u ][ 0u ] = _m[ 3u ][ 1u ] = _m[ 3u ][ 2u ] = _m[ 0u ][ 3u ] = _m[ 1u ][ 3u ] = _m[ 2u ][ 3u ] = 0.0f;
}

GXVoid GXMat4::RotationXY ( GXFloat pitchRadians, GXFloat yawRadians )
{
    GXMat4 x;
    x.RotationX ( pitchRadians );

    GXMat4 y;
    y.RotationY ( yawRadians );

    Multiply ( x, y );
}

GXVoid GXMat4::RotationXYZ ( GXFloat pitchRadians, GXFloat yawRadians, GXFloat rollRadians )
{
    GXMat4 x;
    x.RotationX ( pitchRadians );

    GXMat4 y;
    y.RotationY ( yawRadians );

    GXMat4 z;
    z.RotationZ ( rollRadians );

    GXMat4 temp;
    temp.Multiply ( x, y );

    Multiply ( temp, z );
}

GXVoid GXMat4::ClearRotation ( const GXMat3 &sourceMatrix )
{
    GXVec3 outX;
    GXVec3 outY;
    GXVec3 outZ;

    GXVec3 modelX;
    GXVec3 modelY;
    GXVec3 modelZ;

    sourceMatrix.GetX ( modelX );
    sourceMatrix.GetY ( modelY );
    sourceMatrix.GetZ ( modelZ );

    GXVec3 tmp;
    tmp.Multiply ( modelX, 1.0f / modelX.Length () );
    SetX ( tmp );

    tmp.Multiply ( modelY, 1.0f / modelY.Length () );
    SetY ( tmp );

    tmp.Multiply ( modelZ, 1.0f / modelZ.Length () );
    SetZ ( tmp );

    _m[ 0u ][ 3u ] = _m[ 1u ][ 3u ] = _m[ 2u ][ 3u ] = 0.0f;
    _m[ 3u ][ 0u ] = _m[ 3u ][ 1u ] = _m[ 3u ][ 2u ] = 0.0f;

    _m[ 3u ][ 3u ] = 1.0f;
}

GXVoid GXMat4::ClearRotation ( const GXMat4 &sourceMatrix )
{
    GXVec3 outX;
    GXVec3 outY;
    GXVec3 outZ;

    GXVec3 modelX;
    GXVec3 modelY;
    GXVec3 modelZ;

    sourceMatrix.GetX ( modelX );
    sourceMatrix.GetY ( modelY );
    sourceMatrix.GetZ ( modelZ );

    GXVec3 tmp;
    tmp.Multiply ( modelX, 1.0f / modelX.Length () );
    SetX ( tmp );

    tmp.Multiply ( modelY, 1.0f / modelY.Length () );
    SetY ( tmp );

    tmp.Multiply ( modelZ, 1.0f / modelZ.Length () );
    SetZ ( tmp );

    _m[ 0u ][ 3u ] = _m[ 1u ][ 3u ] = _m[ 2u ][ 3u ] = 0.0f;
    _m[ 3u ][ 0u ] = _m[ 3u ][ 1u ] = _m[ 3u ][ 2u ] = 0.0f;

    _m[ 3u ][ 3u ] = 1.0f;
}

GXVoid GXMat4::Scale ( GXFloat x, GXFloat y, GXFloat z )
{
    _m[ 0u ][ 1u ] = _m[ 0u ][ 2u ] = 0.0f;
    _m[ 1u ][ 0u ] = _m[ 1u ][ 2u ] = 0.0f;
    _m[ 2u ][ 0u ] = _m[ 2u ][ 1u ] = 0.0f;

    _m[ 0u ][ 0u ] = x;
    _m[ 1u ][ 1u ] = y;
    _m[ 2u ][ 2u ] = z;

    _m[ 3u ][ 3u ] = 1.0f;
    _m[ 3u ][ 0u ] = _m[ 3u ][ 1u ] = _m[ 3u ][ 2u ] = _m[ 0u ][ 3u ] = _m[ 1u ][ 3u ] = _m[ 2u ][ 3u ] = 0.0f;
}

GXVoid GXMat4::ClearScale ( GXVec3 &scale ) const
{
    GXVec3 alpha;

    GetX ( alpha );
    scale._data[ 0u ] = alpha.Length ();

    GetY ( alpha );
    scale._data[ 1u ] = alpha.Length ();

    GetZ ( alpha );
    scale._data[ 2u ] = alpha.Length ();
}

GXVoid GXMat4::Inverse ( const GXMat4 &sourceMatrix )
{
    // 2x2 sub-determinants required to calculate 4x4 determinant
    GXFloat det2_01_01 = sourceMatrix._m[ 0u ][ 0u ] * sourceMatrix._m[ 1u ][ 1u ] - sourceMatrix._m[ 1u ][ 0u ] * sourceMatrix._m[ 0u ][ 1u ];
    GXFloat det2_01_02 = sourceMatrix._m[ 0u ][ 0u ] * sourceMatrix._m[ 2u ][ 1u ] - sourceMatrix._m[ 2u ][ 0u ] * sourceMatrix._m[ 0u ][ 1u ];
    GXFloat det2_01_03 = sourceMatrix._m[ 0u ][ 0u ] * sourceMatrix._m[ 3u ][ 1u ] - sourceMatrix._m[ 3u ][ 0u ] * sourceMatrix._m[ 0u ][ 1u ];
    GXFloat det2_01_12 = sourceMatrix._m[ 1u ][ 0u ] * sourceMatrix._m[ 2u ][ 1u ] - sourceMatrix._m[ 2u ][ 0u ] * sourceMatrix._m[ 1u ][ 1u ];
    GXFloat det2_01_13 = sourceMatrix._m[ 1u ][ 0u ] * sourceMatrix._m[ 3u ][ 1u ] - sourceMatrix._m[ 3u ][ 0u ] * sourceMatrix._m[ 1u ][ 1u ];
    GXFloat det2_01_23 = sourceMatrix._m[ 2u ][ 0u ] * sourceMatrix._m[ 3u ][ 1u ] - sourceMatrix._m[ 3u ][ 0u ] * sourceMatrix._m[ 2u ][ 1u ];

    // 3x3 sub-determinants required to calculate 4x4 determinant
    GXFloat det3_201_012 = sourceMatrix._m[ 0u ][ 2u ] * det2_01_12 - sourceMatrix._m[ 1u ][ 2u ] * det2_01_02 + sourceMatrix._m[ 2u ][ 2u ] * det2_01_01;
    GXFloat det3_201_013 = sourceMatrix._m[ 0u ][ 2u ] * det2_01_13 - sourceMatrix._m[ 1u ][ 2u ] * det2_01_03 + sourceMatrix._m[ 3u ][ 2u ] * det2_01_01;
    GXFloat det3_201_023 = sourceMatrix._m[ 0u ][ 2u ] * det2_01_23 - sourceMatrix._m[ 2u ][ 2u ] * det2_01_03 + sourceMatrix._m[ 3u ][ 2u ] * det2_01_02;
    GXFloat det3_201_123 = sourceMatrix._m[ 1u ][ 2u ] * det2_01_23 - sourceMatrix._m[ 2u ][ 2u ] * det2_01_13 + sourceMatrix._m[ 3u ][ 2u ] * det2_01_12;

    GXFloat inverseDeterminant = 1.0f / ( -det3_201_123 * sourceMatrix._m[ 0u ][ 3u ] + det3_201_023 * sourceMatrix._m[ 1u ][ 3u ] - det3_201_013 * sourceMatrix._m[ 2u ][ 3u ] + det3_201_012 * sourceMatrix._m[ 3u ][ 3u ] );

    // remaining 2x2 sub-determinants
    GXFloat det2_03_01 = sourceMatrix._m[ 0u ][ 0u ] * sourceMatrix._m[ 1u ][ 3u ] - sourceMatrix._m[ 1u ][ 0u ] * sourceMatrix._m[ 0u ][ 3u ];
    GXFloat det2_03_02 = sourceMatrix._m[ 0u ][ 0u ] * sourceMatrix._m[ 2u ][ 3u ] - sourceMatrix._m[ 2u ][ 0u ] * sourceMatrix._m[ 0u ][ 3u ];
    GXFloat det2_03_03 = sourceMatrix._m[ 0u ][ 0u ] * sourceMatrix._m[ 3u ][ 3u ] - sourceMatrix._m[ 3u ][ 0u ] * sourceMatrix._m[ 0u ][ 3u ];
    GXFloat det2_03_12 = sourceMatrix._m[ 1u ][ 0u ] * sourceMatrix._m[ 2u ][ 3u ] - sourceMatrix._m[ 2u ][ 0u ] * sourceMatrix._m[ 1u ][ 3u ];
    GXFloat det2_03_13 = sourceMatrix._m[ 1u ][ 0u ] * sourceMatrix._m[ 3u ][ 3u ] - sourceMatrix._m[ 3u ][ 0u ] * sourceMatrix._m[ 1u ][ 3u ];
    GXFloat det2_03_23 = sourceMatrix._m[ 2u ][ 0u ] * sourceMatrix._m[ 3u ][ 3u ] - sourceMatrix._m[ 3u ][ 0u ] * sourceMatrix._m[ 2u ][ 3u ];

    GXFloat det2_13_01 = sourceMatrix._m[ 0u ][ 1u ] * sourceMatrix._m[ 1u ][ 3u ] - sourceMatrix._m[ 1u ][ 1u ] * sourceMatrix._m[ 0u ][ 3u ];
    GXFloat det2_13_02 = sourceMatrix._m[ 0u ][ 1u ] * sourceMatrix._m[ 2u ][ 3u ] - sourceMatrix._m[ 2u ][ 1u ] * sourceMatrix._m[ 0u ][ 3u ];
    GXFloat det2_13_03 = sourceMatrix._m[ 0u ][ 1u ] * sourceMatrix._m[ 3u ][ 3u ] - sourceMatrix._m[ 3u ][ 1u ] * sourceMatrix._m[ 0u ][ 3u ];
    GXFloat det2_13_12 = sourceMatrix._m[ 1u ][ 1u ] * sourceMatrix._m[ 2u ][ 3u ] - sourceMatrix._m[ 2u ][ 1u ] * sourceMatrix._m[ 1u ][ 3u ];
    GXFloat det2_13_13 = sourceMatrix._m[ 1u ][ 1u ] * sourceMatrix._m[ 3u ][ 3u ] - sourceMatrix._m[ 3u ][ 1u ] * sourceMatrix._m[ 1u ][ 3u ];
    GXFloat det2_13_23 = sourceMatrix._m[ 2u ][ 1u ] * sourceMatrix._m[ 3u ][ 3u ] - sourceMatrix._m[ 3u ][ 1u ] * sourceMatrix._m[ 2u ][ 3u ];

    // remaining 3x3 sub-determinants
    GXFloat det3_203_012 = sourceMatrix._m[ 0u ][ 2u ] * det2_03_12 - sourceMatrix._m[ 1u ][ 2u ] * det2_03_02 + sourceMatrix._m[ 2u ][ 2u ] * det2_03_01;
    GXFloat det3_203_013 = sourceMatrix._m[ 0u ][ 2u ] * det2_03_13 - sourceMatrix._m[ 1u ][ 2u ] * det2_03_03 + sourceMatrix._m[ 3u ][ 2u ] * det2_03_01;
    GXFloat det3_203_023 = sourceMatrix._m[ 0u ][ 2u ] * det2_03_23 - sourceMatrix._m[ 2u ][ 2u ] * det2_03_03 + sourceMatrix._m[ 3u ][ 2u ] * det2_03_02;
    GXFloat det3_203_123 = sourceMatrix._m[ 1u ][ 2u ] * det2_03_23 - sourceMatrix._m[ 2u ][ 2u ] * det2_03_13 + sourceMatrix._m[ 3u ][ 2u ] * det2_03_12;

    GXFloat det3_213_012 = sourceMatrix._m[ 0u ][ 2u ] * det2_13_12 - sourceMatrix._m[ 1u ][ 2u ] * det2_13_02 + sourceMatrix._m[ 2u ][ 2u ] * det2_13_01;
    GXFloat det3_213_013 = sourceMatrix._m[ 0u ][ 2u ] * det2_13_13 - sourceMatrix._m[ 1u ][ 2u ] * det2_13_03 + sourceMatrix._m[ 3u ][ 2u ] * det2_13_01;
    GXFloat det3_213_023 = sourceMatrix._m[ 0u ][ 2u ] * det2_13_23 - sourceMatrix._m[ 2u ][ 2u ] * det2_13_03 + sourceMatrix._m[ 3u ][ 2u ] * det2_13_02;
    GXFloat det3_213_123 = sourceMatrix._m[ 1u ][ 2u ] * det2_13_23 - sourceMatrix._m[ 2u ][ 2u ] * det2_13_13 + sourceMatrix._m[ 3u ][ 2u ] * det2_13_12;

    GXFloat det3_301_012 = sourceMatrix._m[ 0u ][ 3u ] * det2_01_12 - sourceMatrix._m[ 1u ][ 3u ] * det2_01_02 + sourceMatrix._m[ 2u ][ 3u ] * det2_01_01;
    GXFloat det3_301_013 = sourceMatrix._m[ 0u ][ 3u ] * det2_01_13 - sourceMatrix._m[ 1u ][ 3u ] * det2_01_03 + sourceMatrix._m[ 3u ][ 3u ] * det2_01_01;
    GXFloat det3_301_023 = sourceMatrix._m[ 0u ][ 3u ] * det2_01_23 - sourceMatrix._m[ 2u ][ 3u ] * det2_01_03 + sourceMatrix._m[ 3u ][ 3u ] * det2_01_02;
    GXFloat det3_301_123 = sourceMatrix._m[ 1u ][ 3u ] * det2_01_23 - sourceMatrix._m[ 2u ][ 3u ] * det2_01_13 + sourceMatrix._m[ 3u ][ 3u ] * det2_01_12;

    _m[ 0u ][ 0u ] = -det3_213_123 * inverseDeterminant;
    _m[ 0u ][ 1u ] = +det3_213_023 * inverseDeterminant;
    _m[ 0u ][ 2u ] = -det3_213_013 * inverseDeterminant;
    _m[ 0u ][ 3u ] = +det3_213_012 * inverseDeterminant;

    _m[ 1u ][ 0u ] = +det3_203_123 * inverseDeterminant;
    _m[ 1u ][ 1u ] = -det3_203_023 * inverseDeterminant;
    _m[ 1u ][ 2u ] = +det3_203_013 * inverseDeterminant;
    _m[ 1u ][ 3u ] = -det3_203_012 * inverseDeterminant;

    _m[ 2u ][ 0u ] = +det3_301_123 * inverseDeterminant;
    _m[ 2u ][ 1u ] = -det3_301_023 * inverseDeterminant;
    _m[ 2u ][ 2u ] = +det3_301_013 * inverseDeterminant;
    _m[ 2u ][ 3u ] = -det3_301_012 * inverseDeterminant;

    _m[ 3u ][ 0u ] = -det3_201_123 * inverseDeterminant;
    _m[ 3u ][ 1u ] = +det3_201_023 * inverseDeterminant;
    _m[ 3u ][ 2u ] = -det3_201_013 * inverseDeterminant;
    _m[ 3u ][ 3u ] = +det3_201_012 * inverseDeterminant;
}

GXVoid GXMat4::Multiply ( const GXMat4 &a, const GXMat4 &b )
{
    _m[ 0u ][ 0u ] = a._m[ 0u ][ 0u ] * b._m[ 0u ][ 0u ] + a._m[ 0u ][ 1u ] * b._m[ 1u ][ 0u ] + a._m[ 0u ][ 2u ] * b._m[ 2u ][ 0u ] + a._m[ 0u ][ 3u ] * b._m[ 3u ][ 0u ];
    _m[ 0u ][ 1u ] = a._m[ 0u ][ 0u ] * b._m[ 0u ][ 1u ] + a._m[ 0u ][ 1u ] * b._m[ 1u ][ 1u ] + a._m[ 0u ][ 2u ] * b._m[ 2u ][ 1u ] + a._m[ 0u ][ 3u ] * b._m[ 3u ][ 1u ];
    _m[ 0u ][ 2u ] = a._m[ 0u ][ 0u ] * b._m[ 0u ][ 2u ] + a._m[ 0u ][ 1u ] * b._m[ 1u ][ 2u ] + a._m[ 0u ][ 2u ] * b._m[ 2u ][ 2u ] + a._m[ 0u ][ 3u ] * b._m[ 3u ][ 2u ];
    _m[ 0u ][ 3u ] = a._m[ 0u ][ 0u ] * b._m[ 0u ][ 3u ] + a._m[ 0u ][ 1u ] * b._m[ 1u ][ 3u ] + a._m[ 0u ][ 2u ] * b._m[ 2u ][ 3u ] + a._m[ 0u ][ 3u ] * b._m[ 3u ][ 3u ];

    _m[ 1u ][ 0u ] = a._m[ 1u ][ 0u ] * b._m[ 0u ][ 0u ] + a._m[ 1u ][ 1u ] * b._m[ 1u ][ 0u ] + a._m[ 1u ][ 2u ] * b._m[ 2u ][ 0u ] + a._m[ 1u ][ 3u ] * b._m[ 3u ][ 0u ];
    _m[ 1u ][ 1u ] = a._m[ 1u ][ 0u ] * b._m[ 0u ][ 1u ] + a._m[ 1u ][ 1u ] * b._m[ 1u ][ 1u ] + a._m[ 1u ][ 2u ] * b._m[ 2u ][ 1u ] + a._m[ 1u ][ 3u ] * b._m[ 3u ][ 1u ];
    _m[ 1u ][ 2u ] = a._m[ 1u ][ 0u ] * b._m[ 0u ][ 2u ] + a._m[ 1u ][ 1u ] * b._m[ 1u ][ 2u ] + a._m[ 1u ][ 2u ] * b._m[ 2u ][ 2u ] + a._m[ 1u ][ 3u ] * b._m[ 3u ][ 2u ];
    _m[ 1u ][ 3u ] = a._m[ 1u ][ 0u ] * b._m[ 0u ][ 3u ] + a._m[ 1u ][ 1u ] * b._m[ 1u ][ 3u ] + a._m[ 1u ][ 2u ] * b._m[ 2u ][ 3u ] + a._m[ 1u ][ 3u ] * b._m[ 3u ][ 3u ];

    _m[ 2u ][ 0u ] = a._m[ 2u ][ 0u ] * b._m[ 0u ][ 0u ] + a._m[ 2u ][ 1u ] * b._m[ 1u ][ 0u ] + a._m[ 2u ][ 2u ] * b._m[ 2u ][ 0u ] + a._m[ 2u ][ 3u ] * b._m[ 3u ][ 0u ];
    _m[ 2u ][ 1u ] = a._m[ 2u ][ 0u ] * b._m[ 0u ][ 1u ] + a._m[ 2u ][ 1u ] * b._m[ 1u ][ 1u ] + a._m[ 2u ][ 2u ] * b._m[ 2u ][ 1u ] + a._m[ 2u ][ 3u ] * b._m[ 3u ][ 1u ];
    _m[ 2u ][ 2u ] = a._m[ 2u ][ 0u ] * b._m[ 0u ][ 2u ] + a._m[ 2u ][ 1u ] * b._m[ 1u ][ 2u ] + a._m[ 2u ][ 2u ] * b._m[ 2u ][ 2u ] + a._m[ 2u ][ 3u ] * b._m[ 3u ][ 2u ];
    _m[ 2u ][ 3u ] = a._m[ 2u ][ 0u ] * b._m[ 0u ][ 3u ] + a._m[ 2u ][ 1u ] * b._m[ 1u ][ 3u ] + a._m[ 2u ][ 2u ] * b._m[ 2u ][ 3u ] + a._m[ 2u ][ 3u ] * b._m[ 3u ][ 3u ];

    _m[ 3u ][ 0u ] = a._m[ 3u ][ 0u ] * b._m[ 0u ][ 0u ] + a._m[ 3u ][ 1u ] * b._m[ 1u ][ 0u ] + a._m[ 3u ][ 2u ] * b._m[ 2u ][ 0u ] + a._m[ 3u ][ 3u ] * b._m[ 3u ][ 0u ];
    _m[ 3u ][ 1u ] = a._m[ 3u ][ 0u ] * b._m[ 0u ][ 1u ] + a._m[ 3u ][ 1u ] * b._m[ 1u ][ 1u ] + a._m[ 3u ][ 2u ] * b._m[ 2u ][ 1u ] + a._m[ 3u ][ 3u ] * b._m[ 3u ][ 1u ];
    _m[ 3u ][ 2u ] = a._m[ 3u ][ 0u ] * b._m[ 0u ][ 2u ] + a._m[ 3u ][ 1u ] * b._m[ 1u ][ 2u ] + a._m[ 3u ][ 2u ] * b._m[ 2u ][ 2u ] + a._m[ 3u ][ 3u ] * b._m[ 3u ][ 2u ];
    _m[ 3u ][ 3u ] = a._m[ 3u ][ 0u ] * b._m[ 0u ][ 3u ] + a._m[ 3u ][ 1u ] * b._m[ 1u ][ 3u ] + a._m[ 3u ][ 2u ] * b._m[ 2u ][ 3u ] + a._m[ 3u ][ 3u ] * b._m[ 3u ][ 3u ];
}

GXVoid GXMat4::MultiplyVectorMatrix ( GXVec4 &out, const GXVec4 &v ) const
{
    out._data[ 0u ] = v._data[ 0u ] * _m[ 0u ][ 0u ] + v._data[ 1u ] * _m[ 1u ][ 0u ] + v._data[ 2u ] * _m[ 2u ][ 0u ] + v._data[ 3u ] * _m[ 3u ][ 0u ];
    out._data[ 1u ] = v._data[ 0u ] * _m[ 0u ][ 1u ] + v._data[ 1u ] * _m[ 1u ][ 1u ] + v._data[ 2u ] * _m[ 2u ][ 1u ] + v._data[ 3u ] * _m[ 3u ][ 1u ];
    out._data[ 2u ] = v._data[ 0u ] * _m[ 0u ][ 2u ] + v._data[ 1u ] * _m[ 1u ][ 2u ] + v._data[ 2u ] * _m[ 2u ][ 2u ] + v._data[ 3u ] * _m[ 3u ][ 2u ];
    out._data[ 3u ] = v._data[ 0u ] * _m[ 0u ][ 3u ] + v._data[ 1u ] * _m[ 1u ][ 3u ] + v._data[ 2u ] * _m[ 2u ][ 3u ] + v._data[ 3u ] * _m[ 3u ][ 3u ];
}

GXVoid GXMat4::MultiplyMatrixVector ( GXVec4 &out, const GXVec4 &v ) const
{
    out._data[ 0u ] = _m[ 0u ][ 0u ] * v._data[ 0u ] + _m[ 0u ][ 1u ] * v._data[ 1u ] + _m[ 0u ][ 2u ] * v._data[ 2u ] + _m[ 0u ][ 3u ] * v._data[ 3u ];
    out._data[ 1u ] = _m[ 1u ][ 0u ] * v._data[ 0u ] + _m[ 1u ][ 1u ] * v._data[ 1u ] + _m[ 1u ][ 2u ] * v._data[ 2u ] + _m[ 1u ][ 3u ] * v._data[ 3u ];
    out._data[ 2u ] = _m[ 2u ][ 0u ] * v._data[ 0u ] + _m[ 2u ][ 1u ] * v._data[ 1u ] + _m[ 2u ][ 2u ] * v._data[ 2u ] + _m[ 2u ][ 3u ] * v._data[ 3u ];
    out._data[ 3u ] = _m[ 3u ][ 0u ] * v._data[ 0u ] + _m[ 3u ][ 1u ] * v._data[ 1u ] + _m[ 3u ][ 2u ] * v._data[ 2u ] + _m[ 3u ][ 3u ] * v._data[ 3u ];
}

GXVoid GXMat4::MultiplyAsNormal ( GXVec3 &out, const GXVec3 &v ) const
{
    out._data[ 0u ] = v._data[ 0u ] * _m[ 0u ][ 0u ] + v._data[ 1u ] * _m[ 1u ][ 0u ] + v._data[ 2u ] * _m[ 2u ][ 0u ];
    out._data[ 1u ] = v._data[ 0u ] * _m[ 0u ][ 1u ] + v._data[ 1u ] * _m[ 1u ][ 1u ] + v._data[ 2u ] * _m[ 2u ][ 1u ];
    out._data[ 2u ] = v._data[ 0u ] * _m[ 0u ][ 2u ] + v._data[ 1u ] * _m[ 1u ][ 2u ] + v._data[ 2u ] * _m[ 2u ][ 2u ];
}

GXVoid GXMat4::MultiplyAsPoint ( GXVec3 &out, const GXVec3 &v ) const
{
    out._data[ 0u ] = v._data[ 0u ] * _m[ 0u ][ 0u ] + v._data[ 1u ] * _m[ 1u ][ 0u ] + v._data[ 2u ] * _m[ 2u ][ 0u ] + _m[ 3u ][ 0u ];
    out._data[ 1u ] = v._data[ 0u ] * _m[ 0u ][ 1u ] + v._data[ 1u ] * _m[ 1u ][ 1u ] + v._data[ 2u ] * _m[ 2u ][ 1u ] + _m[ 3u ][ 1u ];
    out._data[ 2u ] = v._data[ 0u ] * _m[ 0u ][ 2u ] + v._data[ 1u ] * _m[ 1u ][ 2u ] + v._data[ 2u ] * _m[ 2u ][ 2u ] + _m[ 3u ][ 2u ];
}

GXMat4& GXMat4::operator = ( const GXMat4 &matrix )
{
    memcpy ( this, &matrix, sizeof ( GXMat4 ) );
    return *this;
}

//---------------------------------------------------------------------------------------------------------------------

GXAABB::GXAABB ()
{
    Empty ();
}

GXAABB::GXAABB ( const GXAABB &other )
{
    memcpy ( this, &other, sizeof ( GXAABB ) );
}

GXVoid GXAABB::Empty ()
{
    _vertices = 0u;

    _min = GXVec3 ( FLT_MAX, FLT_MAX, FLT_MAX );
    _max = GXVec3 ( -FLT_MAX, -FLT_MAX, -FLT_MAX );
}

GXVoid GXAABB::Transform ( GXAABB &bounds, const GXMat4 &transform ) const
{
    GXVec3 verticesLocal[ 8u ];
    verticesLocal[ 0u ].Init ( _min._data[ 0u ], _min._data[ 1u ], _min._data[ 2u ] );
    verticesLocal[ 1u ].Init ( _max._data[ 0u ], _min._data[ 1u ], _min._data[ 2u ] );
    verticesLocal[ 2u ].Init ( _max._data[ 0u ], _min._data[ 1u ], _max._data[ 2u ] );
    verticesLocal[ 3u ].Init ( _min._data[ 0u ], _min._data[ 1u ], _max._data[ 2u ] );

    verticesLocal[ 4u ].Init ( _min._data[ 0u ], _max._data[ 1u ], _min._data[ 2u ] );
    verticesLocal[ 5u ].Init ( _max._data[ 0u ], _max._data[ 1u ], _min._data[ 2u ] );
    verticesLocal[ 6u ].Init ( _max._data[ 0u ], _max._data[ 1u ], _max._data[ 2u ] );
    verticesLocal[ 7u ].Init ( _min._data[ 0u ], _max._data[ 1u ], _max._data[ 2u ] );

    bounds.Empty ();

    for ( GXUByte i = 0u; i < 8u; ++i )
    {
        GXVec3 vertex;
        transform.MultiplyAsPoint ( vertex, verticesLocal[ i ] );
        bounds.AddVertex ( vertex );
    }
}

GXVoid GXAABB::AddVertex ( const GXVec3 &vertex )
{
    AddVertex ( vertex._data[ 0u ], vertex._data[ 1u ], vertex._data[ 2u ] );
}

GXVoid GXAABB::AddVertex ( GXFloat x, GXFloat y, GXFloat z )
{
    if ( _vertices > 1u )
    {
        if ( _min._data[ 0u ] > x )
            _min._data[ 0u ] = x;
        else if ( _max._data[ 0u ] < x )
            _max._data[ 0u ] = x;

        if ( _min._data[ 1u ] > y )
            _min._data[ 1u ] = y;
        else if ( _max._data[ 1u ] < y )
            _max._data[ 1u ] = y;

        if ( _min._data[ 2u ] > z )
            _min._data[ 2u ] = z;
        else if ( _max._data[ 2u ] < z )
            _max._data[ 2u ] = z;

        return;
    }

    if ( _vertices == 0u )
    {
        ++_vertices;

        _min.Init ( x, y, z );
        return;
    }

    ++_vertices;

    if ( _min._data[ 0u ] > x )
    {
        _max._data[ 0u ] = _min._data[ 0u ];
        _min._data[ 0u ] = x;
    }
    else
    {
        _max._data[ 0u ] = x;
    }

    if ( _min._data[ 1u ] > y )
    {
        _max._data[ 1u ] = _min._data[ 1u ];
        _min._data[ 1u ] = y;
    }
    else
    {
        _max._data[ 1u ] = y;
    }

    if ( _min._data[ 2u ]  > z )
    {
        _max._data[ 2u ] = _min._data[ 2u ];
        _min._data[ 2u ] = z;
    }
    else
    {
        _max._data[ 2u ] = z;
    }
}

GXBool GXAABB::IsOverlaped ( const GXAABB &other ) const
{
    if ( _min._data[ 0u ] > other._max._data[ 0u ] )
        return GX_FALSE;

    if ( _min._data[ 1u ] > other._max._data[ 1u ] )
        return GX_FALSE;

    if ( _min._data[ 2u ] > other._max._data[ 2u ] )
        return GX_FALSE;

    if ( other._min._data[ 0u ] > _max._data[ 0u ] )
        return GX_FALSE;

    if ( other._min._data[ 1u ] > _max._data[ 1u ] )
        return GX_FALSE;

    return other._min._data[ 2u ] <= _max._data[ 2u ];
}

GXBool GXAABB::IsOverlaped ( const GXVec3 &point ) const
{
    return IsOverlaped ( point._data[ 0u ], point._data[ 1u ], point._data[ 2u ] );
}

GXBool GXAABB::IsOverlaped ( GXFloat x, GXFloat y, GXFloat z ) const
{
    if ( _min._data[ 0u ] > x )
        return GX_FALSE;

    if ( _min._data[ 1u ] > y )
        return GX_FALSE;

    if ( _min._data[ 2u ] > z )
        return GX_FALSE;

    if ( _max._data[ 0u ] < x )
        return GX_FALSE;

    if ( _max._data[ 1u ] < y )
        return GX_FALSE;

    return _max._data[ 2u ] >= z;
}

GXVoid GXAABB::GetCenter ( GXVec3 &center ) const
{
    center._data[ 0u ] = 0.5f * ( _min._data[ 0u ] + _max._data[ 0u ] );
    center._data[ 1u ] = 0.5f * ( _min._data[ 1u ] + _max._data[ 1u ] );
    center._data[ 2u ] = 0.5f * ( _min._data[ 2u ] + _max._data[ 2u ] );
}

GXFloat GXAABB::GetWidth () const
{
    if ( _vertices < 2u )
        return -1.0f;

    return _max._data[ 0u ] - _min._data[ 0u ];
}

GXFloat GXAABB::GetHeight () const
{
    if ( _vertices < 2u )
        return -1.0f;

    return _max._data[ 1u ] - _min._data[ 1u ];
}

GXFloat GXAABB::GetDepth () const
{
    if ( _vertices < 2u )
        return -1.0f;

    return _max._data[ 2u ] - _min._data[ 2u ];
}

GXFloat GXAABB::GetSphereRadius () const
{
    GXVec3 center;
    GetCenter ( center );
    return center.Distance ( _min );
}

GXAABB& GXAABB::operator = ( const GXAABB &other )
{
    memcpy ( this, &other, sizeof ( GXAABB ) );
    return *this;
}

//------------------------------------------------------------------

GXPlane::GXPlane ()
{
    _a = 0.0f;
    _b = 1.0f;
    _c = 0.0f;
    _d = 0.0f;
}

GXPlane::GXPlane ( const GXPlane &other )
{
    memcpy ( this, &other, sizeof ( GXPlane ) );
}

GXVoid GXPlane::From ( const GXVec3 &pointA, const GXVec3 &pointB, const GXVec3 &pointC )
{
    GXVec3 ab;
    ab.Substract ( pointB, pointA );

    GXVec3 ac;
    ac.Substract ( pointC, pointA );

    GXVec3 normal;
    normal.CrossProduct ( ab, ac );
    normal.Normalize ();

    _a = normal._data[ 0u ];
    _b = normal._data[ 1u ];
    _c = normal._data[ 2u ];
    _d = -normal.DotProduct ( pointA );
}

GXVoid GXPlane::FromLineToPoint ( const GXVec3 &lineStart, const GXVec3 &lineEnd, const GXVec3 &point )
{
    GXVec3 startToPoint;
    startToPoint.Substract ( point, lineStart );

    GXVec3 startToEnd;
    startToEnd.Substract ( lineEnd, lineStart );

    GXVec3 tempCross;
    tempCross.CrossProduct ( startToEnd, startToPoint );

    GXVec3 normal;
    normal.CrossProduct ( tempCross, startToEnd );

    _a = normal._data[ 0u ];
    _b = normal._data[ 1u ];
    _c = normal._data[ 2u ];
    _d = -_a * lineStart._data[ 0u ] - _b * lineStart._data[ 1u ] - _c * lineStart._data[ 2u ];

    if ( ClassifyVertex ( point ) != eGXPlaneClassifyVertex::Behind ) return;

    Flip ();
}

GXVoid GXPlane::Normalize ()
{
    GXFloat inverseLength = 1.0f / sqrtf ( _a * _a + _b * _b + _c * _c );

    _a *= inverseLength;
    _b *= inverseLength;
    _c *= inverseLength;
    _d *= inverseLength;
}

GXVoid GXPlane::Flip ()
{
    _a = -_a;
    _b = -_b;
    _c = -_c;
    _d = -_d;
}

eGXPlaneClassifyVertex GXPlane::ClassifyVertex ( const GXVec3 &vertex ) const
{
    return ClassifyVertex ( vertex._data[ 0u ], vertex._data[ 1u ], vertex._data[ 2u ] );
}

eGXPlaneClassifyVertex GXPlane::ClassifyVertex ( GXFloat x, GXFloat y, GXFloat z ) const
{
    GXFloat test = _a * x + _b * y + _c * z + _d;

    if ( test < 0.0f )
        return eGXPlaneClassifyVertex::Behind;

    if ( test > 0.0f )
        return eGXPlaneClassifyVertex::InFront;

    return eGXPlaneClassifyVertex::On;
}

GXPlane& GXPlane::operator = ( const GXPlane &other )
{
    memcpy ( this, &other, sizeof ( GXPlane ) );
    return *this;
}

//---------------------------------------------------------------------------------------------------------------------

GXProjectionClipPlanes::GXProjectionClipPlanes ()
{
    memset ( _planes, 0, 6u * sizeof ( GXPlane ) );
}

GXProjectionClipPlanes::GXProjectionClipPlanes ( const GXMat4 &src )
{
    From ( src );
}

GXVoid GXProjectionClipPlanes::From ( const GXMat4 &src )
{
    // Left clipping plane
    _planes[ 0u ]._a = src._m[ 0u ][ 3u ] + src._m[ 0u ][ 0u ];
    _planes[ 0u ]._b = src._m[ 1u ][ 3u ] + src._m[ 1u ][ 0u ];
    _planes[ 0u ]._c = src._m[ 2u ][ 3u ] + src._m[ 2u ][ 0u ];
    _planes[ 0u ]._d = src._m[ 3u ][ 3u ] + src._m[ 3u ][ 0u ];
    
    // Right clipping plane
    _planes[ 1u ]._a = src._m[ 0u ][ 3u ] - src._m[ 0u ][ 0u ];
    _planes[ 1u ]._b = src._m[ 1u ][ 3u ] - src._m[ 1u ][ 0u ];
    _planes[ 1u ]._c = src._m[ 2u ][ 3u ] - src._m[ 2u ][ 0u ];
    _planes[ 1u ]._d = src._m[ 3u ][ 3u ] - src._m[ 3u ][ 0u ];
    
    // Top clipping plane
    _planes[ 2u ]._a = src._m[ 0u ][ 3u ] - src._m[ 0u ][ 1u ];
    _planes[ 2u ]._b = src._m[ 1u ][ 3u ] - src._m[ 1u ][ 1u ];
    _planes[ 2u ]._c = src._m[ 2u ][ 3u ] - src._m[ 2u ][ 1u ];
    _planes[ 2u ]._d = src._m[ 3u ][ 3u ] - src._m[ 3u ][ 1u ];

    // Bottom clipping plane
    _planes[ 3u ]._a = src._m[ 0u ][ 3u ] + src._m[ 0u ][ 1u ];
    _planes[ 3u ]._b = src._m[ 1u ][ 3u ] + src._m[ 1u ][ 1u ];
    _planes[ 3u ]._c = src._m[ 2u ][ 3u ] + src._m[ 2u ][ 1u ];
    _planes[ 3u ]._d = src._m[ 3u ][ 3u ] + src._m[ 3u ][ 1u ];

    // Near clipping plane
    _planes[ 4u ]._a = src._m[ 0u ][ 3u ] + src._m[ 0u ][ 2u ];
    _planes[ 4u ]._b = src._m[ 1u ][ 3u ] + src._m[ 1u ][ 2u ];
    _planes[ 4u ]._c = src._m[ 2u ][ 3u ] + src._m[ 2u ][ 2u ];
    _planes[ 4u ]._d = src._m[ 3u ][ 3u ] + src._m[ 3u ][ 2u ];

    // Far clipping plane
    _planes[ 5u ]._a = src._m[ 0u ][ 3u ] - src._m[ 0u ][ 2u ];
    _planes[ 5u ]._b = src._m[ 1u ][ 3u ] - src._m[ 1u ][ 2u ];
    _planes[ 5u ]._c = src._m[ 2u ][ 3u ] - src._m[ 2u ][ 2u ];
    _planes[ 5u ]._d = src._m[ 3u ][ 3u ] - src._m[ 3u ][ 2u ];
}

GXBool GXProjectionClipPlanes::IsVisible ( const GXAABB &bounds ) const
{
    GXInt flags = static_cast<GXInt> ( PlaneTest ( bounds._min._data[ 0u ], bounds._min._data[ 1u ], bounds._min._data[ 2u ] ) );
    flags &= static_cast<GXInt> ( PlaneTest ( bounds._min._data[ 0u ], bounds._max._data[ 1u ], bounds._min._data[ 2u ] ) );
    flags &= static_cast<GXInt> ( PlaneTest ( bounds._max._data[ 0u ], bounds._max._data[ 1u ], bounds._min._data[ 2u ] ) );
    flags &= static_cast<GXInt> ( PlaneTest ( bounds._max._data[ 0u ], bounds._min._data[ 1u ], bounds._min._data[ 2u ] ) );

    flags &= static_cast<GXInt> ( PlaneTest ( bounds._min._data[ 0u ], bounds._min._data[ 1u ], bounds._max._data[ 2u ] ) );
    flags &= static_cast<GXInt> ( PlaneTest ( bounds._min._data[ 0u ], bounds._max._data[ 1u ], bounds._max._data[ 2u ] ) );
    flags &= static_cast<GXInt> ( PlaneTest ( bounds._max._data[ 0u ], bounds._max._data[ 1u ], bounds._max._data[ 2u ] ) );
    flags &= static_cast<GXInt> ( PlaneTest ( bounds._max._data[ 0u ], bounds._min._data[ 1u ], bounds._max._data[ 2u ] ) );

    return flags <= 0;
}

GXProjectionClipPlanes& GXProjectionClipPlanes::operator = ( const GXProjectionClipPlanes &clipPlanes )
{
    memcpy ( this, &clipPlanes, sizeof ( GXProjectionClipPlanes ) );
    return *this;
}

GXUByte GXProjectionClipPlanes::PlaneTest ( GXFloat x, GXFloat y, GXFloat z ) const
{
    GXUByte flags = 0u;

    for ( GXUByte i = 0u; i < 6u; ++i )
    {
        if ( _planes[ i ].ClassifyVertex ( x, y, z ) != eGXPlaneClassifyVertex::Behind ) continue;

        flags |= static_cast<GXUByte> ( 1u << i );
    }

    return flags;
}

//------------------------------------------------------------------

GXFloat GXCALL GXDegToRad ( GXFloat degrees )
{
    return degrees * DEGREES_TO_RADIANS_FACTOR;
}

GXFloat GXCALL GXRadToDeg ( GXFloat radians )
{
    return radians * RADIANS_TO_DEGREES_FACTOR;
}

GXVoid GXCALL GXConvert3DSMaxToGXEngine ( GXVec3 &gx_out, GXFloat max_x, GXFloat max_y, GXFloat max_z )
{
    gx_out._data[ 0u ] = -max_x;
    gx_out._data[ 1u ] = max_z;
    gx_out._data[ 2u ] = -max_y;
}

GXVoid GXCALL GXRandomize ()
{
    srand ( static_cast<GXUInt> ( time ( nullptr ) ) );
}

GXFloat GXCALL GXRandomNormalize ()
{
    return static_cast<GXFloat> ( rand () ) * INVERSE_RAND_MAX;
}

GXFloat GXCALL GXRandomBetween ( GXFloat from, GXFloat to )
{
    GXFloat delta = to - from;
    return from + delta * GXRandomNormalize ();
}

GXVoid GXCALL GXRandomBetween ( GXVec3 &out, const GXVec3 &from, const GXVec3 &to )
{
    out._data[ 0u ] = GXRandomBetween ( from._data[ 0u ], to._data[ 0u ] );
    out._data[ 1u ] = GXRandomBetween ( from._data[ 1u ], to._data[ 1u ] );
    out._data[ 2u ] = GXRandomBetween ( from._data[ 2u ], to._data[ 2u ] );
}

GXVoid GXCALL GXGetTangentBitangent ( GXVec3 &outTangent, GXVec3 &outBitangent, GXUByte vertexID, const GXUByte* vertices, GXUPointer vertexStride, const GXUByte* uvs, GXUPointer uvStride )
{
    const GXVec3* v0;
    const GXVec3* v1;
    const GXVec3* v2;

    const GXVec2* uv0;
    const GXVec2* uv1;
    const GXVec2* uv2;

    switch ( vertexID )
    {
        case 0u:
        {
            v0 = reinterpret_cast<const GXVec3*> ( vertices );
            v1 = reinterpret_cast<const GXVec3*> ( vertices + vertexStride );
            v2 = reinterpret_cast<const GXVec3*> ( vertices + 2u * vertexStride );

            uv0 = reinterpret_cast<const GXVec2*> ( uvs );
            uv1 = reinterpret_cast<const GXVec2*> ( uvs + uvStride );
            uv2 = reinterpret_cast<const GXVec2*> ( uvs + 2u * uvStride );
        }
        break;

        case 1u:
        {
            v0 = reinterpret_cast<const GXVec3*> ( vertices + vertexStride );
            v1 = reinterpret_cast<const GXVec3*> ( vertices + 2u * vertexStride );
            v2 = reinterpret_cast<const GXVec3*> ( vertices );

            uv0 = reinterpret_cast<const GXVec2*> ( uvs + uvStride );
            uv1 = reinterpret_cast<const GXVec2*> ( uvs + 2u * uvStride );
            uv2 = reinterpret_cast<const GXVec2*> ( uvs );
        }
        break;

        case 2u:
        default:
        {
            v0 = reinterpret_cast<const GXVec3*> ( vertices + 2u * vertexStride );
            v1 = reinterpret_cast<const GXVec3*> ( vertices );
            v2 = reinterpret_cast<const GXVec3*> ( vertices + vertexStride );

            uv0 = reinterpret_cast<const GXVec2*> ( uvs + 2u * uvStride );
            uv1 = reinterpret_cast<const GXVec2*> ( uvs );
            uv2 = reinterpret_cast<const GXVec2*> ( uvs + uvStride );
        }
        break;
    }

    GXVec3 a;
    GXVec3 b;
    a.Substract ( *v1, *v0 );
    b.Substract ( *v2, *v0 );

    GXVec2 dUVa;
    GXVec2 dUVb;
    dUVa.Substract ( *uv1, *uv0 );
    dUVb.Substract ( *uv2, *uv0 );

    GXFloat factor = 1.0f / ( dUVa._data[ 0u ] * dUVb._data[ 1u ] - dUVb._data[ 0u ] * dUVa._data[ 1u ] );

    outTangent._data[ 0u ] = factor * ( dUVb._data[ 1u ] * a._data[ 0u ] - dUVa._data[ 1u ] * b._data[ 0u ] );
    outTangent._data[ 1u ] = factor * ( dUVb._data[ 1u ] * a._data[ 1u ] - dUVa._data[ 1u ] * b._data[ 1u ] );
    outTangent._data[ 2u ] = factor * ( dUVb._data[ 1u ] * a._data[ 2u ] - dUVa._data[ 1u ] * b._data[ 2u ] );
    outTangent.Normalize ();

    outBitangent._data[ 0u ] = factor * ( -dUVb._data[ 0u ] * a._data[ 0u ] + dUVa._data[ 0u ] * b._data[ 0u ] );
    outBitangent._data[ 1u ] = factor * ( -dUVb._data[ 0u ] * a._data[ 1u ] + dUVa._data[ 0u ] * b._data[ 1u ] );
    outBitangent._data[ 2u ] = factor * ( -dUVb._data[ 0u ] * a._data[ 2u ] + dUVa._data[ 0u ] * b._data[ 2u ] );
    outBitangent.Normalize ();
}

GXFloat GXCALL GXClampf ( GXFloat value, GXFloat minValue, GXFloat maxValue )
{
    return ( value < minValue ) ? minValue : ( value > maxValue ) ? maxValue : value;
}

GXInt GXCALL GXClampi ( GXInt value, GXInt minValue, GXInt maxValue )
{
    return ( value < minValue ) ? minValue : ( value > maxValue ) ? maxValue : value;
}

GXFloat GXCALL GXMinf ( GXFloat a, GXFloat b )
{
    return a < b ? a : b;
}

GXFloat GXCALL GXMaxf ( GXFloat a, GXFloat b )
{
    return a > b ? a : b;
}

GXVoid GXCALL GXGetBarycentricCoords ( GXVec3 &out, const GXVec3 &point, const GXVec3 &a, const GXVec3 &b, const GXVec3 &c )
{
    GXVec3 v0;
    GXVec3 v1;
    GXVec3 v2;

    v0.Substract ( b, a );
    v1.Substract ( c, a );
    v2.Substract ( point, a );

    GXFloat d00 = v0.DotProduct ( v0 );
    GXFloat d01 = v0.DotProduct ( v1 );
    GXFloat d11 = v1.DotProduct ( v1 );
    GXFloat d20 = v2.DotProduct ( v0 );
    GXFloat d21 = v2.DotProduct ( v1 );

    GXFloat denom = 1.0f / ( d00 * d11 - d01 * d01 );

    out._data[ 1u ] = ( d11 * d20 - d01 * d21 ) * denom;
    out._data[ 2u ] = ( d00 * d21 - d01 * d20 ) * denom;
    out._data[ 0u ] = 1.0f - out._data[ 1u ] - out._data[ 2u ];
}

GXVoid GXCALL GXGetRayFromViewer ( GXVec3 &origin, GXVec3 &direction, GXUShort x, GXUShort y, GXUShort viewportWidth, GXUShort viewportHeight, const GXVec3& viewerLocation, const GXMat4 &viewProjectionMatrix )
{
    GXFloat halfWidth = viewportWidth * 0.5f;
    GXFloat halfHeight = viewportHeight * 0.5f;

    GXVec4 pointCVV ( ( static_cast<GXFloat> ( x ) - halfWidth ) / halfWidth, ( static_cast<GXFloat> ( y ) - halfHeight ) / halfHeight, 1.0f, 1.0f );

    GXMat4 inverseViewProjectionMatrix;
    inverseViewProjectionMatrix.Inverse ( viewProjectionMatrix );

    GXVec4 pointWorld;
    inverseViewProjectionMatrix.MultiplyVectorMatrix ( pointWorld, pointCVV );
    GXFloat alpha = 1.0f / pointWorld._data[ 3u ];

    pointWorld._data[ 0u ] *= alpha;
    pointWorld._data[ 1u ] *= alpha;
    pointWorld._data[ 2u ] *= alpha;

    direction.Substract ( GXVec3 ( pointWorld._data[ 0u ], pointWorld._data[ 1u ], pointWorld._data[ 2u ] ), viewerLocation );
    direction.Normalize ();

    origin = viewerLocation;
}
