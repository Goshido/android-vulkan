// version 1.56

#include <GXCommon/GXMath.h>
#include <GXCommon/GXWarning.h>

GX_DISABLE_COMMON_WARNINGS

#include <cassert>
#include <cwchar>
#include <cstring>
#include <stdlib.h>
#include <time.h>

GX_RESTORE_WARNING_STATE


#define COLOR_TO_FLOAT_FACTOR               0.00392157f

#define HSVA_FACTOR                         0.016666f
#define HSVA_TO_RGBA_FACTOR                 2.55f
#define HSVA_ALPHA_TO_RGBA_ALPHA_BYTE       2.55f
#define HSVA_TO_RGBA_FLOAT                  0.01f
#define RGBA_TO_UBYTE_FACTOR                255.0f

#define DEGREES_TO_RADIANS_FACTOR           0.0174533f
#define RADIANS_TO_DEGREES_FACTOR           57.295779f

#define FLOAT_EPSILON                       1.0e-4f
#define INVERSE_RAND_MAX                    3.05185e-5f

#define SOLUTION_ALPHA                      0u
#define SOLUTION_BETTA                      1u
#define SOLUTION_GAMMA                      2u
#define SOLUTION_YOTTA                      3u
#define UNKNOWN_SOLUTION                    0xFFu

//---------------------------------------------------------------------------------------------------------------------

GXVec2::GXVec2 ()
{
    memset ( this, 0, sizeof ( GXVec2 ) );
}

GXVec2::GXVec2 ( const GXVec2 &other )
{
    memcpy ( this, &other, sizeof ( GXVec2 ) );
}

GXVoid GXVec2::SetX ( GXFloat x )
{
    _data[ 0u ] = x;
}

GXFloat GXVec2::GetX () const
{
    return _data[ 0u ];
}

GXVoid GXVec2::SetY ( GXFloat y )
{
    _data[ 1u ] = y;
}

GXFloat GXVec2::GetY () const
{
    return _data[ 1u ];
}

GXVoid GXVec2::Init ( GXFloat x, GXFloat y )
{
    _data[ 0u ] = x;
    _data[ 1u ] = y;
}

GXVoid GXVec2::Normalize ()
{
    Multiply ( *this, 1.0f / Length () );
}

GXVoid GXVec2::CalculateNormalFast ( const GXVec2 &a, const GXVec2 &b )
{
    _data[ 0u ] = a._data[ 1u ] - b._data[ 1u ];
    _data[ 1u ] = b._data[ 0u ] - a._data[ 0u ];
}

GXVoid GXVec2::CalculateNormal ( const GXVec2 &a, const GXVec2 &b )
{
    CalculateNormalFast ( a, b );
    Normalize ();
}

GXVoid GXVec2::Sum ( const GXVec2 &a, const GXVec2 &b )
{
    _data[ 0u ] = a._data[ 0u ] + b._data[ 0u ];
    _data[ 1u ] = a._data[ 1u ] + b._data[ 1u ];
}

GXVoid GXVec2::Sum ( const GXVec2 &a, GXFloat bScale, const GXVec2 &b )
{
    _data[ 0u ] = a._data[ 0u ] + bScale * b._data[ 0u ];
    _data[ 1u ] = a._data[ 1u ] + bScale * b._data[ 1u ];
}

GXVoid GXVec2::Substract ( const GXVec2 &a, const GXVec2 &b )
{
    _data[ 0u ] = a._data[ 0u ] - b._data[ 0u ];
    _data[ 1u ] = a._data[ 1u ] - b._data[ 1u ];
}

GXVoid GXVec2::Multiply ( const GXVec2 &a, const GXVec2 &b )
{
    _data[ 0u ] = a._data[ 0u ] * b._data[ 0u ];
    _data[ 1u ] = a._data[ 1u ] * b._data[ 1u ];
}

GXVoid GXVec2::Multiply ( const GXVec2 &v, GXFloat scale )
{
    _data[ 0u ] = v._data[ 0u ] * scale;
    _data[ 1u ] = v._data[ 1u ] * scale;
}

GXFloat GXVec2::DotProduct ( const GXVec2 &other ) const
{
    return _data[ 0u ] * other._data[ 0u ] + _data[ 1u ] * other._data[ 1u ];
}

GXFloat GXVec2::Length () const
{
    return sqrtf ( SquaredLength () );
}

GXFloat GXVec2::SquaredLength () const
{
    return DotProduct ( *this );
}

GXBool GXVec2::IsEqual ( const GXVec2 &other ) const
{
    if ( _data[ 0u ] != other._data[ 0u ] )
        return GX_FALSE;

    return _data[ 1u ] == other._data[ 1u ];
}

GXVec2& GXVec2::operator = ( const GXVec2 &vector )
{
    memcpy ( this, &vector, sizeof ( GXVec2 ) );
    return *this;
}

//---------------------------------------------------------------------------------------------------------------------

eGXLineRelationship GXCALL GXLineIntersection2D ( GXVec2 &intersectionPoint, const GXVec2 &a0, const GXVec2 &a1, const GXVec2 &b0, const GXVec2 &b1 )
{
    GXVec2 alpha;
    alpha.Substract ( a1, a0 );

    GXVec2 betta;
    betta.Substract ( b1, b0 );

    GXVec2 yotta ( -alpha.GetY (), alpha.GetX () );
    GXFloat omega = yotta.DotProduct ( betta );

    if ( omega == 0.0f )
    {
        GXVec2 gamma ( a0.IsEqual ( b0 ) ? b1 : b0 );

        GXVec2 zetta;
        zetta.Substract ( gamma, a0 );
        zetta.Normalize ();

        alpha.Normalize ();

        GXFloat eta = alpha.DotProduct ( zetta );

        if ( eta == 1.0f || eta == -1.0f )
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

//------------------------------------------------------------------------------------------------

GXVec3::GXVec3 ()
{
    memset ( _data, 0, sizeof ( GXVec3 ) );
}

GXVec3::GXVec3 ( const GXVec3 &other )
{
    memcpy ( this, &other, sizeof ( GXVec3 ) );
}

GXVoid GXVec3::SetX ( GXFloat x )
{
    _data[ 0u ] = x;
}

GXFloat GXVec3::GetX () const
{
    return _data[ 0u ];
}

GXVoid GXVec3::SetY ( GXFloat y )
{
    _data[ 1u ] = y;
}

GXFloat GXVec3::GetY () const
{
    return _data[ 1u ];
}

GXVoid GXVec3::SetZ ( GXFloat z )
{
    _data[ 2u ] = z;
}

GXFloat GXVec3::GetZ () const
{
    return _data[ 2u ];
}

GXVoid GXVec3::Init ( GXFloat x, GXFloat y, GXFloat z )
{
    _data[ 0u ] = x;
    _data[ 1u ] = y;
    _data[ 2u ] = z;
}

GXVoid GXVec3::Normalize ()
{
    Multiply ( *this, 1.0f / Length () );
}

GXVoid GXVec3::Reverse ()
{
    _data[ 0u ] = -_data[ 0u ];
    _data[ 1u ] = -_data[ 1u ];
    _data[ 2u ] = -_data[ 2u ];
}

GXVoid GXVec3::Sum ( const GXVec3 &a, const GXVec3 &b )
{
    _data[ 0u ] = a._data[ 0u ] + b._data[ 0u ];
    _data[ 1u ] = a._data[ 1u ] + b._data[ 1u ];
    _data[ 2u ] = a._data[ 2u ] + b._data[ 2u ];
}

GXVoid GXVec3::Sum ( const GXVec3 &a, GXFloat bScale, const GXVec3 &b )
{
    _data[ 0u ] = a._data[ 0u ] + bScale * b._data[ 0u ];
    _data[ 1u ] = a._data[ 1u ] + bScale * b._data[ 1u ];
    _data[ 2u ] = a._data[ 2u ] + bScale * b._data[ 2u ];
}

GXVoid GXVec3::Substract ( const GXVec3 &a, const GXVec3 &b )
{
    _data[ 0u ] = a._data[ 0u ] - b._data[ 0u ];
    _data[ 1u ] = a._data[ 1u ] - b._data[ 1u ];
    _data[ 2u ] = a._data[ 2u ] - b._data[ 2u ];
}

GXVoid GXVec3::Multiply ( const GXVec3 &a, GXFloat scale )
{
    _data[ 0u ] = a._data[ 0u ] * scale;
    _data[ 1u ] = a._data[ 1u ] * scale;
    _data[ 2u ] = a._data[ 2u ] * scale;
}

GXVoid GXVec3::Multiply ( const GXVec3 &a, const GXVec3 &b )
{
    _data[ 0u ] = a._data[ 0u ] * b._data[ 0u ];
    _data[ 1u ] = a._data[ 1u ] * b._data[ 1u ];
    _data[ 2u ] = a._data[ 2u ] * b._data[ 2u ];
}

GXFloat GXVec3::DotProduct ( const GXVec3 &other ) const
{
    return _data[ 0u ] * other._data[ 0u ] + _data[ 1u ] * other._data[ 1u ] + _data[ 2u ] * other._data[ 2u ];
}

GXVoid GXVec3::CrossProduct ( const GXVec3 &a, const GXVec3 &b )
{
    _data[ 0u ] = a._data[ 1u ] * b._data[ 2u ] - a._data[ 2u ] * b._data[ 1u ];
    _data[ 1u ] = a._data[ 2u ] * b._data[ 0u ] - a._data[ 0u ] * b._data[ 2u ];
    _data[ 2u ] = a._data[ 0u ] * b._data[ 1u ] - a._data[ 1u ] * b._data[ 0u ];
}

GXFloat GXVec3::Length () const
{
    return sqrtf ( DotProduct ( *this ) );
}

GXFloat GXVec3::SquaredLength () const
{
    return DotProduct ( *this );
}

GXFloat GXVec3::Distance ( const GXVec3 &other ) const
{
    GXVec3 difference;
    difference.Substract ( *this, other );
    return difference.Length ();
}

GXFloat GXVec3::SquaredDistance ( const GXVec3 &other ) const
{
    GXVec3 difference;
    difference.Substract ( *this, other );
    return difference.SquaredLength ();
}

GXVoid GXVec3::LinearInterpolation ( const GXVec3 &start, const GXVec3 &finish, GXFloat interpolationFactor )
{
    GXVec3 difference;
    difference.Substract ( finish, start );
    Sum ( start, interpolationFactor, difference );
}

GXVoid GXVec3::Project ( const GXVec3 &vector, const GXVec3 &axis )
{
    GXVec3 normalVector ( vector );
    normalVector.Normalize ();
    GXFloat factor = vector.Length () * axis.DotProduct ( normalVector );
    Multiply ( axis, factor );
}

GXBool GXVec3::IsEqual ( const GXVec3 &other )
{
    if ( _data[ 0u ] != other._data[ 0u ] )
        return GX_FALSE;

    if ( _data[ 1u ] != other._data[ 1u ] )
        return GX_FALSE;

    return _data[ 2u ] == other._data[ 2u ];
}

const GXVec3& GXVec3::GetAbsoluteX ()
{
    static constexpr GXVec3 absoluteX ( 1.0f, 0.0f, 0.0 );
    return absoluteX;
}

const GXVec3& GXVec3::GetAbsoluteY ()
{
    static constexpr GXVec3 absoluteY ( 0.0f, 1.0f, 0.0 );
    return absoluteY;
}

const GXVec3& GXVec3::GetAbsoluteZ ()
{
    static constexpr GXVec3 absoluteZ ( 0.0f, 0.0f, 1.0 );
    return absoluteZ;
}

GXVoid GXCALL GXVec3::MakeOrthonormalBasis ( GXVec3 &baseX, GXVec3 &adjustedY, GXVec3 &adjustedZ )
{
    adjustedZ.CrossProduct ( baseX, adjustedY );

    if ( adjustedZ.SquaredLength () == 0.0f )
    {
        assert ( !"GXVec3::MakeOrthonormalBasis - Unexpected value." );
        return;
    }

    adjustedY.CrossProduct ( adjustedZ, baseX );

    baseX.Normalize ();
    adjustedY.Normalize ();
    adjustedZ.Normalize ();
}

GXVec3& GXVec3::operator = ( const GXVec3 &vector )
{
    memcpy ( this, &vector, sizeof ( GXVec3 ) );
    return *this;
}

//---------------------------------------------------------------------------------------------------------------------

GXBool GXCALL GXRayTriangleIntersection3D ( GXFloat &outT, const GXVec3 &origin, const GXVec3 &direction, GXFloat length, const GXVec3 &a, const GXVec3 &b, const GXVec3 &c )
{
    // Implementation https://graphics.stanford.edu/courses/cs348b-98/gg/intersect.html

    GXPlane plane;
    plane.From ( a, b, c );
    GXVec3 normal ( plane._a, plane._b, plane._c );

    GXFloat t = ( plane._d + normal.DotProduct ( origin ) ) / ( -normal.DotProduct ( direction ) );

    if ( t < 0.0f )
        return GX_FALSE;

    if ( t > length )
        return GX_FALSE;

    GXVec3 triangle[ 3u ];
    triangle[ 0u ] = a;
    triangle[ 1u ] = b;
    triangle[ 2u ] = c;

    GXVec3 point;
    point.Sum ( origin, t, direction );

    GXUByte selector;

    GXFloat gamma = fabsf ( plane._a );
    GXFloat omega = fabsf ( plane._b );
    GXFloat yotta = fabsf ( plane._c );

    if ( gamma > omega )
        selector = gamma > yotta ? 0u : 2u;
    else
        selector = omega > yotta ? 1u : 2u;

    GXUByte i1 = 0xFFu;
    GXUByte i2 = 0xFFu;

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

    GXFloat u0 = point._data[ i1 ] - triangle[ 0u ]._data[ i1 ];
    GXFloat v0 = point._data[ i2 ] - triangle[ 0u ]._data[ i2 ];

    GXFloat u1 = triangle[ 1u ]._data[ i1 ] - triangle[ 0u ]._data[ i1 ];
    GXFloat v1 = triangle[ 1u ]._data[ i2 ] - triangle[ 0u ]._data[ i2 ];

    GXFloat u2 = triangle[ 2u ]._data[ i1 ] - triangle[ 0u ]._data[ i1 ];
    GXFloat v2 = triangle[ 2u ]._data[ i2 ] - triangle[ 0u ]._data[ i2 ];

    gamma = 1.0f / ( u1 * v2 - v1 * u2 );
    GXFloat alpha = ( u0 * v2 - v0 * u2 ) * gamma;

    if ( alpha < 0.0f || alpha > 1.0f )
        return GX_FALSE;

    GXFloat betta = ( u1 * v0 - v1 * u0 ) * gamma;

    if ( betta < 0.0f || betta > 1.0f || ( alpha + betta ) > 1.0f )
        return GX_FALSE;

    outT = t;
    return GX_TRUE;
}

//----------------------------------------------------------------------------

GXEuler::GXEuler ():
    _pitchRadians ( 0.0f ),
    _yawRadians ( 0.0f ),
    _rollRadians ( 0.0f )
{
    // NOTHING
}

GXEuler::GXEuler ( const GXEuler &other )
{
    memcpy ( this, &other, sizeof ( GXEuler ) );
}

GXEuler& GXEuler::operator = ( const GXEuler &other )
{
    memcpy ( this, &other, sizeof ( GXEuler ) );
    return *this;
}

//---------------------------------------------------------------------------------------------------------------------

GXVec4::GXVec4 ()
{
    // NOTHING
}

GXVec4::GXVec4 ( const GXVec4 &other )
{
    memcpy ( this, &other, sizeof ( GXVec4 ) );
}

GXVec4::GXVec4 ( const GXVec3 &vector, GXFloat w )
{
    memcpy ( _data, vector._data, sizeof ( GXVec3 ) );
    _data[ 3u ] = w;
}

GXVoid GXVec4::Init ( GXFloat x, GXFloat y, GXFloat z, GXFloat w )
{
    _data[ 0u ] = x;
    _data[ 1u ] = y;
    _data[ 2u ] = z;
    _data[ 3u ] = w;
}

GXVoid GXVec4::SetX ( GXFloat x )
{
    _data[ 0u ] = x;
}

GXFloat GXVec4::GetX () const
{
    return _data[ 0u ];
}

GXVoid GXVec4::SetY ( GXFloat y )
{
    _data[ 1u ] = y;
}

GXFloat GXVec4::GetY () const
{
    return _data[ 1u ];
}

GXVoid GXVec4::SetZ ( GXFloat z )
{
    _data[ 2u ] = z;
}

GXFloat GXVec4::GetZ () const
{
    return _data[ 2u ];
}

GXVoid GXVec4::SetW ( GXFloat w )
{
    _data[ 3u ] = w;
}

GXFloat GXVec4::GetW () const
{
    return _data[ 3u ];
}

GXVoid GXVec4::Sum ( const GXVec4 &a, const GXVec4 &b )
{
    _data[ 0u ] = a._data[ 0u ] + b._data[ 0u ];
    _data[ 1u ] = a._data[ 1u ] + b._data[ 1u ];
    _data[ 2u ] = a._data[ 2u ] + b._data[ 2u ];
    _data[ 3u ] = a._data[ 3u ] + b._data[ 3u ];
}

GXVoid GXVec4::Sum ( const GXVec4 &a, GXFloat bScale, const GXVec4 &b )
{
    _data[ 0u ] = a._data[ 0u ] + bScale * b._data[ 0u ];
    _data[ 1u ] = a._data[ 1u ] + bScale * b._data[ 1u ];
    _data[ 2u ] = a._data[ 2u ] + bScale * b._data[ 2u ];
    _data[ 3u ] = a._data[ 3u ] + bScale * b._data[ 3u ];
}

GXVoid GXVec4::Substract ( const GXVec4 &a, const GXVec4 &b )
{
    _data[ 0u ] = a._data[ 0u ] - b._data[ 0u ];
    _data[ 1u ] = a._data[ 1u ] - b._data[ 1u ];
    _data[ 2u ] = a._data[ 2u ] - b._data[ 2u ];
    _data[ 3u ] = a._data[ 3u ] - b._data[ 3u ];
}

GXFloat GXVec4::DotProduct ( const GXVec4 &other ) const
{
    return _data[ 0u ] * other._data[ 0u ] + _data[ 1u ] * other._data[ 1u ] + _data[ 2u ] * other._data[ 2u ] + _data[ 3u ] * other._data[ 3u ];
}

GXFloat GXVec4::Length () const
{
    return sqrtf ( DotProduct ( *this ) );
}

GXFloat GXVec4::SquaredLength () const
{
    return DotProduct ( *this );
}

GXVec4& GXVec4::operator = ( const GXVec4 &vector )
{
    memcpy ( this, &vector, sizeof ( GXVec4 ) );
    return *this;
}

//---------------------------------------------------------------------------------------------------------------------

GXVec6::GXVec6 ()
{
    // NOTHING
}

GXVec6::GXVec6 ( const GXVec6 &other )
{
    memcpy ( this, &other, sizeof ( GXVec6 ) );
}

GXVoid GXVec6::Init ( GXFloat a1, GXFloat a2, GXFloat a3, GXFloat a4, GXFloat a5, GXFloat a6 )
{
    _data[ 0u ] = a1;
    _data[ 1u ] = a2;
    _data[ 2u ] = a3;
    _data[ 3u ] = a4;
    _data[ 4u ] = a5;
    _data[ 5u ] = a6;
}

GXVoid GXVec6::From ( const GXVec3 &v1, const GXVec3 &v2 )
{
    memcpy ( _data, &v1, sizeof ( GXVec3 ) );
    memcpy ( _data + 3u, &v2, sizeof ( GXVec3 ) );
}

GXFloat GXVec6::DotProduct ( const GXVec6 &other ) const
{
    return _data[ 0u ] * other._data[ 0u ] + _data[ 1u ] * other._data[ 1u ] + _data[ 2u ] * other._data[ 2u ] + _data[ 3u ] * other._data[ 3u ] + _data[ 4u ] * other._data[ 4u ] + _data[ 5u ] * other._data[ 5u ];
}

GXVoid GXVec6::Sum ( const GXVec6 &a, const GXVec6 &b )
{
    _data[ 0u ] = a._data[ 0u ] + b._data[ 0u ];
    _data[ 1u ] = a._data[ 1u ] + b._data[ 1u ];
    _data[ 2u ] = a._data[ 2u ] + b._data[ 2u ];
    _data[ 3u ] = a._data[ 3u ] + b._data[ 3u ];
    _data[ 4u ] = a._data[ 4u ] + b._data[ 4u ];
    _data[ 5u ] = a._data[ 5u ] + b._data[ 5u ];
}

GXVoid GXVec6::Sum ( const GXVec6 &a, GXFloat bScale, const GXVec6 &b )
{
    _data[ 0u ] = a._data[ 0u ] + bScale * b._data[ 0u ];
    _data[ 1u ] = a._data[ 1u ] + bScale * b._data[ 1u ];
    _data[ 2u ] = a._data[ 2u ] + bScale * b._data[ 2u ];
    _data[ 3u ] = a._data[ 3u ] + bScale * b._data[ 3u ];
    _data[ 4u ] = a._data[ 4u ] + bScale * b._data[ 4u ];
    _data[ 5u ] = a._data[ 5u ] + bScale * b._data[ 5u ];
}

GXVoid GXVec6::Multiply ( const GXVec6 &a, GXFloat factor )
{
    _data[ 0u ] = a._data[ 0u ] * factor;
    _data[ 1u ] = a._data[ 1u ] * factor;
    _data[ 2u ] = a._data[ 2u ] * factor;
    _data[ 3u ] = a._data[ 3u ] * factor;
    _data[ 4u ] = a._data[ 4u ] * factor;
    _data[ 5u ] = a._data[ 5u ] * factor;
}

GXVec6& GXVec6::operator = ( const GXVec6 &other )
{
    memcpy ( this, &other, sizeof ( GXVec6 ) );
    return *this;
}

//---------------------------------------------------------------------------------------------------------------------

GXColorRGB::GXColorRGB ()
{
    memset ( _data, 0, 4u * sizeof ( GXFloat ) );
}

GXColorRGB::GXColorRGB ( const GXColorRGB &other )
{
    memcpy ( this, &other, sizeof ( GXColorRGB ) );
}

GXColorRGB::GXColorRGB ( GXUByte red, GXUByte green, GXUByte blue, GXFloat alpha )
{
    From ( red, green, blue, alpha );
}

GXColorRGB::GXColorRGB ( const GXColorHSV &color )
{
    From ( color );
}

GXVoid GXColorRGB::Init ( GXFloat red, GXFloat green, GXFloat blue, GXFloat alpha )
{
    _data[ 0u ] = red;
    _data[ 1u ] = green;
    _data[ 2u ] = blue;
    _data[ 3u ] = alpha;
}

GXVoid GXColorRGB::SetRed ( GXFloat red )
{
    _data[ 0u ] = red;
}

GXFloat GXColorRGB::GetRed () const
{
    return _data[ 0u ];
}

GXVoid GXColorRGB::SetGreen ( GXFloat green )
{
    _data[ 1u ] = green;
}

GXFloat GXColorRGB::GetGreen () const
{
    return _data[ 1u ];
}

GXVoid GXColorRGB::SetBlue ( GXFloat blue )
{
    _data[ 2u ] = blue;
}

GXFloat GXColorRGB::GetBlue () const
{
    return _data[ 2u ];
}

GXVoid GXColorRGB::SetAlpha ( GXFloat alpha )
{
    _data[ 3u ] = alpha;
}

GXFloat GXColorRGB::GetAlpha () const
{
    return _data[ 3u ];
}

GXVoid GXColorRGB::From ( GXUByte red, GXUByte green, GXUByte blue, GXFloat alpha )
{
    _data[ 0u ] = red * COLOR_TO_FLOAT_FACTOR;
    _data[ 1u ] = green * COLOR_TO_FLOAT_FACTOR;
    _data[ 2u ] = blue * COLOR_TO_FLOAT_FACTOR;
    _data[ 3u ] = alpha * COLOR_TO_FLOAT_FACTOR;
}

GXVoid GXColorRGB::From ( const GXColorHSV &color )
{
    GXFloat correctedHue = color.GetHue ();

    while ( correctedHue >= 360.0f )
        correctedHue -= 360.0f;

    while ( correctedHue < 0.0f )
        correctedHue += 360.0f;

    GXFloat value = color.GetValue ();

    GXUByte selector = static_cast<GXUByte> ( ( static_cast<GXInt> ( correctedHue ) / 60 ) % 6 );
    GXFloat minValue = ( ( 100.0f - color.GetSaturation () ) * value ) * 0.01f;
    GXFloat delta = ( value - minValue ) * ( ( static_cast<GXInt> ( correctedHue ) % 60 ) * HSVA_FACTOR );
    GXFloat increment = minValue + delta;
    GXFloat decrement = value - delta;

    _data[ 3 ] = color.GetAlpha () * HSVA_TO_RGBA_FLOAT;

    switch ( selector )
    {
        case 0u:
            _data[ 0u ] = value * HSVA_TO_RGBA_FLOAT;
            _data[ 1u ] = increment * HSVA_TO_RGBA_FLOAT;
            _data[ 2u ] = minValue * HSVA_TO_RGBA_FLOAT;
        break;

        case 1u:
            _data[ 0u ] = decrement * HSVA_TO_RGBA_FLOAT;
            _data[ 1u ] = value * HSVA_TO_RGBA_FLOAT;
            _data[ 2u ] = minValue * HSVA_TO_RGBA_FLOAT;
        break;

        case 2u:
            _data[ 0u ] = minValue * HSVA_TO_RGBA_FLOAT;
            _data[ 1u ] = value * HSVA_TO_RGBA_FLOAT;
            _data[ 2u ] = increment * HSVA_TO_RGBA_FLOAT;
        break;

        case 3u:
            _data[ 0u ] = minValue * HSVA_TO_RGBA_FLOAT;
            _data[ 1u ] = decrement * HSVA_TO_RGBA_FLOAT;
            _data[ 2u ] = value * HSVA_TO_RGBA_FLOAT;
        break;

        case 4u:
            _data[ 0u ] = increment * HSVA_TO_RGBA_FLOAT;
            _data[ 1u ] = minValue * HSVA_TO_RGBA_FLOAT;
            _data[ 2u ] = value * HSVA_TO_RGBA_FLOAT;
        break;

        case 5u:
            _data[ 0u ] = value * HSVA_TO_RGBA_FLOAT;
            _data[ 1u ] = minValue * HSVA_TO_RGBA_FLOAT;
            _data[ 2u ] = decrement * HSVA_TO_RGBA_FLOAT;
        break;

        default:
            assert ( !"GXColorRGB::From - Something wrong!" );
        break;
    }
}

GXVoid GXColorRGB::ConvertToUByte ( GXUByte &red, GXUByte &green, GXUByte &blue, GXUByte &alpha ) const
{
    red = static_cast<GXUByte> ( _data[ 0u ] * RGBA_TO_UBYTE_FACTOR + 0.5f );
    green = static_cast<GXUByte> ( _data[ 1u ] * RGBA_TO_UBYTE_FACTOR + 0.5f );
    blue = static_cast<GXUByte> ( _data[ 2u ] * RGBA_TO_UBYTE_FACTOR + 0.5f );
    alpha = static_cast<GXUByte> ( _data[ 3u ] * RGBA_TO_UBYTE_FACTOR + 0.5f );
}

GXColorRGB& GXColorRGB::operator = ( const GXColorRGB &other )
{
    memcpy ( this, &other, sizeof ( GXColorRGB ) );
    return *this;
}

//---------------------------------------------------------------------------------------------------------------------

GXColorHSV::GXColorHSV ()
{
    memset ( _data, 0, 4u * sizeof ( GXFloat ) );
}

GXColorHSV::GXColorHSV ( const GXColorHSV &other )
{
    memcpy ( this, &other, sizeof ( GXColorHSV ) );
}

GXColorHSV::GXColorHSV ( const GXColorRGB &color )
{
    From ( color );
}

GXVoid GXColorHSV::SetHue ( GXFloat hue )
{
    _data[ 0u ] = hue;
}

GXFloat GXColorHSV::GetHue () const
{
    return _data[ 0u ];
}

GXVoid GXColorHSV::SetSaturation ( GXFloat saturation )
{
    _data[ 1u ] = saturation;
}

GXFloat GXColorHSV::GetSaturation () const
{
    return _data[ 1u ];
}

GXVoid GXColorHSV::SetValue ( GXFloat value )
{
    _data[ 2u ] = value;
}

GXFloat GXColorHSV::GetValue () const
{
    return _data[ 2u ];
}

GXVoid GXColorHSV::SetAlpha ( GXFloat alpha )
{
    _data[ 3u ] = alpha;
}

GXFloat GXColorHSV::GetAlpha () const
{
    return _data[ 3u ];
}

GXVoid GXColorHSV::From ( const GXColorRGB &color )
{
    GXFloat maxValue = GXMaxf ( GXMaxf ( color.GetRed (), color.GetGreen () ), color.GetBlue () );
    GXFloat minValue = GXMinf ( GXMinf ( color.GetRed (), color.GetGreen () ), color.GetBlue () );

    if ( maxValue == minValue )
    {
        _data[ 0u ] = 0.0f;
    }
    else if ( maxValue == color.GetRed () && color.GetGreen () >= color.GetBlue () )
    {
        _data[ 0u ] = 60.0f * ( ( color.GetGreen () - color.GetBlue () ) / ( maxValue - minValue ) );
    }
    else if ( maxValue == color.GetRed () && color.GetGreen () < color.GetBlue () )
    {
        _data[ 0u ] = 60.0f * ( ( color.GetGreen () - color.GetBlue () ) / ( maxValue - minValue ) ) + 360.0f;
    }
    else if ( maxValue == color.GetGreen () )
    {
        _data[ 0u ] = 60.0f * ( ( color.GetBlue () - color.GetRed () ) / ( maxValue - minValue ) ) + 120.0f;
    }
    else if ( maxValue == color.GetBlue () )
    {
        _data[ 0u ] = 60.0f * ( ( color.GetRed () - color.GetGreen () ) / ( maxValue - minValue ) ) + 240.0f;
    }
    else
    {
        assert ( !"GXColorHSV::From - Something wrong!" );
    }

    if ( maxValue == 0.0f )
        _data[ 1u ] = 0.0f;
    else
        _data[ 1u ] = 100.0f * ( 1.0f - minValue / maxValue );

    _data[ 2u ] = 100.0f * maxValue;
    _data[ 3u ] = 100.0f * color.GetAlpha ();
}

GXVoid GXColorHSV::operator = ( const GXColorHSV &other )
{
    memcpy ( this, &other, sizeof ( GXColorHSV ) );
}

//---------------------------------------------------------------------------------------------------------------------

GXPreciseComplex::GXPreciseComplex ()
{
    // NOTHING
}

GXPreciseComplex::GXPreciseComplex ( const GXPreciseComplex &other ):
    _r ( other._r ),
    _i ( other._i )
{
    // NOTHING
}

GXPreciseComplex::~GXPreciseComplex ()
{
    // NOTHING
}

GXVoid GXPreciseComplex::Init ( GXDouble real, GXDouble imaginary )
{
    _r = real;
    _i = imaginary;
}

GXDouble GXPreciseComplex::Length ()
{
    return sqrt ( SquaredLength () );
}

GXDouble GXPreciseComplex::SquaredLength ()
{
    return _r * _r + _i * _i;
}

GXBool GXPreciseComplex::Power ( GXUInt power )
{
    if ( power == 1u )
        return GX_TRUE;

    if ( power > 1u )
    {
        GXPreciseComplex alpha ( *this );

        while ( power > 0u )
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

GXPreciseComplex& GXPreciseComplex::operator = ( const GXPreciseComplex &other )
{
    memcpy ( this, &other, sizeof ( GXPreciseComplex ) );
    return *this;
}

GXPreciseComplex GXPreciseComplex::operator + ( const GXPreciseComplex &other )
{
    return GXPreciseComplex ( _r + other._r, _i + other._i );
}

GXPreciseComplex GXPreciseComplex::operator - ( const GXPreciseComplex &other )
{
    return GXPreciseComplex ( _r - other._r, _i - other._i );
}

GXPreciseComplex GXPreciseComplex::operator * ( const GXPreciseComplex &other )
{
    return GXPreciseComplex ( _r * other._r - _i * other._i, _r * other._i + _i * other._r );
}

GXPreciseComplex GXPreciseComplex::operator * ( GXDouble a )
{
    return GXPreciseComplex ( _r * a, _i * a );
}

GXPreciseComplex GXPreciseComplex::operator / ( GXDouble a )
{
    const GXDouble invA = 1.0 / a;
    return GXPreciseComplex ( _r * invA, _i * invA );
}

//---------------------------------------------------------------------------------------------------------------------

GXQuat::GXQuat ()
{
    memset ( _data, 0, 4u * sizeof ( GXFloat ) );
}

GXQuat::GXQuat ( const GXQuat &other )
{
    memcpy ( this, &other, sizeof ( GXQuat ) );
}

GXQuat::GXQuat ( const GXMat3 &rotationMatrix )
{
    From ( rotationMatrix );
}

GXQuat::GXQuat ( const GXMat4 &rotationMatrix )
{
    From ( rotationMatrix );
}

GXVoid GXQuat::Init ( GXFloat r, GXFloat a, GXFloat b, GXFloat c )
{
    _data[ 0u ] = r;
    _data[ 1u ] = a;
    _data[ 2u ] = b;
    _data[ 3u ] = c;
}

GXVoid GXQuat::SetR ( GXFloat r )
{
    _data[ 0u ] = r;
}

GXFloat GXQuat::GetR () const
{
    return _data[ 0u ];
}

GXVoid GXQuat::SetA ( GXFloat a )
{
    _data[ 1u ] = a;
}

GXFloat GXQuat::GetA () const
{
    return _data[ 1u ];
}

GXVoid GXQuat::SetB ( GXFloat b )
{
    _data[ 2u ] = b;
}

GXFloat GXQuat::GetB () const
{
    return _data[ 2u ];
}

GXVoid GXQuat::SetC ( GXFloat c )
{
    _data[ 3u ] = c;
}

GXFloat GXQuat::GetC () const
{
    return _data[ 3u ];
}

GXVoid GXQuat::Identity ()
{
    _data[ 0u ] = 1.0f;
    _data[ 1u ] = _data[ 2u ] = _data[ 3u ] = 0.0f;
}

GXVoid GXQuat::Normalize ()
{
    GXFloat squaredLength = _data[ 0u ] * _data[ 0u ] + _data[ 1u ] * _data[ 1u ] + _data[ 2u ] * _data[ 2u ] + _data[ 3u ] * _data[ 3u ];

    if ( fabsf ( squaredLength ) < FLOAT_EPSILON )
    {
        assert ( !"GXQuat::Normalize - Error." );
        return;
    }

    Multiply ( *this, 1.0f / sqrtf ( squaredLength ) );
}

GXVoid GXQuat::Inverse ( const GXQuat &q )
{
    GXFloat squaredLength = q._data[ 0u ] * q._data[ 0u ] + q._data[ 1u ] * q._data[ 1u ] + q._data[ 2u ] * q._data[ 2u ] + q._data[ 3u ] * q._data[ 3u ];

    if ( fabsf ( squaredLength ) <= FLOAT_EPSILON )
    {
        assert ( !"GXQuat::Inverse - Error." );
        Identity ();
        return;
    }

    GXFloat inverseSquaredLength = 1.0f / squaredLength;

    _data[ 0u ] = q._data[ 0u ] * inverseSquaredLength;
    _data[ 1u ] = -q._data[ 1u ] * inverseSquaredLength;
    _data[ 2u ] = -q._data[ 2u ] * inverseSquaredLength;
    _data[ 3u ] = -q._data[ 3u ] * inverseSquaredLength;
}

GXVoid GXQuat::FromAxisAngle ( GXFloat x, GXFloat y, GXFloat z, GXFloat angle )
{
    GXFloat halfAngle = 0.5f * angle;
    GXFloat sinom = sinf ( halfAngle );

    _data[ 0u ] = cosf ( halfAngle );
    _data[ 1u ] = x * sinom;
    _data[ 2u ] = y * sinom;
    _data[ 3u ] = z * sinom;
}

GXVoid GXQuat::FromAxisAngle ( const GXVec3 &axis, GXFloat angle )
{
    FromAxisAngle ( axis._data[ 0u ], axis._data[ 1u ], axis._data[ 2u ], angle );
}

GXVoid GXQuat::From ( const GXMat3 &rotationMatrix )
{
    GXMat3 pureRotationMatrix;
    pureRotationMatrix.ClearRotation ( rotationMatrix );
    FromFast ( pureRotationMatrix );
}

GXVoid GXQuat::From ( const GXMat4 &rotationMatrix )
{
    GXMat4 pureRotationMatrix;
    pureRotationMatrix.ClearRotation ( rotationMatrix );
    FromFast ( pureRotationMatrix );
}

GXVoid GXQuat::FromFast ( const GXMat3 &pureRotationMatrix )
{
    // In ideal mathematics world all solutions are right.
    // But in practice more precise solution is the biggest "solutionFactorXXX" because of square root operation.

    GXFloat solutionFactorAlpha = pureRotationMatrix._m[ 0u ][ 0u ] + pureRotationMatrix._m[ 1u ][ 1u ] + pureRotationMatrix._m[ 2u ][ 2u ] + 1.0f;
    GXFloat solutionFactorBetta = pureRotationMatrix._m[ 0u ][ 0u ] - pureRotationMatrix._m[ 1u ][ 1u ] - pureRotationMatrix._m[ 2u ][ 2u ] + 1.0f;
    GXFloat solutionFactorGamma = -pureRotationMatrix._m[ 0u ][ 0u ] + pureRotationMatrix._m[ 1u ][ 1u ] - pureRotationMatrix._m[ 2u ][ 2u ] + 1.0f;
    GXFloat solutionFactorYotta = -pureRotationMatrix._m[ 0u ][ 0u ] - pureRotationMatrix._m[ 1u ][ 1u ] + pureRotationMatrix._m[ 2u ][ 2u ] + 1.0f;

    GXUByte solution = static_cast<GXUByte> ( UNKNOWN_SOLUTION );

    if ( solutionFactorAlpha > solutionFactorBetta )
    {
        if ( solutionFactorAlpha > solutionFactorGamma )
        {
            solution = solutionFactorAlpha > solutionFactorYotta ? static_cast<GXUByte> ( SOLUTION_ALPHA ) : solution = static_cast<GXUByte> ( SOLUTION_YOTTA );
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
        solution = solutionFactorBetta > solutionFactorYotta ? static_cast<GXUByte> ( SOLUTION_BETTA ) : static_cast<GXUByte> ( SOLUTION_YOTTA );
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
            GXFloat phi = 0.5f * sqrtf ( solutionFactorAlpha );
            GXFloat omega = 1.0f / ( 4.0f * phi );

            _data[ 0u ] = phi;
            _data[ 1u ] = omega * ( pureRotationMatrix._m[ 1u ][ 2u ] - pureRotationMatrix._m[ 2u ][ 1u ] );
            _data[ 2u ] = omega * ( pureRotationMatrix._m[ 2u ][ 0u ] - pureRotationMatrix._m[ 0u ][ 2u ] );
            _data[ 3u ] = omega * ( pureRotationMatrix._m[ 0u ][ 1u ] - pureRotationMatrix._m[ 1u ][ 0u ] );
        }
        break;

        case SOLUTION_BETTA:
        {
            GXFloat phi = 0.5f * sqrtf ( solutionFactorBetta );
            GXFloat omega = 1.0f / ( 4.0f * phi );

            _data[ 0u ] = omega * ( pureRotationMatrix._m[ 1u ][ 2u ] - pureRotationMatrix._m[ 2u ][ 1u ] );
            _data[ 1u ] = phi;
            _data[ 2u ] = omega * ( pureRotationMatrix._m[ 0u ][ 1u ] + pureRotationMatrix._m[ 1u ][ 0u ] );
            _data[ 3u ] = omega * ( pureRotationMatrix._m[ 0u ][ 2u ] + pureRotationMatrix._m[ 2u ][ 0u ] );
        }
        break;

        case SOLUTION_GAMMA:
        {
            GXFloat phi = 0.5f * sqrtf ( solutionFactorGamma );
            GXFloat omega = 1.0f / ( 4.0f * phi );

            _data[ 0u ] = omega * ( pureRotationMatrix._m[ 2u ][ 0u ] - pureRotationMatrix._m[ 0u ][ 2u ] );
            _data[ 1u ] = omega * ( pureRotationMatrix._m[ 0u ][ 1u ] + pureRotationMatrix._m[ 1u ][ 0u ] );
            _data[ 2u ] = phi;
            _data[ 3u ] = omega * ( pureRotationMatrix._m[ 1u ][ 2u ] + pureRotationMatrix._m[ 2u ][ 1u ] );
        }
        break;

        case SOLUTION_YOTTA:
        {
            GXFloat phi = 0.5f * sqrtf ( solutionFactorYotta );
            GXFloat omega = 1.0f / ( 4.0f * phi );

            _data[ 0u ] = omega * ( pureRotationMatrix._m[ 0u ][ 1u ] - pureRotationMatrix._m[ 1u ][ 0u ] );
            _data[ 1u ] = omega * ( pureRotationMatrix._m[ 0u ][ 2u ] + pureRotationMatrix._m[ 2u ][ 0u ] );
            _data[ 2u ] = omega * ( pureRotationMatrix._m[ 1u ][ 2u ] + pureRotationMatrix._m[ 2u ][ 1u ] );
            _data[ 3u ] = phi;
        }
        break;

        default:
            // NOTHING
        break;
    }
}

GXVoid GXQuat::FromFast ( const GXMat4 &pureRotationMatrix )
{
    // In ideal mathematics world all solutions are right.
    // But in practice more precise solution is the biggest "solutionFactorXXX" because of square root operation.

    GXFloat solutionFactorAlpha = pureRotationMatrix._m[ 0u ][ 0u ] + pureRotationMatrix._m[ 1u ][ 1u ] + pureRotationMatrix._m[ 2u ][ 2u ] + 1.0f;
    GXFloat solutionFactorBetta = pureRotationMatrix._m[ 0u ][ 0u ] - pureRotationMatrix._m[ 1u ][ 1u ] - pureRotationMatrix._m[ 2u ][ 2u ] + 1.0f;
    GXFloat solutionFactorGamma = -pureRotationMatrix._m[ 0u ][ 0u ] + pureRotationMatrix._m[ 1u ][ 1u ] - pureRotationMatrix._m[ 2u ][ 2u ] + 1.0f;
    GXFloat solutionFactorYotta = -pureRotationMatrix._m[ 0u ][ 0u ] - pureRotationMatrix._m[ 1u ][ 1u ] + pureRotationMatrix._m[ 2u ][ 2u ] + 1.0f;

    GXUByte solution = static_cast<GXUByte> ( UNKNOWN_SOLUTION );

    if ( solutionFactorAlpha > solutionFactorBetta )
    {
        if ( solutionFactorAlpha > solutionFactorGamma )
        {
            solution = solutionFactorAlpha > solutionFactorYotta ? static_cast<GXUByte> ( SOLUTION_ALPHA ) : solution = static_cast<GXUByte> ( SOLUTION_YOTTA );
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
        solution = solutionFactorBetta > solutionFactorYotta ? static_cast<GXUByte> ( SOLUTION_BETTA ) : static_cast<GXUByte> ( SOLUTION_YOTTA );
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
            GXFloat phi = 0.5f * sqrtf ( solutionFactorAlpha );
            GXFloat omega = 1.0f / ( 4.0f * phi );

            _data[ 0u ] = phi;
            _data[ 1u ] = omega * ( pureRotationMatrix._m[ 1u ][ 2u ] - pureRotationMatrix._m[ 2u ][ 1u ] );
            _data[ 2u ] = omega * ( pureRotationMatrix._m[ 2u ][ 0u ] - pureRotationMatrix._m[ 0u ][ 2u ] );
            _data[ 3u ] = omega * ( pureRotationMatrix._m[ 0u ][ 1u ] - pureRotationMatrix._m[ 1u ][ 0u ] );
        }
        break;

        case SOLUTION_BETTA:
        {
            GXFloat phi = 0.5f * sqrtf ( solutionFactorBetta );
            GXFloat omega = 1.0f / ( 4.0f * phi );

            _data[ 0u ] = omega * ( pureRotationMatrix._m[ 1u ][ 2u ] - pureRotationMatrix._m[ 2u ][ 1u ] );
            _data[ 1u ] = phi;
            _data[ 2u ] = omega * ( pureRotationMatrix._m[ 0u ][ 1u ] + pureRotationMatrix._m[ 1u ][ 0u ] );
            _data[ 3u ] = omega * ( pureRotationMatrix._m[ 0u ][ 2u ] + pureRotationMatrix._m[ 2u ][ 0u ] );
        }
        break;

        case SOLUTION_GAMMA:
        {
            GXFloat phi = 0.5f * sqrtf ( solutionFactorGamma );
            GXFloat omega = 1.0f / ( 4.0f * phi );

            _data[ 0u ] = omega * ( pureRotationMatrix._m[ 2u ][ 0u ] - pureRotationMatrix._m[ 0u ][ 2u ] );
            _data[ 1u ] = omega * ( pureRotationMatrix._m[ 0u ][ 1u ] + pureRotationMatrix._m[ 1u ][ 0u ] );
            _data[ 2u ] = phi;
            _data[ 3u ] = omega * ( pureRotationMatrix._m[ 1u ][ 2u ] + pureRotationMatrix._m[ 2u ][ 1u ] );
        }
        break;

        case SOLUTION_YOTTA:
        {
            GXFloat phi = 0.5f * sqrtf ( solutionFactorYotta );
            GXFloat omega = 1.0f / ( 4.0f * phi );

            _data[ 0u ] = omega * ( pureRotationMatrix._m[ 0u ][ 1u ] - pureRotationMatrix._m[ 1u ][ 0u ] );
            _data[ 1u ] = omega * ( pureRotationMatrix._m[ 0u ][ 2u ] + pureRotationMatrix._m[ 2u ][ 0u ] );
            _data[ 2u ] = omega * ( pureRotationMatrix._m[ 1u ][ 2u ] + pureRotationMatrix._m[ 2u ][ 1u ] );
            _data[ 3u ] = phi;
        }
        break;

        default:
            // NOTHING
        break;
    }
}

GXVoid GXQuat::Multiply ( const GXQuat &a, const GXQuat &b )
{
    _data[ 0u ] = a._data[ 0u ] * b._data[ 0u ] - a._data[ 1u ] * b._data[ 1u ] - a._data[ 2u ] * b._data[ 2u ] - a._data[ 3u ] * b._data[ 3u ];
    _data[ 1u ] = a._data[ 0u ] * b._data[ 1u ] + a._data[ 1u ] * b._data[ 0u ] + a._data[ 2u ] * b._data[ 3u ] - a._data[ 3u ] * b._data[ 2u ];
    _data[ 2u ] = a._data[ 0u ] * b._data[ 2u ] - a._data[ 1u ] * b._data[ 3u ] + a._data[ 2u ] * b._data[ 0u ] + a._data[ 3u ] * b._data[ 1u ];
    _data[ 3u ] = a._data[ 0u ] * b._data[ 3u ] + a._data[ 1u ] * b._data[ 2u ] - a._data[ 2u ] * b._data[ 1u ] + a._data[ 3u ] * b._data[ 0u ];
}

GXVoid GXQuat::Multiply ( const GXQuat &q, GXFloat scale )
{
    _data[ 0u ] = q._data[ 0u ] * scale;
    _data[ 1u ] = q._data[ 1u ] * scale;
    _data[ 2u ] = q._data[ 2u ] * scale;
    _data[ 3u ] = q._data[ 3u ] * scale;
}

GXVoid GXQuat::Sum ( const GXQuat &a, const GXQuat &b )
{
    _data[ 0u ] = a._data[ 0u ] + b._data[ 0u ];
    _data[ 1u ] = a._data[ 1u ] + b._data[ 1u ];
    _data[ 2u ] = a._data[ 2u ] + b._data[ 2u ];
    _data[ 3u ] = a._data[ 3u ] + b._data[ 3u ];
}

GXVoid GXQuat::Substract ( const GXQuat &a, const GXQuat &b )
{
    _data[ 0u ] = a._data[ 0u ] - b._data[ 0u ];
    _data[ 1u ] = a._data[ 1u ] - b._data[ 1u ];
    _data[ 2u ] = a._data[ 2u ] - b._data[ 2u ];
    _data[ 3u ] = a._data[ 3u ] - b._data[ 3u ];
}

GXVoid GXQuat::SphericalLinearInterpolation ( const GXQuat &start, const GXQuat &finish, GXFloat interpolationFactor )
{
    GXQuat temp;
    GXFloat omega;
    GXFloat cosom;
    GXFloat sinom;
    GXFloat scale0;
    GXFloat scale1;

    if ( interpolationFactor <= 0.0f )
    {
        *this = start;
        return;
    }

    if ( interpolationFactor >= 1.0f )
    {
        *this = finish;
        return;
    }

    cosom = start._data[ 0u ] * finish._data[ 0u ] + start._data[ 1u ] * finish._data[ 1u ] + start._data[ 2u ] * finish._data[ 2u ] + start._data[ 3u ] * finish._data[ 3u ];

    if ( cosom < 0.0f )
    {
        temp._data[ 0u ] = -finish._data[ 0u ];
        temp._data[ 1u ] = -finish._data[ 1u ];
        temp._data[ 2u ] = -finish._data[ 2u ];
        temp._data[ 3u ] = -finish._data[ 3u ];
        cosom = -cosom;
    }
    else
    {
        temp = finish;
    }

    if ( ( 1.0f - cosom ) > FLOAT_EPSILON )
    {
        omega = acosf ( cosom );
        sinom = 1.0f / sinf ( omega );
        scale0 = sinf ( ( 1.0f - interpolationFactor ) * omega ) * sinom;
        scale1 = sinf ( interpolationFactor * omega ) * sinom;
    }
    else
    {
        scale0 = 1.0f - interpolationFactor;
        scale1 = interpolationFactor;
    }

    _data[ 0u ] = start._data[ 0u ] * scale0 + temp._data[ 0u ] * scale1;
    _data[ 1u ] = start._data[ 1u ] * scale0 + temp._data[ 1u ] * scale1;
    _data[ 2u ] = start._data[ 2u ] * scale0 + temp._data[ 2u ] * scale1;
    _data[ 3u ] = start._data[ 3u ] * scale0 + temp._data[ 3u ] * scale1;
}

GXVoid GXQuat::GetAxisAngle ( GXVec3 &axis, GXFloat &angle ) const
{
    GXQuat q ( *this );

    if ( fabsf ( q._data[ 0u ] ) > 1.0f )
        q.Normalize ();

    angle = 2.0f * acosf ( q._data[ 0u ] );

    axis._data[ 0u ] = q._data[ 1u ];
    axis._data[ 1u ] = q._data[ 2u ];
    axis._data[ 2u ] = q._data[ 3u ];

    GXFloat s = sqrtf ( 1.0f - q._data[ 0u ] * q._data[ 0u ] );

    if ( s < FLOAT_EPSILON ) return;

    axis.Multiply ( axis, 1.0f / s );
}

GXVoid GXQuat::Transform ( GXVec3 &out, const GXVec3 &v ) const
{
    GXFloat rr = _data[ 0u ] * _data[ 0u ];
    GXFloat ra2 = _data[ 0u ] * _data[ 1u ] * 2.0f;
    GXFloat rb2 = _data[ 0u ] * _data[ 2u ] * 2.0f;
    GXFloat rc2 = _data[ 0u ] * _data[ 3u ] * 2.0f;

    GXFloat aa = _data[ 1u ] * _data[ 1u ];
    GXFloat ab2 = _data[ 1u ] * _data[ 2u ] * 2.0f;
    GXFloat ac2 = _data[ 1u ] * _data[ 3u ] * 2.0f;

    GXFloat bb = _data[ 2u ] * _data[ 2u ];
    GXFloat bc2 = _data[ 2u ] * _data[ 3u ] * 2.0f;

    GXFloat cc = _data[ 3u ] * _data[ 3u ];

    GXFloat inverseSquaredLength = 1.0f / ( rr + aa + bb + cc );

    out._data[ 0u ] = inverseSquaredLength * ( v._data[ 0u ] * ( rr + aa - bb - cc ) + v._data[ 1u ] * ( ab2 - rc2 ) + v._data[ 2u ] * ( rb2 + ac2 ) );
    out._data[ 1u ] = inverseSquaredLength * ( v._data[ 0u ] * ( rc2 + ab2 ) + v._data[ 1u ] * ( rr - aa + bb - cc ) + v._data[ 2u ] * ( bc2 - ra2 ) );
    out._data[ 2u ] = inverseSquaredLength * ( v._data[ 0u ] * ( ac2 - rb2 ) + v._data[ 1u ] * ( ra2 + bc2 ) + v._data[ 2u ] * ( rr - aa - bb + cc ) );
}

GXVoid GXQuat::TransformFast ( GXVec3 &out, const GXVec3 &v ) const
{
    GXFloat rr = _data[ 0u ] * _data[ 0u ];
    GXFloat ra2 = _data[ 0u ] * _data[ 1u ] * 2.0f;
    GXFloat rb2 = _data[ 0u ] * _data[ 2u ] * 2.0f;
    GXFloat rc2 = _data[ 0u ] * _data[ 3u ] * 2.0f;

    GXFloat aa = _data[ 1u ] * _data[ 1u ];
    GXFloat ab2 = _data[ 1u ] * _data[ 2u ] * 2.0f;
    GXFloat ac2 = _data[ 1u ] * _data[ 3u ] * 2.0f;

    GXFloat bb = _data[ 2u ] * _data[ 2u ];
    GXFloat bc2 = _data[ 2u ] * _data[ 3u ] * 2.0f;

    GXFloat cc = _data[ 3u ] * _data[ 3u ];

    out._data[ 0u ] = v._data[ 0u ] * ( rr + aa - bb - cc ) + v._data[ 1u ] * ( ab2 - rc2 ) + v._data[ 2u ] * ( rb2 + ac2 );
    out._data[ 1u ] = v._data[ 0u ] * ( rc2 + ab2 ) + v._data[ 1u ] * ( rr - aa + bb - cc ) + v._data[ 2u ] * ( bc2 - ra2 );
    out._data[ 2u ] = v._data[ 0u ] * ( ac2 - rb2 ) + v._data[ 1u ] * ( ra2 + bc2 ) + v._data[ 2u ] * ( rr - aa - bb + cc );
}

GXQuat& GXQuat::operator = ( const GXVec4 &other )
{
    memcpy ( this, &other, sizeof ( GXQuat ) );
    return *this;
}

//---------------------------------------------------------------------------------------------------------------------

GXMat3::GXMat3 ()
{
    Zeros ();
}

GXMat3::GXMat3 ( const GXMat4 &matrix )
{
    From ( matrix );
}

GXMat3::GXMat3 ( const GXMat3 &other )
{
    memcpy ( this, &other, sizeof ( GXMat3 ) );
}

GXVoid GXMat3::From ( const GXQuat &quaternion )
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

GXVoid GXMat3::From ( const GXMat4 &matrix )
{
    GXUPointer lineSize = 3u * sizeof ( GXFloat );

    memcpy ( _data, matrix._data, lineSize );
    memcpy ( _data + 3u, matrix._data + 4u, lineSize );
    memcpy ( _data + 6u, matrix._data + 8u, lineSize );
}

GXVoid GXMat3::From ( const GXVec3 &zDirection )
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
}

GXVoid GXMat3::FromFast ( const GXQuat &quaternion )
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

GXVoid GXMat3::SetX ( const GXVec3 &x )
{
    memcpy ( _data, &x, sizeof ( GXVec3 ) );
}

GXVoid GXMat3::GetX ( GXVec3 &x ) const
{
    memcpy ( &x, _data, sizeof ( GXVec3 ) );
}

GXVoid GXMat3::SetY ( const GXVec3& y )
{
    memcpy ( _data + 3u, &y, sizeof ( GXVec3 ) );
}

GXVoid GXMat3::GetY ( GXVec3 &y ) const
{
    memcpy ( &y, _data + 3u, sizeof ( GXVec3 ) );
}

GXVoid GXMat3::SetZ ( const GXVec3 &z )
{
    memcpy ( _data + 6u, &z, sizeof ( GXVec3 ) );
}

GXVoid GXMat3::GetZ ( GXVec3 &z ) const
{
    memcpy ( &z, _data + 6u, sizeof ( GXVec3 ) );
}

GXVoid GXMat3::Identity ()
{
    _m[ 0u ][ 0u ] = _m[ 1u ][ 1u ] = _m[ 2u ][ 2u ] = 1.0f;
    _m[ 0u ][ 1u ] = _m[ 0u ][ 2u ] = 0.0f;
    _m[ 1u ][ 0u ] = _m[ 1u ][ 2u ] = 0.0f;
    _m[ 2u ][ 0u ] = _m[ 2u ][ 1u ] = 0.0f;
}

GXVoid GXMat3::Zeros ()
{
    memset ( _data, 0, 9u * sizeof ( GXFloat ) );
}

GXVoid GXMat3::Inverse ( const GXMat3 &sourceMatrix )
{
    GXFloat determinant = sourceMatrix._m[ 0u ][ 0u ] * ( sourceMatrix._m[ 1u ][ 1u ] * sourceMatrix._m[ 2u ][ 2u ] - sourceMatrix._m[ 2u ][ 1u ] * sourceMatrix._m[ 1u ][ 2u ] );
    determinant -= sourceMatrix._m[ 0u ][ 1u ] * ( sourceMatrix._m[ 1u ][ 0u ] * sourceMatrix._m[ 2u ][ 2u ] - sourceMatrix._m[ 2u ][ 0u ] * sourceMatrix._m[ 1u ][ 2u ] );
    determinant += sourceMatrix._m[ 0u ][ 2u ] * ( sourceMatrix._m[ 1u ][ 0u ] * sourceMatrix._m[ 2u ][ 1u ] - sourceMatrix._m[ 2u ][ 0u ] * sourceMatrix._m[ 1u ][ 1u ] );

    GXFloat invDeterminant = 1.0f / determinant;

    _m[ 0u ][ 0u ] = invDeterminant * ( sourceMatrix._m[ 1u ][ 1u ] * sourceMatrix._m[ 2u ][ 2u ] - sourceMatrix._m[ 2u ][ 1u ] * sourceMatrix._m[ 1u ][ 2u ] );
    _m[ 0u ][ 1u ] = invDeterminant * ( sourceMatrix._m[ 0u ][ 2u ] * sourceMatrix._m[ 2u ][ 1u ] - sourceMatrix._m[ 2u ][ 2u ] * sourceMatrix._m[ 0u ][ 1u ] );
    _m[ 0u ][ 2u ] = invDeterminant * ( sourceMatrix._m[ 0u ][ 1u ] * sourceMatrix._m[ 1u ][ 2u ] - sourceMatrix._m[ 1u ][ 1u ] * sourceMatrix._m[ 0u ][ 2u ] );

    _m[ 1u ][ 0u ] = invDeterminant * ( sourceMatrix._m[ 1u ][ 2u ] * sourceMatrix._m[ 2u ][ 0u ] - sourceMatrix._m[ 2u ][ 2u ] * sourceMatrix._m[ 1u ][ 0u ] );
    _m[ 1u ][ 1u ] = invDeterminant * ( sourceMatrix._m[ 0u ][ 0u ] * sourceMatrix._m[ 2u ][ 2u ] - sourceMatrix._m[ 2u ][ 0u ] * sourceMatrix._m[ 0u ][ 2u ] );
    _m[ 1u ][ 2u ] = invDeterminant * ( sourceMatrix._m[ 0u ][ 2u ] * sourceMatrix._m[ 1u ][ 0u ] - sourceMatrix._m[ 1u ][ 2u ] * sourceMatrix._m[ 0u ][ 0u ] );

    _m[ 2u ][ 0u ] = invDeterminant * ( sourceMatrix._m[ 1u ][ 0u ] * sourceMatrix._m[ 2u ][ 1u ] - sourceMatrix._m[ 2u ][ 0u ] * sourceMatrix._m[ 1u ][ 1u ] );
    _m[ 2u ][ 1u ] = invDeterminant * ( sourceMatrix._m[ 0u ][ 1u ] * sourceMatrix._m[ 2u ][ 0u ] - sourceMatrix._m[ 2u ][ 1u ] * sourceMatrix._m[ 0u ][ 0u ] );
    _m[ 2u ][ 2u ] = invDeterminant * ( sourceMatrix._m[ 0u ][ 0u ] * sourceMatrix._m[ 1u ][ 1u ] - sourceMatrix._m[ 1u ][ 0u ] * sourceMatrix._m[ 0u ][ 1u ] );
}

GXVoid GXMat3::Transponse ( const GXMat3 &sourceMatrix )
{
    _m[ 0u ][ 0u ] = sourceMatrix._m[ 0u ][ 0u ];
    _m[ 0u ][ 1u ] = sourceMatrix._m[ 1u ][ 0u ];
    _m[ 0u ][ 2u ] = sourceMatrix._m[ 2u ][ 0u ];

    _m[ 1u ][ 0u ] = sourceMatrix._m[ 0u ][ 1u ];
    _m[ 1u ][ 1u ] = sourceMatrix._m[ 1u ][ 1u ];
    _m[ 1u ][ 2u ] = sourceMatrix._m[ 2u ][ 1u ];

    _m[ 2u ][ 0u ] = sourceMatrix._m[ 0u ][ 2u ];
    _m[ 2u ][ 1u ] = sourceMatrix._m[ 1u ][ 2u ];
    _m[ 2u ][ 2u ] = sourceMatrix._m[ 2u ][ 2u ];
}

GXVoid GXMat3::ClearRotation ( const GXMat3 &sourceMatrix )
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
}

GXVoid GXMat3::ClearRotation ( const GXMat4 &sourceMatrix )
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
}

GXVoid GXMat3::SkewSymmetric ( const GXVec3 &base )
{
    _m[ 0u ][ 0u ] = _m[ 1u ][ 1u ] = _m[ 2u ][ 2u ] = 0.0f;

    _m[ 0u ][ 1u ] = -base._data[ 2u ];
    _m[ 0u ][ 2u ] = base._data[ 1u ];

    _m[ 1u ][ 0u ] = base._data[ 2u ];
    _m[ 1u ][ 2u ] = -base._data[ 0u ];

    _m[ 2u ][ 0u ] = -base._data[ 1u ];
    _m[ 2u ][ 1u ] = base._data[ 0u ];
}

GXVoid GXMat3::Sum ( const GXMat3 &a, const GXMat3 &b )
{
    _m[ 0u ][ 0u ] = a._m[ 0u ][ 0u ] + b._m[ 0u ][ 0u ];
    _m[ 0u ][ 1u ] = a._m[ 0u ][ 1u ] + b._m[ 0u ][ 1u ];
    _m[ 0u ][ 2u ] = a._m[ 0u ][ 2u ] + b._m[ 0u ][ 2u ];

    _m[ 1u ][ 0u ] = a._m[ 1u ][ 0u ] + b._m[ 1u ][ 0u ];
    _m[ 1u ][ 1u ] = a._m[ 1u ][ 1u ] + b._m[ 1u ][ 1u ];
    _m[ 1u ][ 2u ] = a._m[ 1u ][ 2u ] + b._m[ 1u ][ 2u ];

    _m[ 2u ][ 0u ] = a._m[ 2u ][ 0u ] + b._m[ 2u ][ 0u ];
    _m[ 2u ][ 1u ] = a._m[ 2u ][ 1u ] + b._m[ 2u ][ 1u ];
    _m[ 2u ][ 2u ] = a._m[ 2u ][ 2u ] + b._m[ 2u ][ 2u ];
}

GXVoid GXMat3::Substract ( const GXMat3 &a, const GXMat3 &b )
{
    _m[ 0u ][ 0u ] = a._m[ 0u ][ 0u ] - b._m[ 0u ][ 0u ];
    _m[ 0u ][ 1u ] = a._m[ 0u ][ 1u ] - b._m[ 0u ][ 1u ];
    _m[ 0u ][ 2u ] = a._m[ 0u ][ 2u ] - b._m[ 0u ][ 2u ];

    _m[ 1u ][ 0u ] = a._m[ 1u ][ 0u ] - b._m[ 1u ][ 0u ];
    _m[ 1u ][ 1u ] = a._m[ 1u ][ 1u ] - b._m[ 1u ][ 1u ];
    _m[ 1u ][ 2u ] = a._m[ 1u ][ 2u ] - b._m[ 1u ][ 2u ];

    _m[ 2u ][ 0u ] = a._m[ 2u ][ 0u ] - b._m[ 2u ][ 0u ];
    _m[ 2u ][ 1u ] = a._m[ 2u ][ 1u ] - b._m[ 2u ][ 1u ];
    _m[ 2u ][ 2u ] = a._m[ 2u ][ 2u ] - b._m[ 2u ][ 2u ];
}

GXVoid GXMat3::Multiply ( const GXMat3 &a, const GXMat3 &b )
{
    _m[ 0u ][ 0u ] = a._m[ 0u ][ 0u ] * b._m[ 0u ][ 0u ] + a._m[ 0u ][ 1u ] * b._m[ 1u ][ 0u ] + a._m[ 0u ][ 2u ] * b._m[ 2u ][ 0u ];
    _m[ 0u ][ 1u ] = a._m[ 0u ][ 0u ] * b._m[ 0u ][ 1u ] + a._m[ 0u ][ 1u ] * b._m[ 1u ][ 1u ] + a._m[ 0u ][ 2u ] * b._m[ 2u ][ 1u ];
    _m[ 0u ][ 2u ] = a._m[ 0u ][ 0u ] * b._m[ 0u ][ 2u ] + a._m[ 0u ][ 1u ] * b._m[ 1u ][ 2u ] + a._m[ 0u ][ 2u ] * b._m[ 2u ][ 2u ];

    _m[ 1u ][ 0u ] = a._m[ 1u ][ 0u ] * b._m[ 0u ][ 0u ] + a._m[ 1u ][ 1u ] * b._m[ 1u ][ 0u ] + a._m[ 1u ][ 2u ] * b._m[ 2u ][ 0u ];
    _m[ 1u ][ 1u ] = a._m[ 1u ][ 0u ] * b._m[ 0u ][ 1u ] + a._m[ 1u ][ 1u ] * b._m[ 1u ][ 1u ] + a._m[ 1u ][ 2u ] * b._m[ 2u ][ 1u ];
    _m[ 1u ][ 2u ] = a._m[ 1u ][ 0u ] * b._m[ 0u ][ 2u ] + a._m[ 1u ][ 1u ] * b._m[ 1u ][ 2u ] + a._m[ 1u ][ 2u ] * b._m[ 2u ][ 2u ];

    _m[ 2u ][ 0u ] = a._m[ 2u ][ 0u ] * b._m[ 0u ][ 0u ] + a._m[ 2u ][ 1u ] * b._m[ 1u ][ 0u ] + a._m[ 2u ][ 2u ] * b._m[ 2u ][ 0u ];
    _m[ 2u ][ 1u ] = a._m[ 2u ][ 0u ] * b._m[ 0u ][ 1u ] + a._m[ 2u ][ 1u ] * b._m[ 1u ][ 1u ] + a._m[ 2u ][ 2u ] * b._m[ 2u ][ 1u ];
    _m[ 2u ][ 2u ] = a._m[ 2u ][ 0u ] * b._m[ 0u ][ 2u ] + a._m[ 2u ][ 1u ] * b._m[ 1u ][ 2u ] + a._m[ 2u ][ 2u ] * b._m[ 2u ][ 2u ];
}

GXVoid GXMat3::MultiplyVectorMatrix ( GXVec3 &out, const GXVec3 &v ) const
{
    out._data[ 0u ] = v._data[ 0u ] * _m[ 0u ][ 0u ] + v._data[ 1u ] * _m[ 1u ][ 0u ] + v._data[ 2u ] * _m[ 2u ][ 0u ];
    out._data[ 1u ] = v._data[ 0u ] * _m[ 0u ][ 1u ] + v._data[ 1u ] * _m[ 1u ][ 1u ] + v._data[ 2u ] * _m[ 2u ][ 1u ];
    out._data[ 2u ] = v._data[ 0u ] * _m[ 0u ][ 2u ] + v._data[ 1u ] * _m[ 1u ][ 2u ] + v._data[ 2u ] * _m[ 2u ][ 2u ];
}

GXVoid GXMat3::MultiplyMatrixVector ( GXVec3 &out, const GXVec3 &v ) const
{
    out._data[ 0u ] = _m[ 0u ][ 0u ] * v._data[ 0u ] + _m[ 0u ][ 1u ] * v._data[ 1u ] + _m[ 0u ][ 2u ] * v._data[ 2u ];
    out._data[ 1u ] = _m[ 1u ][ 0u ] * v._data[ 0u ] + _m[ 1u ][ 1u ] * v._data[ 1u ] + _m[ 1u ][ 2u ] * v._data[ 2u ];
    out._data[ 2u ] = _m[ 2u ][ 0u ] * v._data[ 0u ] + _m[ 2u ][ 1u ] * v._data[ 1u ] + _m[ 2u ][ 2u ] * v._data[ 2u ];
}

GXVoid GXMat3::Multiply ( const GXMat3 &a, GXFloat factor )
{
    _m[ 0u ][ 0u ] = a._m[ 0u ][ 0u ] * factor;
    _m[ 0u ][ 1u ] = a._m[ 0u ][ 1u ] * factor;
    _m[ 0u ][ 2u ] = a._m[ 0u ][ 2u ] * factor;

    _m[ 1u ][ 0u ] = a._m[ 1u ][ 0u ] * factor;
    _m[ 1u ][ 1u ] = a._m[ 1u ][ 1u ] * factor;
    _m[ 1u ][ 2u ] = a._m[ 1u ][ 2u ] * factor;

    _m[ 2u ][ 0u ] = a._m[ 2u ][ 0u ] * factor;
    _m[ 2u ][ 1u ] = a._m[ 2u ][ 1u ] * factor;
    _m[ 2u ][ 2u ] = a._m[ 2u ][ 2u ] * factor;
}

GXMat3& GXMat3::operator = ( const GXMat3 &matrix )
{
    memcpy ( this, &matrix, sizeof ( GXMat3 ) );
    return *this;
}

//---------------------------------------------------------------------------------------------------------------------

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
