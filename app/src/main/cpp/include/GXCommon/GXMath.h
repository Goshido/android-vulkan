// version 1.62

#ifndef GX_MATH
#define GX_MATH


#include "GXTypes.h"
#include "GXWarning.h"

GX_DISABLE_COMMON_WARNINGS

#include <cmath>
#include <limits.h>

GX_RESTORE_WARNING_STATE


[[maybe_unused]] constexpr GXFloat const GX_MATH_HALF_PI = 1.5707963F;
[[maybe_unused]] constexpr GXFloat const GX_MATH_PI = 3.1415927F;
[[maybe_unused]] constexpr GXFloat const GX_MATH_DOUBLE_PI = 6.2831853F;

//----------------------------------------------------------------------------------------------------------------------

// By convention it is row-vertex.
struct [[maybe_unused]] GXVec2 final
{
    // Stores vector components in x, y order.
    GXFloat     _data[ 2U ];

    [[maybe_unused]] constexpr GXVec2 ():
        _data { 0.0F, 0.0F }
    {
        // NOTHING
    }

    [[maybe_unused]] GXVec2 ( GXVec2 const & ) = default;
    [[maybe_unused]] GXVec2& operator = ( GXVec2 const & ) = default;

    // constexpr constructor is implicitly inline
    // see https://timsong-cpp.github.io/cppwp/n4140/dcl.constexpr
    [[maybe_unused]] constexpr explicit GXVec2 ( GXFloat x, GXFloat y ):
        _data { x, y }
    {
        // NOTHING
    }

    [[maybe_unused]] GXVoid SetX ( GXFloat x );
    [[maybe_unused]] GXFloat GetX () const;

    [[maybe_unused]] GXVoid SetY ( GXFloat y );
    [[maybe_unused]] GXFloat GetY () const;

    [[maybe_unused]] GXVoid Init ( GXFloat x, GXFloat y );
    [[maybe_unused]] GXVoid Normalize ();

    // No normalization
    [[maybe_unused]] GXVoid CalculateNormalFast ( GXVec2 const &a, GXVec2 const &b );

    [[maybe_unused]] GXVoid CalculateNormal ( GXVec2 const &a, GXVec2 const &b );

    [[maybe_unused]] GXVoid Sum ( GXVec2 const &a, GXVec2 const &b );
    [[maybe_unused]] GXVoid Sum ( GXVec2 const &a, GXFloat bScale, GXVec2 const &b );
    [[maybe_unused]] GXVoid Subtract ( GXVec2 const &a, GXVec2 const &b );
    [[maybe_unused]] GXVoid Multiply ( GXVec2 const &a, GXVec2 const &b );
    [[maybe_unused]] GXVoid Multiply ( GXVec2 const &v, GXFloat scale );

    [[maybe_unused, nodiscard]] GXFloat DotProduct ( GXVec2 const &other ) const;
    [[maybe_unused, nodiscard]] GXFloat Length () const;
    [[maybe_unused, nodiscard]] GXFloat SquaredLength () const;

    [[maybe_unused, nodiscard]] GXBool IsEqual ( GXVec2 const &other ) const;
};

//----------------------------------------------------------------------------------------------------------------------

enum class eGXLineRelationship : GXUByte
{
    NoIntersection [[maybe_unused]] = 0U,
    Intersection [[maybe_unused]] = 1U,
    Overlap [[maybe_unused]] = 2U
};

[[maybe_unused]] eGXLineRelationship GXCALL GXLineIntersection2D ( GXVec2 &intersectionPoint,
    GXVec2 const &a0,
    GXVec2 const &a1,
    GXVec2 const &b0,
    GXVec2 const &b1
);

//----------------------------------------------------------------------------------------------------------------------

// By convention it is row-vector.
struct [[maybe_unused]] GXVec3 final
{
    // Stores vector components in x, y, z order.
    GXFloat     _data[ 3U ];

    [[maybe_unused]] constexpr GXVec3 ():
        _data { 0.0F, 0.0F, 0.0F }
    {
        // NOTHING
    }

    [[maybe_unused]] GXVec3 ( GXVec3 const & ) = default;
    [[maybe_unused]] GXVec3& operator = ( GXVec3 const & ) = default;

    // constexpr constructor is implicitly inline
    // see https://timsong-cpp.github.io/cppwp/n4140/dcl.constexpr
    [[maybe_unused]] constexpr explicit GXVec3 ( GXFloat x, GXFloat y, GXFloat z ):
        _data { x, y, z }
    {
        // NOTHING
    }

    [[maybe_unused]] GXVoid SetX ( GXFloat x );
    [[maybe_unused, nodiscard]] GXFloat GetX () const;

    [[maybe_unused]] GXVoid SetY ( GXFloat y );
    [[maybe_unused, nodiscard]] GXFloat GetY () const;

    [[maybe_unused]] GXVoid SetZ ( GXFloat z );
    [[maybe_unused, nodiscard]] GXFloat GetZ () const;

    [[maybe_unused]] GXVoid Init ( GXFloat x, GXFloat y, GXFloat z );
    [[maybe_unused]] GXVoid Normalize ();
    [[maybe_unused]] GXVoid Reverse ();

    [[maybe_unused]] GXVoid Sum ( GXVec3 const &a, GXVec3 const &b );
    [[maybe_unused]] GXVoid Sum ( GXVec3 const &a, GXFloat bScale, GXVec3 const &b );
    [[maybe_unused]] GXVoid Subtract ( GXVec3 const &a, GXVec3 const &b );
    [[maybe_unused]] GXVoid Multiply ( GXVec3 const &a, GXFloat scale );
    [[maybe_unused]] GXVoid Multiply ( GXVec3 const &a, GXVec3 const &b );

    [[maybe_unused, nodiscard]] GXFloat DotProduct ( GXVec3 const &other ) const;
    [[maybe_unused]] GXVoid CrossProduct ( GXVec3 const &a, GXVec3 const &b );

    [[maybe_unused, nodiscard]] GXFloat Length () const;
    [[maybe_unused, nodiscard]] GXFloat SquaredLength () const;
    [[maybe_unused, nodiscard]] GXFloat Distance ( GXVec3 const &other ) const;
    [[maybe_unused, nodiscard]] GXFloat SquaredDistance ( GXVec3 const &other ) const;

    [[maybe_unused]] GXVoid LinearInterpolation ( GXVec3 const &start,
        GXVec3 const &finish,
        GXFloat interpolationFactor
    );

    [[maybe_unused]] GXVoid Project ( GXVec3 const &vector, GXVec3 const &axis );
    [[maybe_unused, nodiscard]] GXBool IsEqual ( GXVec3 const &other );

    [[maybe_unused, nodiscard]] static GXVec3 const& GetAbsoluteX ();
    [[maybe_unused, nodiscard]] static GXVec3 const& GetAbsoluteY ();
    [[maybe_unused, nodiscard]] static GXVec3 const& GetAbsoluteZ ();

    //baseX - correct direction, adjustedY - desirable, adjustedZ - calculated.
    [[maybe_unused]] static GXVoid GXCALL MakeOrthonormalBasis ( GXVec3 &baseX, GXVec3 &adjustedY, GXVec3 &adjustedZ );
};

//----------------------------------------------------------------------------------------------------------------------

[[maybe_unused]] GXBool GXCALL GXRayTriangleIntersection3D ( GXFloat &outT,
    GXVec3 const &origin,
    GXVec3 const &direction,
    GXFloat length,
    GXVec3 const &a,
    GXVec3 const &b,
    GXVec3 const &c
);

//----------------------------------------------------------------------------------------------------------------------

struct [[maybe_unused]] GXEuler final
{
    GXFloat     _pitchRadians;
    GXFloat     _yawRadians;
    GXFloat     _rollRadians;

    [[maybe_unused]] constexpr GXEuler ():
        _pitchRadians ( 0.0F ),
        _yawRadians ( 0.0F ),
        _rollRadians ( 0.0F )
    {
        // NOTHING
    }

    [[maybe_unused]] GXEuler ( GXEuler const & ) = default;
    [[maybe_unused]] GXEuler& operator = ( GXEuler const & ) = default;

    // constexpr constructor is implicitly inline
    // see https://timsong-cpp.github.io/cppwp/n4140/dcl.constexpr
    [[maybe_unused]] constexpr explicit GXEuler ( GXFloat pitchRadians, GXFloat yawRadians, GXFloat rollRadians ):
        _pitchRadians ( pitchRadians ),
        _yawRadians ( yawRadians ),
        _rollRadians ( rollRadians )
    {
        // NOTHING
    }
};

//----------------------------------------------------------------------------------------------------------------------

// By convention it is row-vector.
struct [[maybe_unused]] GXVec4 final
{
    // Stores vector components in x, y, z, w order.
    GXFloat     _data[ 4U ];

    [[maybe_unused]] constexpr GXVec4 ():
        _data { 0.0F, 0.0F, 0.0F, 0.0F }
    {
        // NOTHING
    }

    [[maybe_unused]] GXVec4 ( GXVec4 const & ) = default;
    [[maybe_unused]] GXVec4& operator = ( GXVec4 const & ) = default;

    [[maybe_unused]] explicit GXVec4 ( GXVec3 const &vector, GXFloat w );

    // constexpr constructor is implicitly inline
    // see https://timsong-cpp.github.io/cppwp/n4140/dcl.constexpr
    [[maybe_unused]] constexpr explicit GXVec4 ( GXFloat x, GXFloat y, GXFloat z, GXFloat w ):
        _data { x, y, z, w }
    {
        // NOTHING
    }

    [[maybe_unused]] GXVoid Init ( GXFloat x, GXFloat y, GXFloat z, GXFloat w );

    [[maybe_unused]] GXVoid SetX ( GXFloat x );
    [[maybe_unused, nodiscard]] GXFloat GetX () const;

    [[maybe_unused]] GXVoid SetY ( GXFloat y );
    [[maybe_unused, nodiscard]] GXFloat GetY () const;

    [[maybe_unused]] GXVoid SetZ ( GXFloat z );
    [[maybe_unused, nodiscard]] GXFloat GetZ () const;

    [[maybe_unused]] GXVoid SetW ( GXFloat w );
    [[maybe_unused, nodiscard]] GXFloat GetW () const;

    [[maybe_unused]] GXVoid Sum ( GXVec4 const &a, GXVec4 const &b );
    [[maybe_unused]] GXVoid Sum ( GXVec4 const &a, GXFloat bScale, GXVec4 const &b );
    [[maybe_unused]] GXVoid Subtract ( GXVec4 const &a, GXVec4 const &b );

    [[maybe_unused, nodiscard]] GXFloat DotProduct ( GXVec4 const &other ) const;

    [[maybe_unused, nodiscard]] GXFloat Length () const;
    [[maybe_unused, nodiscard]] GXFloat SquaredLength () const;
};

//----------------------------------------------------------------------------------------------------------------------

struct [[maybe_unused]] GXVec6 final
{
    GXFloat     _data[ 6U ];

    [[maybe_unused]] GXVec6 () = default;

    [[maybe_unused]] GXVec6 ( GXVec6 const & ) = default;
    [[maybe_unused]] GXVec6& operator = ( GXVec6 const & ) = default;

    // constexpr constructor is implicitly inline
    // see https://timsong-cpp.github.io/cppwp/n4140/dcl.constexpr
    [[maybe_unused]] constexpr explicit GXVec6 ( GXFloat a1,
        GXFloat a2,
        GXFloat a3,
        GXFloat a4,
        GXFloat a5,
        GXFloat a6
    ):
        _data { a1, a2, a3, a4, a5, a6 }
    {
        // NOTHING
    }

    [[maybe_unused]] GXVoid Init ( GXFloat a1, GXFloat a2, GXFloat a3, GXFloat a4, GXFloat a5, GXFloat a6 );
    [[maybe_unused]] GXVoid From ( GXVec3 const &v1, GXVec3 const &v2 );

    [[maybe_unused, nodiscard]] GXFloat DotProduct ( GXVec6 const &other ) const;
    [[maybe_unused]] GXVoid Sum ( GXVec6 const &a, GXVec6 const &b );
    [[maybe_unused]] GXVoid Sum ( GXVec6 const &a, GXFloat bScale, GXVec6 const &b );
    [[maybe_unused]] GXVoid Multiply ( GXVec6 const &a, GXFloat factor );
};

//----------------------------------------------------------------------------------------------------------------------

struct GXColorHSV;
struct [[maybe_unused]] GXColorRGB final
{
    // Stores components in red, green, blue, alpha order.
    GXFloat     _data[ 4U ];

    [[maybe_unused]] constexpr GXColorRGB ():
        _data { 0.0F, 0.0F, 0.0F, 0.0F }
    {
        // NOTHING
    }

    [[maybe_unused]] GXColorRGB ( GXColorRGB const & ) = default;
    [[maybe_unused]] GXColorRGB& operator = ( GXColorRGB const & ) = default;

    // constexpr constructor is implicitly inline
    // see https://timsong-cpp.github.io/cppwp/n4140/dcl.constexpr
    [[maybe_unused]] constexpr explicit GXColorRGB ( GXFloat red, GXFloat green, GXFloat blue, GXFloat alpha ):
        _data { red, green, blue, alpha }
    {
        // NOTHING
    }

    [[maybe_unused]] explicit GXColorRGB ( GXUByte red, GXUByte green, GXUByte blue, GXFloat alpha );
    [[maybe_unused]] explicit GXColorRGB ( GXColorHSV const &color );

    [[maybe_unused]] GXVoid Init ( GXFloat red, GXFloat green, GXFloat blue, GXFloat alpha );

    // [0.0F +inf)
    [[maybe_unused]] GXVoid SetRed ( GXFloat red );
    [[maybe_unused, nodiscard]] GXFloat GetRed () const;

    // [0.0F +inf)
    [[maybe_unused]] GXVoid SetGreen ( GXFloat green );
    [[maybe_unused, nodiscard]] GXFloat GetGreen () const;

    // [0.0F +inf)
    [[maybe_unused]] GXVoid SetBlue ( GXFloat blue );
    [[maybe_unused, nodiscard]] GXFloat GetBlue () const;

    // [0.0f 1.0F]
    [[maybe_unused]] GXVoid SetAlpha ( GXFloat alpha );
    [[maybe_unused, nodiscard]] GXFloat GetAlpha () const;

    [[maybe_unused]] GXVoid From ( GXUByte red, GXUByte green, GXUByte blue, GXFloat alpha );
    [[maybe_unused]] GXVoid From ( GXColorHSV const &color );

    [[maybe_unused]] GXVoid ConvertToUByte ( GXUByte &red, GXUByte &green, GXUByte &blue, GXUByte &alpha ) const;
};

//----------------------------------------------------------------------------------------------------------------------

struct [[maybe_unused]] GXColorHSV final
{
    // Stores components in hue, saturation, value, alpha order.
    GXFloat     _data[ 4u ];

    [[maybe_unused]] constexpr GXColorHSV ():
        _data { 0.0F, 0.0F, 0.0F, 0.0F }
    {
        // NOTHING
    }

    [[maybe_unused]] GXColorHSV ( GXColorHSV const & ) = default;
    [[maybe_unused]] GXColorHSV& operator = ( GXColorHSV const & ) = default;

    // constexpr constructor is implicitly inline
    // see https://timsong-cpp.github.io/cppwp/n4140/dcl.constexpr
    [[maybe_unused]] constexpr explicit GXColorHSV ( GXFloat hue, GXFloat saturation, GXFloat value, GXFloat alpha ):
        _data { hue, saturation, value, alpha }
    {
        // NOTHING
    }

    [[maybe_unused]] explicit GXColorHSV ( GXColorRGB const &color );

    // [0.0F 360.0F]
    [[maybe_unused]] GXVoid SetHue ( GXFloat hue );
    [[maybe_unused, nodiscard]] GXFloat GetHue () const;

    // [0.0F 100.0F]
    [[maybe_unused]] GXVoid SetSaturation ( GXFloat saturation );
    [[maybe_unused, nodiscard]] GXFloat GetSaturation () const;

    // [0.0F 100.0F]
    [[maybe_unused]] GXVoid SetValue ( GXFloat value );
    [[maybe_unused, nodiscard]] GXFloat GetValue () const;

    // [0.0F 100.0F]
    [[maybe_unused]] GXVoid SetAlpha ( GXFloat alpha );
    [[maybe_unused, nodiscard]] GXFloat GetAlpha () const;

    [[maybe_unused]] GXVoid From ( GXColorRGB const &color );
};

//----------------------------------------------------------------------------------------------------------------------

struct [[maybe_unused]] GXPreciseComplex final
{
    GXDouble    _r;
    GXDouble    _i;

    [[maybe_unused]] GXPreciseComplex () = default;

    [[maybe_unused]] constexpr GXPreciseComplex ( GXPreciseComplex const &other ):
        _r ( other._r ),
        _i ( other._i )
    {
        // NOTHING
    }

    [[maybe_unused]] GXPreciseComplex& operator = ( GXPreciseComplex const &other ) = default;

    // constexpr constructor is implicitly inline
    // see https://timsong-cpp.github.io/cppwp/n4140/dcl.constexpr
    [[maybe_unused]] explicit GXPreciseComplex ( GXDouble real, GXDouble imaginary ):
        _r ( real ),
        _i ( imaginary )
    {
        // NOTHING
    }

    [[maybe_unused]] GXVoid Init ( GXDouble real, GXDouble imaginary );

    [[maybe_unused, nodiscard]] GXDouble Length () const;
    [[maybe_unused, nodiscard]] GXDouble SquaredLength () const;

    // Method returns GX_FALSE if ( 0.0 + 0.0i ) ^ 0 will happen.
    [[maybe_unused]] GXBool Power ( GXUInt power );

    [[maybe_unused]] GXPreciseComplex operator + ( GXPreciseComplex const &other ) const;
    [[maybe_unused]] GXPreciseComplex operator - ( GXPreciseComplex const &other ) const;
    [[maybe_unused]] GXPreciseComplex operator * ( GXPreciseComplex const &other ) const;
    [[maybe_unused]] GXPreciseComplex operator * ( GXDouble a ) const;
    [[maybe_unused]] GXPreciseComplex operator / ( GXDouble a ) const;
};

//----------------------------------------------------------------------------------------------------------------------

struct GXMat3;
struct GXMat4;

// Quaternion representation: r + ai + bj + ck.
// By convention stores only orientation without any scale.
struct [[maybe_unused]] GXQuat final
{
    // Stores quaternion components in r, a, b, c order.
    GXFloat     _data[ 4u ];

    [[maybe_unused]] constexpr GXQuat ():
        _data { 0.0F, 0.0F, 0.0F, 0.0F }
    {
        // NOTHING
    }

    [[maybe_unused]] GXQuat ( GXQuat const & ) = default;
    [[maybe_unused]] GXQuat& operator = ( GXQuat const & ) = default;

    // constexpr constructor is implicitly inline
    // see https://timsong-cpp.github.io/cppwp/n4140/dcl.constexpr
    [[maybe_unused]] constexpr explicit GXQuat ( GXFloat r, GXFloat a, GXFloat b, GXFloat c ):
        _data { r, a, b, c }
    {
        // NOTHING
    }

    // Result is valid if rotationMatrix is rotation matrix. Any scale will be ignored.
    [[maybe_unused]] explicit GXQuat ( GXMat3 const &rotationMatrix );

    // Result is valid if rotationMatrix is rotation matrix. Any scale will be ignored.
    [[maybe_unused]] explicit GXQuat ( GXMat4 const &rotationMatrix );

    [[maybe_unused]] GXVoid Init ( GXFloat r, GXFloat a, GXFloat b, GXFloat c );

    [[maybe_unused]] GXVoid SetR ( GXFloat r );
    [[maybe_unused, nodiscard]] GXFloat GetR () const;

    [[maybe_unused]] GXVoid SetA ( GXFloat a );
    [[maybe_unused, nodiscard]] GXFloat GetA () const;

    [[maybe_unused]] GXVoid SetB ( GXFloat b );
    [[maybe_unused, nodiscard]] GXFloat GetB () const;

    [[maybe_unused]] GXVoid SetC ( GXFloat c );
    [[maybe_unused, nodiscard]] GXFloat GetC () const;

    [[maybe_unused]] GXVoid Identity ();
    [[maybe_unused]] GXVoid Normalize ();
    [[maybe_unused]] GXVoid Inverse ( GXQuat const &q );
    [[maybe_unused]] GXVoid FromAxisAngle ( GXFloat x, GXFloat y, GXFloat z, GXFloat angle );
    [[maybe_unused]] GXVoid FromAxisAngle ( GXVec3 const &axis, GXFloat angle );

    // Result is valid if rotationMatrix is rotation matrix. Any scale will be ignored.
    [[maybe_unused]] GXVoid From ( GXMat3 const &rotationMatrix );

    // Result is valid if rotationMatrix is rotation matrix. Any scale will be ignored.
    [[maybe_unused]] GXVoid From ( GXMat4 const &rotationMatrix );

    // Result is valid if pureRotationMatrix is not scaled rotation matrix.
    [[maybe_unused]] GXVoid FromFast ( GXMat3 const &pureRotationMatrix );

    // Result is valid if pureRotationMatrix is not scaled rotation matrix.
    [[maybe_unused]] GXVoid FromFast ( GXMat4 const &pureRotationMatrix );

    [[maybe_unused]] GXVoid Multiply ( GXQuat const &a, GXQuat const &b );
    [[maybe_unused]] GXVoid Multiply ( GXQuat const &q, GXFloat scale );
    [[maybe_unused]] GXVoid Sum ( GXQuat const &a, GXQuat const &b );
    [[maybe_unused]] GXVoid Substract ( GXQuat const &a, GXQuat const &b );

    [[maybe_unused]] GXVoid SphericalLinearInterpolation ( GXQuat const &start,
        GXQuat const &finish,
        GXFloat interpolationFactor
    );

    [[maybe_unused]] GXVoid GetAxisAngle ( GXVec3 &axis, GXFloat &angle ) const;
    [[maybe_unused]] GXVoid Transform ( GXVec3 &out, GXVec3 const &v ) const;

    // Result is valid if quaternion is normalized.
    [[maybe_unused]] GXVoid TransformFast ( GXVec3 &out, GXVec3 const &v ) const;
};

//----------------------------------------------------------------------------------------------------------------------

struct [[maybe_unused]] GXMat3 final
{
    union
    {
        GXFloat     _data[ 9u ];
        GXFloat     _m[ 3u ][ 3u ];
    };

    [[maybe_unused]] constexpr GXMat3 ():
        _data
        {
            0.0F, 0.0F, 0.0F,
            0.0F, 0.0F, 0.0F,
            0.0F, 0.0F, 0.0F
        }
    {
        // NOTHING
    }

    [[maybe_unused]] GXMat3 ( GXMat3 const & ) = default;
    [[maybe_unused]] GXMat3& operator = ( GXMat3 const & ) = default;

    [[maybe_unused]] explicit GXMat3 ( GXMat4 const &matrix );

    [[maybe_unused]] GXVoid From ( GXQuat const &quaternion );
    [[maybe_unused]] GXVoid From ( GXMat4 const &matrix );

    // Constructs orthonormal basis. Result is valid if zDirection is unit vector.
    [[maybe_unused]] GXVoid From ( GXVec3 const &zDirection );

    // Result is valid if quaternion is normalized.
    [[maybe_unused]] GXVoid FromFast ( GXQuat const &quaternion );

    [[maybe_unused]] GXVoid SetX ( GXVec3 const &x );
    [[maybe_unused]] GXVoid GetX ( GXVec3 &x ) const;

    [[maybe_unused]] GXVoid SetY ( GXVec3 const &y );
    [[maybe_unused]] GXVoid GetY ( GXVec3 &y ) const;

    [[maybe_unused]] GXVoid SetZ ( GXVec3 const &z );
    [[maybe_unused]] GXVoid GetZ ( GXVec3 &z ) const;

    [[maybe_unused]] GXVoid Identity ();
    [[maybe_unused]] GXVoid Zeros ();

    [[maybe_unused]] GXVoid Inverse ( GXMat3 const &sourceMatrix );
    [[maybe_unused]] GXVoid Transpose ( GXMat3 const &sourceMatrix );
    [[maybe_unused]] GXVoid ClearRotation ( GXMat3 const &sourceMatrix );
    [[maybe_unused]] GXVoid ClearRotation ( GXMat4 const &sourceMatrix );

    // It is cross product in matrix form.
    // Proper result will be achieved for this construction only:
    // a x b = c
    //
    // GXVec3 a ( ... );
    // GXVec3 b ( ... );
    //
    // GXMat3 skew;
    // skew.SkewSymmetric ( b );
    //
    // GXVec3 c;
    // skew.MultiplyVectorMatrix ( c, a );
    [[maybe_unused]] GXVoid SkewSymmetric ( GXVec3 const &base );

    [[maybe_unused]] GXVoid Sum ( GXMat3 const &a, GXMat3 const &b );
    [[maybe_unused]] GXVoid Subtract ( GXMat3 const &a, GXMat3 const &b );
    [[maybe_unused]] GXVoid Multiply ( GXMat3 const &a, GXMat3 const &b );

    [[maybe_unused]] GXVoid MultiplyVectorMatrix ( GXVec3 &out, GXVec3 const &v ) const;
    [[maybe_unused]] GXVoid MultiplyMatrixVector ( GXVec3 &out, GXVec3 const &v ) const;

    [[maybe_unused]] GXVoid Multiply ( GXMat3 const &a, GXFloat factor );
};

//----------------------------------------------------------------------------------------------------------------------

struct [[maybe_unused]] GXMat4 final
{
    union
    {
        GXFloat     _data[ 16u ];
        GXFloat     _m[ 4u ][ 4u ];
    };

    [[maybe_unused]] constexpr GXMat4 ():
        _data
        {
            0.0F, 0.0F, 0.0F, 0.0F,
            0.0F, 0.0F, 0.0F, 0.0F,
            0.0F, 0.0F, 0.0F, 0.0F,
            0.0F, 0.0F, 0.0F, 0.0F
        }
    {
        // NOTHING
    }

    [[maybe_unused]] GXMat4 ( GXMat4 const & ) = default;
    [[maybe_unused]] GXMat4& operator = ( GXMat4 const & ) = default;

    [[maybe_unused]] GXVoid SetRotation ( GXQuat const &quaternion );

    // Result is valid if quaternion is normalized.
    [[maybe_unused]] GXVoid SetRotationFast ( GXQuat const &quaternion );

    [[maybe_unused]] GXVoid SetOrigin ( GXVec3 const &origin );
    [[maybe_unused]] GXVoid From ( GXQuat const &quaternion, GXVec3 const &origin );
    [[maybe_unused]] GXVoid From ( GXMat3 const &rotation, GXVec3 const &origin );
    [[maybe_unused]] GXVoid From ( GXVec3 const &zDirection, GXVec3 const &origin );

    // Result is valid if quaternion is normalized.
    [[maybe_unused]] GXVoid FromFast ( GXQuat const &quaternion, GXVec3 const &origin );

    [[maybe_unused]] GXVoid SetX ( GXVec3 const &x );
    [[maybe_unused]] GXVoid GetX ( GXVec3 &x ) const;

    [[maybe_unused]] GXVoid SetY ( GXVec3 const &y );
    [[maybe_unused]] GXVoid GetY ( GXVec3 &y ) const;
    
    [[maybe_unused]] GXVoid SetZ ( GXVec3 const &z );
    [[maybe_unused]] GXVoid GetZ ( GXVec3 &z ) const;
    
    [[maybe_unused]] GXVoid SetW ( GXVec3 const &w );
    [[maybe_unused]] GXVoid GetW ( GXVec3 &w ) const;

    [[maybe_unused]] GXVoid Identity ();

    [[maybe_unused]] GXVoid Perspective ( GXFloat fieldOfViewYRadiands,
        GXFloat aspectRatio,
        GXFloat zNear,
        GXFloat zFar
    );

    [[maybe_unused]] GXVoid Ortho ( GXFloat width, GXFloat height, GXFloat zNear, GXFloat zFar );

    [[maybe_unused]] GXVoid Translation ( GXFloat x, GXFloat y, GXFloat z );
    [[maybe_unused]] GXVoid TranslateTo ( GXFloat x, GXFloat y, GXFloat z );
    [[maybe_unused]] GXVoid TranslateTo ( GXVec3 const &location );

    [[maybe_unused]] GXVoid RotationX ( GXFloat angle );
    [[maybe_unused]] GXVoid RotationY ( GXFloat angle );
    [[maybe_unused]] GXVoid RotationZ ( GXFloat angle );
    [[maybe_unused]] GXVoid RotationXY ( GXFloat pitchRadians, GXFloat yawRadians );
    [[maybe_unused]] GXVoid RotationXYZ ( GXFloat pitchRadians, GXFloat yawRadians, GXFloat rollRadians );
    [[maybe_unused]] GXVoid ClearRotation ( GXMat3 const &sourceMatrix );
    [[maybe_unused]] GXVoid ClearRotation ( GXMat4 const &sourceMatrix );

    [[maybe_unused]] GXVoid Scale ( GXFloat x, GXFloat y, GXFloat z );
    [[maybe_unused]] GXVoid ClearScale ( GXVec3 &scale ) const;

    [[maybe_unused]] GXVoid Inverse ( GXMat4 const &sourceMatrix );

    [[maybe_unused]] GXVoid Multiply ( GXMat4 const &a, GXMat4 const &b );

    // Multiply row-vector [1x4] by own matrix [4x4].
    [[maybe_unused]] GXVoid MultiplyVectorMatrix ( GXVec4 &out, GXVec4 const &v ) const;

    // Multiply own matrix [4x4] by column-vector [4x1].
    [[maybe_unused]] GXVoid MultiplyMatrixVector ( GXVec4 &out, GXVec4 const &v ) const;

    // Multiply row-vector [1x3] by own matrix sub matrix [3x3].
    [[maybe_unused]] GXVoid MultiplyAsNormal ( GXVec3 &out, GXVec3 const &v ) const;

    // Multiply row-vector [1x3] by own matrix sub matrix [3x3] and add own w-vector.
    [[maybe_unused]] GXVoid MultiplyAsPoint ( GXVec3 &out, GXVec3 const &v ) const;

    // Result is valid if own matrix is perspective matrix.
    [[maybe_unused]] GXVoid GetPerspectiveParams ( GXFloat &fieldOfViewYRadiands,
        GXFloat &aspectRatio,
        GXFloat &zNear,
        GXFloat &zFar
    );

    // Result is valid if own matrix is ortho matrix.
    [[maybe_unused]] GXVoid GetOrthoParams ( GXFloat &width, GXFloat &height, GXFloat &zNear, GXFloat &zFar );

    // Result is valid if own matrix is perspective matrix.
    [[maybe_unused]] GXVoid GetRayPerspective ( GXVec3 &rayView, GXVec2 const &mouseCVV ) const;
};

//----------------------------------------------------------------------------------------------------------------------

struct [[maybe_unused]] GXAABB final
{
    GXUByte     _vertices;

    GXVec3      _min;
    GXVec3      _max;

    [[maybe_unused]] constexpr GXAABB ():
        _vertices ( 0U ),
        _min ( FLT_MAX, FLT_MAX, FLT_MAX ),
        _max ( -FLT_MAX, -FLT_MAX, -FLT_MAX )
    {
        // NOTHING
    }

    [[maybe_unused]] GXAABB ( GXAABB const & ) = default;
    [[maybe_unused]] GXAABB& operator = ( GXAABB const & ) = default;

    [[maybe_unused]] GXVoid Empty ();

    [[maybe_unused]] GXVoid Transform ( GXAABB &bounds, GXMat4 const &transform ) const;
    [[maybe_unused]] GXVoid AddVertex ( GXVec3 const &vertex );
    [[maybe_unused]] GXVoid AddVertex ( GXFloat x, GXFloat y, GXFloat z );

    [[maybe_unused, nodiscard]] GXBool IsOverlaped ( GXAABB const &other ) const;
    [[maybe_unused, nodiscard]] GXBool IsOverlaped ( GXVec3 const &point ) const;
    [[maybe_unused, nodiscard]] GXBool IsOverlaped ( GXFloat x, GXFloat y, GXFloat z ) const;

    [[maybe_unused]] GXVoid GetCenter ( GXVec3 &center ) const;
    [[maybe_unused, nodiscard]] GXFloat GetWidth () const;
    [[maybe_unused, nodiscard]] GXFloat GetHeight () const;
    [[maybe_unused, nodiscard]] GXFloat GetDepth () const;
    [[maybe_unused, nodiscard]] GXFloat GetSphereRadius () const;
};

//----------------------------------------------------------------------------------------------------------------------

enum class eGXPlaneClassifyVertex : GXUByte
{
    InFront [[maybe_unused]] = 0U,
    On [[maybe_unused]] = 1U,
    Behind [[maybe_unused]] = 2U
};

struct [[maybe_unused]] GXPlane final
{
    GXFloat     _a;
    GXFloat     _b;
    GXFloat     _c;
    GXFloat     _d;

    [[maybe_unused]] constexpr GXPlane ():
        _a ( 0.0F ),
        _b ( 1.0F ),
        _c ( 0.0F ),
        _d ( 0.0F )
    {
        // NOTHING
    }

    [[maybe_unused]] GXPlane ( GXPlane const & ) = default;
    [[maybe_unused]] GXPlane& operator = ( GXPlane const & ) = default;

    [[maybe_unused]] GXVoid From ( GXVec3 const &pointA, GXVec3 const &pointB, GXVec3 const &pointC );
    [[maybe_unused]] GXVoid FromLineToPoint ( GXVec3 const &lineStart, GXVec3 const &lineEnd, GXVec3 const &point );

    [[maybe_unused]] GXVoid Normalize ();
    [[maybe_unused]] GXVoid Flip ();

    [[maybe_unused, nodiscard]] eGXPlaneClassifyVertex ClassifyVertex ( GXVec3 const &vertex ) const;
    [[maybe_unused, nodiscard]] eGXPlaneClassifyVertex ClassifyVertex ( GXFloat x, GXFloat y, GXFloat z ) const;
};

//----------------------------------------------------------------------------------------------------------------------

class [[maybe_unused]] GXProjectionClipPlanes final
{
    private:
        GXPlane     _planes[ 6U ];

    public:
        [[maybe_unused]] constexpr GXProjectionClipPlanes () = default;
        [[maybe_unused]] GXProjectionClipPlanes& operator = ( GXProjectionClipPlanes const & ) = default;

        [[maybe_unused]] explicit GXProjectionClipPlanes ( GXMat4 const &src );

        // Normals will be directed inside view volume.
        [[maybe_unused]] GXVoid From ( GXMat4 const &src );

        // Trivial invisibility test.
        [[maybe_unused, nodiscard]] GXBool IsVisible ( GXAABB const &bounds ) const;

    private:
        [[nodiscard]] GXUByte PlaneTest ( GXFloat x, GXFloat y, GXFloat z ) const;
};

//---------------------------------------------------------------------------------------------------------------------

[[maybe_unused, nodiscard]] GXFloat GXCALL GXDegToRad ( GXFloat degrees );
[[maybe_unused, nodiscard]] GXFloat GXCALL GXRadToDeg ( GXFloat radians );

[[maybe_unused]] GXVoid GXCALL GXConvert3DSMaxToGXEngine ( GXVec3 &gx_out,
    GXFloat max_x,
    GXFloat max_y,
    GXFloat max_z
);

[[maybe_unused]] GXVoid GXCALL GXRandomize ();
[[maybe_unused, nodiscard]] GXFloat GXCALL GXRandomNormalize ();
[[maybe_unused, nodiscard]] GXFloat GXCALL GXRandomBetween ( GXFloat from, GXFloat to );
[[maybe_unused]] GXVoid GXCALL GXRandomBetween ( GXVec3 &out, GXVec3 const &from, GXVec3 const &to );

[[maybe_unused]] GXVoid GXCALL GXGetTangentBitangent ( GXVec3 &outTangent,
    GXVec3 &outBitangent,
    GXUByte vertexID,
    GXUByte const* vertices,
    GXUPointer vertexStride,
    GXUByte const* uvs,
    GXUPointer uvStride
);

[[maybe_unused, nodiscard]] GXFloat GXCALL GXClampf ( GXFloat value, GXFloat minValue, GXFloat maxValue );
[[maybe_unused, nodiscard]] GXInt GXCALL GXClampi ( GXInt value, GXInt minValue, GXInt maxValue );

[[maybe_unused, nodiscard]] GXFloat GXCALL GXMinf ( GXFloat a, GXFloat b );
[[maybe_unused, nodiscard]] GXFloat GXCALL GXMaxf ( GXFloat a, GXFloat b );

[[maybe_unused]] GXVoid GXCALL GXGetBarycentricCoords ( GXVec3 &out,
    GXVec3 const &point,
    GXVec3 const &aPivot,
    GXVec3 const &bPivot,
    GXVec3 const &cPivot
);

[[maybe_unused]] GXVoid GXCALL GXGetRayFromViewer ( GXVec3 &origin,
    GXVec3 &direction,
    GXUShort x,
    GXUShort y,
    GXUShort viewportWidth,
    GXUShort viewportHeight,
    GXVec3 const &viewerLocation,
    GXMat4 const &viewProjectionMatrix
);


#endif // GX_MATH
