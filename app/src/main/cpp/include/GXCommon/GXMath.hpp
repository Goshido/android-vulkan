// version 1.92

#ifndef GX_MATH_HPP
#define GX_MATH_HPP

#include "GXTypes.hpp"
#include "GXWarning.hpp"

GX_DISABLE_COMMON_WARNINGS

#include <cfloat>
#include <climits>
#include <cmath>

GX_RESTORE_WARNING_STATE


[[maybe_unused]] constexpr GXFloat GX_MATH_FLOAT_EPSILON = 1.0e-4F;

[[maybe_unused]] constexpr GXFloat GX_MATH_HALF_PI = 1.5707963F;
[[maybe_unused]] constexpr GXFloat GX_MATH_PI = 3.1415927F;
[[maybe_unused]] constexpr GXFloat GX_MATH_DOUBLE_PI = 6.2831853F;

// 1.0F / 255.0F
[[maybe_unused]] constexpr GXFloat GX_MATH_UNORM_FACTOR = 3.92157e-3F;

//----------------------------------------------------------------------------------------------------------------------

// By convention it is row-vertex.
struct [[maybe_unused]] GXVec2 final
{
    // Stores vector components in x, y order.
    GXFloat     _data[ 2U ];

    [[maybe_unused]] GXVec2 () = default;

    [[maybe_unused]] GXVec2 ( GXVec2 const & ) = default;
    [[maybe_unused]] GXVec2 &operator = ( GXVec2 const & ) = default;

    [[maybe_unused]] GXVec2 ( GXVec2 && ) = default;
    [[maybe_unused]] GXVec2 &operator = ( GXVec2 && ) = default;

    // constexpr constructor is implicitly inline
    // see https://timsong-cpp.github.io/cppwp/n4140/dcl.constexpr
    [[maybe_unused]] constexpr GXVec2 ( GXFloat x, GXFloat y ) noexcept:
        _data { x, y }
    {
        // NOTHING
    }

    [[maybe_unused]] ~GXVec2 () = default;

    [[maybe_unused]] GXVoid SetX ( GXFloat x ) noexcept;
    [[maybe_unused, nodiscard]] GXFloat GetX () const noexcept;

    [[maybe_unused]] GXVoid SetY ( GXFloat y ) noexcept;
    [[maybe_unused, nodiscard]] GXFloat GetY () const noexcept;

    [[maybe_unused]] GXVoid Init ( GXFloat x, GXFloat y ) noexcept;
    [[maybe_unused]] GXVoid Normalize () noexcept;
    [[maybe_unused]] GXVoid Reverse () noexcept;

    // No normalization
    [[maybe_unused]] GXVoid CalculateNormalFast ( GXVec2 const &a, GXVec2 const &b ) noexcept;

    [[maybe_unused]] GXVoid CalculateNormal ( GXVec2 const &a, GXVec2 const &b ) noexcept;

    [[maybe_unused]] GXVoid Sum ( GXVec2 const &a, GXVec2 const &b ) noexcept;
    [[maybe_unused]] GXVoid Sum ( GXVec2 const &a, GXFloat bScale, GXVec2 const &b ) noexcept;
    [[maybe_unused]] GXVoid Subtract ( GXVec2 const &a, GXVec2 const &b ) noexcept;
    [[maybe_unused]] GXVoid Multiply ( GXVec2 const &a, GXVec2 const &b ) noexcept;
    [[maybe_unused]] GXVoid Multiply ( GXVec2 const &v, GXFloat scale ) noexcept;

    [[maybe_unused, nodiscard]] GXFloat DotProduct ( GXVec2 const &other ) const noexcept;
    [[maybe_unused, nodiscard]] GXFloat Length () const noexcept;
    [[maybe_unused, nodiscard]] GXFloat SquaredLength () const noexcept;

    [[maybe_unused, nodiscard]] GXBool IsEqual ( GXVec2 const &other ) const noexcept;
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
) noexcept;

//----------------------------------------------------------------------------------------------------------------------

// By convention it is row-vector.
struct [[maybe_unused]] GXVec3 final
{
    // Stores vector components in x, y, z order.
    GXFloat     _data[ 3U ];

    [[maybe_unused]] GXVec3 () = default;

    [[maybe_unused]] GXVec3 ( GXVec3 const & ) = default;
    [[maybe_unused]] GXVec3 &operator = ( GXVec3 const & ) = default;

    [[maybe_unused]] GXVec3 ( GXVec3 && ) = default;
    [[maybe_unused]] GXVec3 &operator = ( GXVec3 && ) = default;

    // constexpr constructor is implicitly inline
    // see https://timsong-cpp.github.io/cppwp/n4140/dcl.constexpr
    [[maybe_unused]] constexpr GXVec3 ( GXFloat x, GXFloat y, GXFloat z ) noexcept:
        _data { x, y, z }
    {
        // NOTHING
    }

    [[maybe_unused]] ~GXVec3 () = default;

    [[maybe_unused]] GXVoid SetX ( GXFloat x ) noexcept;
    [[maybe_unused, nodiscard]] GXFloat GetX () const noexcept;

    [[maybe_unused]] GXVoid SetY ( GXFloat y ) noexcept;
    [[maybe_unused, nodiscard]] GXFloat GetY () const noexcept;

    [[maybe_unused]] GXVoid SetZ ( GXFloat z ) noexcept;
    [[maybe_unused, nodiscard]] GXFloat GetZ () const noexcept;

    [[maybe_unused]] GXVoid Init ( GXFloat x, GXFloat y, GXFloat z ) noexcept;
    [[maybe_unused]] GXVoid Normalize () noexcept;
    [[maybe_unused]] GXVoid Reverse () noexcept;

    [[maybe_unused]] GXVoid Sum ( GXVec3 const &a, GXVec3 const &b ) noexcept;
    [[maybe_unused]] GXVoid Sum ( GXVec3 const &a, GXFloat bScale, GXVec3 const &b ) noexcept;
    [[maybe_unused]] GXVoid Subtract ( GXVec3 const &a, GXVec3 const &b ) noexcept;
    [[maybe_unused]] GXVoid Multiply ( GXVec3 const &a, GXFloat scale ) noexcept;
    [[maybe_unused]] GXVoid Multiply ( GXVec3 const &a, GXVec3 const &b ) noexcept;

    [[maybe_unused, nodiscard]] GXFloat DotProduct ( GXVec3 const &other ) const noexcept;
    [[maybe_unused]] GXVoid CrossProduct ( GXVec3 const &a, GXVec3 const &b ) noexcept;

    [[maybe_unused, nodiscard]] GXFloat Length () const noexcept;
    [[maybe_unused, nodiscard]] GXFloat SquaredLength () const noexcept;
    [[maybe_unused, nodiscard]] GXFloat Distance ( GXVec3 const &other ) const noexcept;
    [[maybe_unused, nodiscard]] GXFloat SquaredDistance ( GXVec3 const &other ) const noexcept;

    [[maybe_unused]] GXVoid LinearInterpolation ( GXVec3 const &start,
        GXVec3 const &finish,
        GXFloat interpolationFactor
    ) noexcept;

    // Note the axis must be a unit vector.
    [[maybe_unused]] GXVoid Project ( GXVec3 const &vector, GXVec3 const &axis ) noexcept;

    [[maybe_unused, nodiscard]] GXBool IsEqual ( GXVec3 const &other ) noexcept;

    [[maybe_unused, nodiscard]] static GXVec3 const &GetAbsoluteX () noexcept;
    [[maybe_unused, nodiscard]] static GXVec3 const &GetAbsoluteY () noexcept;
    [[maybe_unused, nodiscard]] static GXVec3 const &GetAbsoluteZ () noexcept;

    // baseX - correct direction, adjustedY - desirable, adjustedZ - calculated.
    [[maybe_unused]] static GXVoid GXCALL MakeOrthonormalBasis ( GXVec3 &baseX,
        GXVec3 &adjustedY,
        GXVec3 &adjustedZ
    ) noexcept;
};

//----------------------------------------------------------------------------------------------------------------------

[[maybe_unused]] GXBool GXCALL GXRayTriangleIntersection3D ( GXFloat &outT,
    GXVec3 const &origin,
    GXVec3 const &direction,
    GXFloat length,
    GXVec3 const &a,
    GXVec3 const &b,
    GXVec3 const &c
) noexcept;

//----------------------------------------------------------------------------------------------------------------------

struct [[maybe_unused]] GXEuler final
{
    [[maybe_unused]] GXFloat    _pitchRadians;
    [[maybe_unused]] GXFloat    _yawRadians;
    [[maybe_unused]] GXFloat    _rollRadians;

    [[maybe_unused]] GXEuler () = default;

    [[maybe_unused]] GXEuler ( GXEuler const & ) = default;
    [[maybe_unused]] GXEuler &operator = ( GXEuler const & ) = default;

    [[maybe_unused]] GXEuler ( GXEuler && ) = default;
    [[maybe_unused]] GXEuler &operator = ( GXEuler && ) = default;

    // constexpr constructor is implicitly inline
    // see https://timsong-cpp.github.io/cppwp/n4140/dcl.constexpr
    [[maybe_unused]] constexpr GXEuler ( GXFloat pitchRadians,
        GXFloat yawRadians,
        GXFloat rollRadians
    ) noexcept:
        _pitchRadians ( pitchRadians ),
        _yawRadians ( yawRadians ),
        _rollRadians ( rollRadians )
    {
        // NOTHING
    }

    [[maybe_unused]] ~GXEuler () = default;
};

//----------------------------------------------------------------------------------------------------------------------

// By convention it is row-vector.
struct [[maybe_unused]] GXVec4 final
{
    // Stores vector components in x, y, z, w order.
    GXFloat     _data[ 4U ];

    [[maybe_unused]] GXVec4 () = default;

    [[maybe_unused]] GXVec4 ( GXVec4 const & ) = default;
    [[maybe_unused]] GXVec4 &operator = ( GXVec4 const & ) = default;

    [[maybe_unused]] GXVec4 ( GXVec4 && ) = default;
    [[maybe_unused]] GXVec4 &operator = ( GXVec4 && ) = default;

    [[maybe_unused]] GXVec4 ( GXVec3 const &vector, GXFloat w ) noexcept;

    // constexpr constructor is implicitly inline
    // see https://timsong-cpp.github.io/cppwp/n4140/dcl.constexpr
    [[maybe_unused]] constexpr GXVec4 ( GXFloat x, GXFloat y, GXFloat z, GXFloat w ) noexcept:
        _data { x, y, z, w }
    {
        // NOTHING
    }

    [[maybe_unused]] ~GXVec4 () = default;

    [[maybe_unused]] GXVoid Init ( GXFloat x, GXFloat y, GXFloat z, GXFloat w ) noexcept;

    [[maybe_unused]] GXVoid SetX ( GXFloat x ) noexcept;
    [[maybe_unused, nodiscard]] GXFloat GetX () const noexcept;

    [[maybe_unused]] GXVoid SetY ( GXFloat y ) noexcept;
    [[maybe_unused, nodiscard]] GXFloat GetY () const noexcept;

    [[maybe_unused]] GXVoid SetZ ( GXFloat z ) noexcept;
    [[maybe_unused, nodiscard]] GXFloat GetZ () const noexcept;

    [[maybe_unused]] GXVoid SetW ( GXFloat w ) noexcept;
    [[maybe_unused, nodiscard]] GXFloat GetW () const noexcept;

    [[maybe_unused]] GXVoid Sum ( GXVec4 const &a, GXVec4 const &b ) noexcept;
    [[maybe_unused]] GXVoid Sum ( GXVec4 const &a, GXFloat bScale, GXVec4 const &b ) noexcept;
    [[maybe_unused]] GXVoid Subtract ( GXVec4 const &a, GXVec4 const &b ) noexcept;

    [[maybe_unused, nodiscard]] GXFloat DotProduct ( GXVec4 const &other ) const noexcept;

    [[maybe_unused, nodiscard]] GXFloat Length () const noexcept;
    [[maybe_unused, nodiscard]] GXFloat SquaredLength () const noexcept;
};

//----------------------------------------------------------------------------------------------------------------------

struct [[maybe_unused]] GXVec6 final
{
    GXFloat     _data[ 6U ];

    [[maybe_unused]] GXVec6 () = default;

    [[maybe_unused]] GXVec6 ( GXVec6 const & ) = default;
    [[maybe_unused]] GXVec6 &operator = ( GXVec6 const & ) = default;

    [[maybe_unused]] GXVec6 ( GXVec6 && ) = default;
    [[maybe_unused]] GXVec6 &operator = ( GXVec6 && ) = default;

    // constexpr constructor is implicitly inline
    // see https://timsong-cpp.github.io/cppwp/n4140/dcl.constexpr
    [[maybe_unused]] constexpr GXVec6 ( GXFloat a1,
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

    // constexpr constructor is implicitly inline
    // see https://timsong-cpp.github.io/cppwp/n4140/dcl.constexpr
    [[maybe_unused]] constexpr GXVec6 ( GXVec3 const &part1, GXVec3 const &part2 ) noexcept:
        _data
        {
            part1._data[ 0U ],
            part1._data[ 1U ],
            part1._data[ 2U ],
            part2._data[ 0U ],
            part2._data[ 1U ],
            part2._data[ 2U ]
        }
    {
        // NOTHING
    }

    [[maybe_unused]] ~GXVec6 () = default;

    [[maybe_unused]] GXVoid Init ( GXFloat a1, GXFloat a2, GXFloat a3, GXFloat a4, GXFloat a5, GXFloat a6 ) noexcept;
    [[maybe_unused]] GXVoid From ( GXVec3 const &v1, GXVec3 const &v2 ) noexcept;

    [[maybe_unused, nodiscard]] GXFloat DotProduct ( GXVec6 const &other ) const noexcept;
    [[maybe_unused]] GXVoid Sum ( GXVec6 const &a, GXVec6 const &b ) noexcept;
    [[maybe_unused]] GXVoid Sum ( GXVec6 const &a, GXFloat bScale, GXVec6 const &b ) noexcept;
    [[maybe_unused]] GXVoid Multiply ( GXVec6 const &a, GXFloat factor ) noexcept;
};

//----------------------------------------------------------------------------------------------------------------------

struct [[maybe_unused]] GXColorUNORM final
{
    // Stores components in red, green, blue, alpha order.
    GXUByte     _data[ 4U ];

    [[maybe_unused]] GXColorUNORM () = default;

    [[maybe_unused]] GXColorUNORM ( GXColorUNORM const & ) = default;
    [[maybe_unused]] GXColorUNORM &operator = ( GXColorUNORM const & ) = default;

    [[maybe_unused]] GXColorUNORM ( GXColorUNORM && ) = default;
    [[maybe_unused]] GXColorUNORM &operator = ( GXColorUNORM && ) = default;

    [[maybe_unused]] constexpr GXColorUNORM ( GXUInt red, GXUInt green, GXUInt blue, GXUInt alpha ) noexcept:
        _data
        {
            static_cast<GXUByte> ( red ),
            static_cast<GXUByte> ( green ),
            static_cast<GXUByte> ( blue ),
            static_cast<GXUByte> ( alpha )
        }
    {
        // NOTHING
    }

    [[maybe_unused]] constexpr GXColorUNORM ( GXUByte red, GXUByte green, GXUByte blue, GXUByte alpha ) noexcept:
        _data { red, green, blue, alpha }
    {
        // NOTHING
    }

    [[maybe_unused]] ~GXColorUNORM () = default;
};

//----------------------------------------------------------------------------------------------------------------------

struct GXColorHSV;
struct [[maybe_unused]] GXColorRGB final
{
    // Stores components in red, green, blue, alpha order.
    GXFloat     _data[ 4U ];

    [[maybe_unused]] GXColorRGB () = default;

    [[maybe_unused]] GXColorRGB ( GXColorRGB const & ) = default;
    [[maybe_unused]] GXColorRGB &operator = ( GXColorRGB const & ) = default;

    [[maybe_unused]] GXColorRGB ( GXColorRGB && ) = default;
    [[maybe_unused]] GXColorRGB &operator = ( GXColorRGB && ) = default;

    // constexpr constructor is implicitly inline
    // see https://timsong-cpp.github.io/cppwp/n4140/dcl.constexpr
    [[maybe_unused]] constexpr GXColorRGB ( GXFloat red, GXFloat green, GXFloat blue, GXFloat alpha ) noexcept:
        _data { red, green, blue, alpha }
    {
        // NOTHING
    }

    [[maybe_unused]] constexpr GXColorRGB ( GXUInt red, GXUInt green, GXUInt blue, GXFloat alpha ) noexcept:
        _data
        {
            GX_MATH_UNORM_FACTOR * static_cast<GXFloat> ( red ),
            GX_MATH_UNORM_FACTOR * static_cast<GXFloat> ( green ),
            GX_MATH_UNORM_FACTOR * static_cast<GXFloat> ( blue ),
            alpha
        }
    {
        // NOTHING
    }

    [[maybe_unused]] GXColorRGB ( GXUByte red, GXUByte green, GXUByte blue, GXFloat alpha ) noexcept;
    [[maybe_unused]] explicit GXColorRGB ( GXColorHSV const &color ) noexcept;
    [[maybe_unused]] explicit GXColorRGB ( GXColorUNORM color ) noexcept;

    [[maybe_unused]] ~GXColorRGB () = default;

    [[maybe_unused]] GXVoid Init ( GXFloat red, GXFloat green, GXFloat blue, GXFloat alpha ) noexcept;

    // [0.0F +inf)
    [[maybe_unused]] GXVoid SetRed ( GXFloat red ) noexcept;
    [[maybe_unused, nodiscard]] GXFloat GetRed () const noexcept;

    // [0.0F +inf)
    [[maybe_unused]] GXVoid SetGreen ( GXFloat green ) noexcept;
    [[maybe_unused, nodiscard]] GXFloat GetGreen () const noexcept;

    // [0.0F +inf)
    [[maybe_unused]] GXVoid SetBlue ( GXFloat blue ) noexcept;
    [[maybe_unused, nodiscard]] GXFloat GetBlue () const noexcept;

    // [0.0f 1.0F]
    [[maybe_unused]] GXVoid SetAlpha ( GXFloat alpha ) noexcept;
    [[maybe_unused, nodiscard]] GXFloat GetAlpha () const noexcept;

    [[maybe_unused]] GXVoid From ( GXUByte red, GXUByte green, GXUByte blue, GXFloat alpha ) noexcept;
    [[maybe_unused]] GXVoid From ( GXUInt red, GXUInt green, GXUInt blue, GXFloat alpha ) noexcept;
    [[maybe_unused]] GXVoid From ( GXColorHSV const &color ) noexcept;

    // It is assumed that current color space is sRGB.
    [[maybe_unused, nodiscard]] GXColorRGB ToLinearSpace () const noexcept;

    [[maybe_unused, nodiscard]] GXColorUNORM ToColorUNORM () const noexcept;

    [[maybe_unused]] GXVoid ConvertToUByte ( GXUByte &red,
        GXUByte &green,
        GXUByte &blue,
        GXUByte &alpha
    ) const noexcept;
};

//----------------------------------------------------------------------------------------------------------------------

struct [[maybe_unused]] GXColorHSV final
{
    // Stores components in hue, saturation, value, alpha order.
    GXFloat     _data[ 4U ];

    [[maybe_unused]] GXColorHSV () = default;

    [[maybe_unused]] GXColorHSV ( GXColorHSV const & ) = default;
    [[maybe_unused]] GXColorHSV &operator = ( GXColorHSV const & ) = default;

    [[maybe_unused]] GXColorHSV ( GXColorHSV && ) = default;
    [[maybe_unused]] GXColorHSV &operator = ( GXColorHSV && ) = default;

    // constexpr constructor is implicitly inline
    // see https://timsong-cpp.github.io/cppwp/n4140/dcl.constexpr
    [[maybe_unused]] constexpr GXColorHSV ( GXFloat hue,
        GXFloat saturation,
        GXFloat value,
        GXFloat alpha
    ) noexcept:
        _data { hue, saturation, value, alpha }
    {
        // NOTHING
    }

    [[maybe_unused]] explicit GXColorHSV ( GXColorRGB const &color ) noexcept;

    [[maybe_unused]] ~GXColorHSV () = default;

    // [0.0F 360.0F]
    [[maybe_unused]] GXVoid SetHue ( GXFloat hue ) noexcept;
    [[maybe_unused, nodiscard]] GXFloat GetHue () const noexcept;

    // [0.0F 100.0F]
    [[maybe_unused]] GXVoid SetSaturation ( GXFloat saturation ) noexcept;
    [[maybe_unused, nodiscard]] GXFloat GetSaturation () const noexcept;

    // [0.0F 100.0F]
    [[maybe_unused]] GXVoid SetValue ( GXFloat value ) noexcept;
    [[maybe_unused, nodiscard]] GXFloat GetValue () const noexcept;

    // [0.0F 100.0F]
    [[maybe_unused]] GXVoid SetAlpha ( GXFloat alpha ) noexcept;
    [[maybe_unused, nodiscard]] GXFloat GetAlpha () const noexcept;

    [[maybe_unused]] GXVoid From ( GXColorRGB const &color ) noexcept;
};

//----------------------------------------------------------------------------------------------------------------------

struct [[maybe_unused]] GXPreciseComplex final
{
    GXDouble    _r;
    GXDouble    _i;

    [[maybe_unused]] GXPreciseComplex () = default;

    [[maybe_unused]] GXPreciseComplex ( GXPreciseComplex const &other ) = default;
    [[maybe_unused]] GXPreciseComplex &operator = ( GXPreciseComplex const &other ) = default;

    [[maybe_unused]] GXPreciseComplex ( GXPreciseComplex && ) = default;
    [[maybe_unused]] GXPreciseComplex &operator = ( GXPreciseComplex && ) = default;

    // constexpr constructor is implicitly inline
    // see https://timsong-cpp.github.io/cppwp/n4140/dcl.constexpr
    [[maybe_unused]] constexpr GXPreciseComplex ( GXDouble real, GXDouble imaginary ) noexcept:
        _r ( real ),
        _i ( imaginary )
    {
        // NOTHING
    }

    [[maybe_unused]] ~GXPreciseComplex () = default;

    [[maybe_unused]] GXVoid Init ( GXDouble real, GXDouble imaginary ) noexcept;

    [[maybe_unused, nodiscard]] GXDouble Length () const noexcept;
    [[maybe_unused, nodiscard]] GXDouble SquaredLength () const noexcept;

    // Method returns GX_FALSE if ( 0.0 + 0.0i ) ^ 0 will happen.
    [[maybe_unused]] GXBool Power ( GXUInt power ) noexcept;

    [[maybe_unused]] GXPreciseComplex operator + ( GXPreciseComplex const &other ) const noexcept;
    [[maybe_unused]] GXPreciseComplex operator - ( GXPreciseComplex const &other ) const noexcept;
    [[maybe_unused]] GXPreciseComplex operator * ( GXPreciseComplex const &other ) const noexcept;
    [[maybe_unused]] GXPreciseComplex operator * ( GXDouble a ) const noexcept;
    [[maybe_unused]] GXPreciseComplex operator / ( GXDouble a ) const noexcept;
};

//----------------------------------------------------------------------------------------------------------------------

struct GXMat3;
struct GXMat4;

// Quaternion representation: r + ai + bj + ck.
// By convention stores only orientation without any scale.
struct [[maybe_unused]] GXQuat final
{
    // Stores quaternion components in r, a, b, c order.
    GXFloat     _data[ 4U ];

    [[maybe_unused]] GXQuat () = default;

    [[maybe_unused]] GXQuat ( GXQuat const & ) = default;
    [[maybe_unused]] GXQuat &operator = ( GXQuat const & ) = default;

    [[maybe_unused]] GXQuat ( GXQuat && ) = default;
    [[maybe_unused]] GXQuat &operator = ( GXQuat && ) = default;

    // constexpr constructor is implicitly inline
    // see https://timsong-cpp.github.io/cppwp/n4140/dcl.constexpr
    [[maybe_unused]] constexpr GXQuat ( GXFloat r, GXFloat a, GXFloat b, GXFloat c ) noexcept:
        _data { r, a, b, c }
    {
        // NOTHING
    }

    // Result is valid if rotationMatrix is rotation matrix. Any scale will be ignored.
    [[maybe_unused]] explicit GXQuat ( GXMat3 const &rotationMatrix ) noexcept;

    // Result is valid if rotationMatrix is rotation matrix. Any scale will be ignored.
    [[maybe_unused]] explicit GXQuat ( GXMat4 const &rotationMatrix ) noexcept;

    [[maybe_unused]] ~GXQuat () = default;

    // Packing TBN basis into A2R10G10B10_UNORM format. "Real" component could be restored using unit quaternion
    // property. It's guarantee to be positive real component eliminating quaternion duality flaw.
    // 'w' component will contain information about bitangent reflection, the scalar: -1.0 or 1.0.
    // bits 0-9: a component
    // bits 10-19: b component
    // bits 20-29: c component
    // bits 30-31: bitangent reflection scalar
    [[maybe_unused, nodiscard]] GXUInt Compress ( bool reflectBitangent ) const noexcept;

    [[maybe_unused]] GXVoid Init ( GXFloat r, GXFloat a, GXFloat b, GXFloat c ) noexcept;

    [[maybe_unused]] GXVoid SetR ( GXFloat r ) noexcept;
    [[maybe_unused, nodiscard]] GXFloat GetR () const noexcept;

    [[maybe_unused]] GXVoid SetA ( GXFloat a ) noexcept;
    [[maybe_unused, nodiscard]] GXFloat GetA () const noexcept;

    [[maybe_unused]] GXVoid SetB ( GXFloat b ) noexcept;
    [[maybe_unused, nodiscard]] GXFloat GetB () const noexcept;

    [[maybe_unused]] GXVoid SetC ( GXFloat c ) noexcept;
    [[maybe_unused, nodiscard]] GXFloat GetC () const noexcept;

    [[maybe_unused]] GXVoid Identity () noexcept;
    [[maybe_unused]] GXVoid Normalize () noexcept;
    [[maybe_unused]] GXVoid Inverse ( GXQuat const &q ) noexcept;

    // Result is valid if "unitQuaternion" is normalized.
    [[maybe_unused]] GXVoid InverseFast ( GXQuat const &unitQuaternion ) noexcept;

    [[maybe_unused]] GXVoid FromAxisAngle ( GXFloat x, GXFloat y, GXFloat z, GXFloat angle ) noexcept;
    [[maybe_unused]] GXVoid FromAxisAngle ( GXVec3 const &axis, GXFloat angle ) noexcept;

    // Result is valid if rotationMatrix is rotation matrix. Any scale will be ignored.
    [[maybe_unused]] GXVoid From ( GXMat3 const &rotationMatrix ) noexcept;

    // Result is valid if rotationMatrix is rotation matrix. Any scale will be ignored.
    [[maybe_unused]] GXVoid From ( GXMat4 const &rotationMatrix ) noexcept;

    // Result is valid if pureRotationMatrix is not scaled rotation matrix.
    [[maybe_unused]] GXVoid FromFast ( GXMat3 const &pureRotationMatrix ) noexcept;

    // Result is valid if pureRotationMatrix is not scaled rotation matrix.
    [[maybe_unused]] GXVoid FromFast ( GXMat4 const &pureRotationMatrix ) noexcept;

    [[maybe_unused]] GXVoid Multiply ( GXQuat const &a, GXQuat const &b ) noexcept;
    [[maybe_unused]] GXVoid Multiply ( GXQuat const &q, GXFloat scale ) noexcept;
    [[maybe_unused]] GXVoid Sum ( GXQuat const &a, GXQuat const &b ) noexcept;
    [[maybe_unused]] GXVoid Subtract ( GXQuat const &a, GXQuat const &b ) noexcept;

    [[maybe_unused]] GXVoid SphericalLinearInterpolation ( GXQuat const &start,
        GXQuat const &finish,
        GXFloat interpolationFactor
    ) noexcept;

    [[maybe_unused]] GXVoid GetAxisAngle ( GXVec3 &axis, GXFloat &angle ) const noexcept;
    [[maybe_unused]] GXVoid Transform ( GXVec3 &out, GXVec3 const &v ) const noexcept;

    // Result is valid if quaternion is normalized.
    [[maybe_unused]] GXVoid TransformFast ( GXVec3 &out, GXVec3 const &v ) const noexcept;
};

//----------------------------------------------------------------------------------------------------------------------

struct [[maybe_unused]] GXMat3 final
{
    union
    {
        GXFloat     _data[ 9U ];
        GXFloat     _m[ 3U ][ 3U ];
    };

    [[maybe_unused]] GXMat3 () = default;

    [[maybe_unused]] GXMat3 ( GXMat3 const & ) = default;
    [[maybe_unused]] GXMat3 &operator = ( GXMat3 const & ) = default;

    [[maybe_unused]] GXMat3 ( GXMat3 && ) = default;
    [[maybe_unused]] GXMat3 &operator = ( GXMat3 && ) = default;

    [[maybe_unused]] explicit GXMat3 ( GXMat4 const &matrix ) noexcept;

    [[maybe_unused]] ~GXMat3 () = default;

    [[maybe_unused]] GXVoid From ( GXQuat const &quaternion ) noexcept;
    [[maybe_unused]] GXVoid From ( GXMat4 const &matrix ) noexcept;

    // Constructs orthonormal basis. Result is valid if zDirection is unit vector.
    [[maybe_unused]] GXVoid From ( GXVec3 const &zDirection ) noexcept;

    // Result is valid if quaternion is normalized.
    [[maybe_unused]] GXVoid FromFast ( GXQuat const &quaternion ) noexcept;

    [[maybe_unused]] GXVoid SetX ( GXVec3 const &x ) noexcept;
    [[maybe_unused]] GXVoid GetX ( GXVec3 &x ) const noexcept;

    [[maybe_unused]] GXVoid SetY ( GXVec3 const &y ) noexcept;
    [[maybe_unused]] GXVoid GetY ( GXVec3 &y ) const noexcept;

    [[maybe_unused]] GXVoid SetZ ( GXVec3 const &z ) noexcept;
    [[maybe_unused]] GXVoid GetZ ( GXVec3 &z ) const noexcept;

    [[maybe_unused]] GXVoid Identity () noexcept;
    [[maybe_unused]] GXVoid Zeros () noexcept;

    [[maybe_unused]] GXVoid Inverse ( GXMat3 const &sourceMatrix ) noexcept;
    [[maybe_unused]] GXVoid Transpose ( GXMat3 const &sourceMatrix ) noexcept;
    [[maybe_unused]] GXVoid ClearRotation ( GXMat3 const &sourceMatrix ) noexcept;
    [[maybe_unused]] GXVoid ClearRotation ( GXMat4 const &sourceMatrix ) noexcept;

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
    [[maybe_unused]] GXVoid SkewSymmetric ( GXVec3 const &base ) noexcept;

    [[maybe_unused]] GXVoid Sum ( GXMat3 const &a, GXMat3 const &b ) noexcept;
    [[maybe_unused]] GXVoid Subtract ( GXMat3 const &a, GXMat3 const &b ) noexcept;
    [[maybe_unused]] GXVoid Multiply ( GXMat3 const &a, GXMat3 const &b ) noexcept;

    [[maybe_unused]] GXVoid MultiplyVectorMatrix ( GXVec3 &out, GXVec3 const &v ) const noexcept;
    [[maybe_unused]] GXVoid MultiplyMatrixVector ( GXVec3 &out, GXVec3 const &v ) const noexcept;

    [[maybe_unused]] GXVoid Multiply ( GXMat3 const &a, GXFloat factor ) noexcept;
};

//----------------------------------------------------------------------------------------------------------------------

struct [[maybe_unused]] GXMat4 final
{
    union
    {
        GXFloat     _data[ 16u ];
        GXFloat     _m[ 4u ][ 4u ];
    };

    [[maybe_unused]] GXMat4 () = default;

    [[maybe_unused]] GXMat4 ( GXMat4 const & ) = default;
    [[maybe_unused]] GXMat4 &operator = ( GXMat4 const & ) = default;

    [[maybe_unused]] GXMat4 ( GXMat4 && ) = default;
    [[maybe_unused]] GXMat4 &operator = ( GXMat4 && ) = default;

    [[maybe_unused]] ~GXMat4 () = default;

    [[maybe_unused]] GXVoid SetRotation ( GXQuat const &quaternion ) noexcept;

    // Result is valid if quaternion is normalized.
    [[maybe_unused]] GXVoid SetRotationFast ( GXQuat const &quaternion ) noexcept;

    [[maybe_unused]] GXVoid SetOrigin ( GXVec3 const &origin ) noexcept;
    [[maybe_unused]] GXVoid From ( GXQuat const &quaternion, GXVec3 const &origin ) noexcept;
    [[maybe_unused]] GXVoid From ( GXMat3 const &rotation, GXVec3 const &origin ) noexcept;
    [[maybe_unused]] GXVoid From ( GXVec3 const &zDirection, GXVec3 const &origin ) noexcept;

    // Result is valid if quaternion is normalized.
    [[maybe_unused]] GXVoid FromFast ( GXQuat const &quaternion, GXVec3 const &origin ) noexcept;

    [[maybe_unused]] GXVoid SetX ( GXVec3 const &x ) noexcept;
    [[maybe_unused]] GXVoid GetX ( GXVec3 &x ) const noexcept;

    [[maybe_unused]] GXVoid SetY ( GXVec3 const &y ) noexcept;
    [[maybe_unused]] GXVoid GetY ( GXVec3 &y ) const noexcept;

    [[maybe_unused]] GXVoid SetZ ( GXVec3 const &z ) noexcept;
    [[maybe_unused]] GXVoid GetZ ( GXVec3 &z ) const noexcept;

    [[maybe_unused]] GXVoid SetW ( GXVec3 const &w ) noexcept;
    [[maybe_unused]] GXVoid GetW ( GXVec3 &w ) const noexcept;

    [[maybe_unused]] GXVoid Identity () noexcept;

    [[maybe_unused]] GXVoid Perspective ( GXFloat fieldOfViewYRadians,
        GXFloat aspectRatio,
        GXFloat zNear,
        GXFloat zFar
    ) noexcept;

    [[maybe_unused]] GXVoid Ortho ( GXFloat width, GXFloat height, GXFloat zNear, GXFloat zFar ) noexcept;

    [[maybe_unused]] GXVoid Translation ( GXFloat x, GXFloat y, GXFloat z ) noexcept;
    [[maybe_unused]] GXVoid Translation ( GXVec3 const &location ) noexcept;

    [[maybe_unused]] GXVoid TranslateTo ( GXFloat x, GXFloat y, GXFloat z ) noexcept;
    [[maybe_unused]] GXVoid TranslateTo ( GXVec3 const &location ) noexcept;

    [[maybe_unused]] GXVoid RotationX ( GXFloat angle ) noexcept;
    [[maybe_unused]] GXVoid RotationY ( GXFloat angle ) noexcept;
    [[maybe_unused]] GXVoid RotationZ ( GXFloat angle ) noexcept;
    [[maybe_unused]] GXVoid RotationXY ( GXFloat pitchRadians, GXFloat yawRadians ) noexcept;
    [[maybe_unused]] GXVoid RotationXYZ ( GXFloat pitchRadians, GXFloat yawRadians, GXFloat rollRadians ) noexcept;
    [[maybe_unused]] GXVoid ClearRotation ( GXMat3 const &sourceMatrix ) noexcept;
    [[maybe_unused]] GXVoid ClearRotation ( GXMat4 const &sourceMatrix ) noexcept;

    [[maybe_unused]] GXVoid Scale ( GXFloat x, GXFloat y, GXFloat z ) noexcept;
    [[maybe_unused]] GXVoid ClearScale ( GXVec3 &scale ) const noexcept;

    [[maybe_unused]] GXVoid Inverse ( GXMat4 const &sourceMatrix ) noexcept;

    [[maybe_unused]] GXVoid Multiply ( GXMat4 const &a, GXMat4 const &b ) noexcept;

    // Multiply row-vector [1x4] by own matrix [4x4].
    [[maybe_unused]] GXVoid MultiplyVectorMatrix ( GXVec4 &out, GXVec4 const &v ) const noexcept;

    // Multiply own matrix [4x4] by column-vector [4x1].
    [[maybe_unused]] GXVoid MultiplyMatrixVector ( GXVec4 &out, GXVec4 const &v ) const noexcept;

    // Multiply row-vector [1x3] by own matrix sub matrix [3x3].
    [[maybe_unused]] GXVoid MultiplyAsNormal ( GXVec3 &out, GXVec3 const &v ) const noexcept;

    // Multiply row-vector [1x3] by own matrix sub matrix [3x3] and add own w-vector.
    [[maybe_unused]] GXVoid MultiplyAsPoint ( GXVec3 &out, GXVec3 const &v ) const noexcept;

    // Result is valid if own matrix is perspective matrix.
    [[maybe_unused]] GXVoid GetPerspectiveParams ( GXFloat &fieldOfViewYRadians,
        GXFloat &aspectRatio,
        GXFloat &zNear,
        GXFloat &zFar
    ) noexcept;

    // Result is valid if own matrix is ortho matrix.
    [[maybe_unused]] GXVoid GetOrthoParams ( GXFloat &width, GXFloat &height, GXFloat &zNear, GXFloat &zFar ) noexcept;

    // Result is valid if own matrix is perspective matrix.
    [[maybe_unused]] GXVoid GetRayPerspective ( GXVec3 &rayView, GXVec2 const &mouseCVV ) const noexcept;
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
    [[maybe_unused]] GXAABB &operator = ( GXAABB const & ) = default;

    [[maybe_unused]] GXAABB ( GXAABB && ) = default;
    [[maybe_unused]] GXAABB &operator = ( GXAABB && ) = default;

    [[maybe_unused]] ~GXAABB () = default;

    [[maybe_unused]] GXVoid Empty () noexcept;

    [[maybe_unused]] GXVoid Transform ( GXAABB &bounds, GXMat4 const &transform ) const noexcept;
    [[maybe_unused]] GXVoid AddVertex ( GXVec3 const &vertex ) noexcept;
    [[maybe_unused]] GXVoid AddVertex ( GXFloat x, GXFloat y, GXFloat z ) noexcept;

    [[maybe_unused, nodiscard]] GXBool IsOverlapped ( GXAABB const &other ) const noexcept;
    [[maybe_unused, nodiscard]] GXBool IsOverlapped ( GXVec3 const &point ) const noexcept;
    [[maybe_unused, nodiscard]] GXBool IsOverlapped ( GXFloat x, GXFloat y, GXFloat z ) const noexcept;

    [[maybe_unused]] GXVoid GetCenter ( GXVec3 &center ) const noexcept;
    [[maybe_unused, nodiscard]] GXFloat GetWidth () const noexcept;
    [[maybe_unused, nodiscard]] GXFloat GetHeight () const noexcept;
    [[maybe_unused, nodiscard]] GXFloat GetDepth () const noexcept;
    [[maybe_unused, nodiscard]] GXFloat GetSphereRadius () const noexcept;
};

//----------------------------------------------------------------------------------------------------------------------

enum class eGXPlaneClassifyVertex : GXUByte
{
    InFront = 0U,
    On = 1U,
    Behind = 2U
};

struct [[maybe_unused]] GXPlane final
{
    GXFloat     _a;
    GXFloat     _b;
    GXFloat     _c;
    GXFloat     _d;

    [[maybe_unused]] GXPlane () = default;

    [[maybe_unused]] GXPlane ( GXPlane const & ) = default;
    [[maybe_unused]] GXPlane &operator = ( GXPlane const & ) = default;

    [[maybe_unused]] GXPlane ( GXPlane && ) = default;
    [[maybe_unused]] GXPlane &operator = ( GXPlane && ) = default;

    [[maybe_unused]] ~GXPlane () = default;

    [[maybe_unused]] GXVoid From ( GXVec3 const &pointA, GXVec3 const &pointB, GXVec3 const &pointC ) noexcept;

    [[maybe_unused]] GXVoid FromLineToPoint ( GXVec3 const &lineStart,
        GXVec3 const &lineEnd,
        GXVec3 const &point
    ) noexcept;

    [[maybe_unused]] GXVoid Normalize () noexcept;
    [[maybe_unused]] GXVoid Flip () noexcept;

    [[maybe_unused, nodiscard]] eGXPlaneClassifyVertex ClassifyVertex ( GXVec3 const &vertex ) const noexcept;

    [[maybe_unused, nodiscard]] eGXPlaneClassifyVertex ClassifyVertex ( GXFloat x,
        GXFloat y,
        GXFloat z
    ) const noexcept;
};

//----------------------------------------------------------------------------------------------------------------------

class [[maybe_unused]] GXProjectionClipPlanes final
{
    private:
        GXPlane     _planes[ 6U ];

    public:
        [[maybe_unused]] GXProjectionClipPlanes () = default;

        [[maybe_unused]] GXProjectionClipPlanes ( GXProjectionClipPlanes const & ) = default;
        [[maybe_unused]] GXProjectionClipPlanes &operator = ( GXProjectionClipPlanes const & ) = default;

        [[maybe_unused]] GXProjectionClipPlanes ( GXProjectionClipPlanes && ) = default;
        [[maybe_unused]] GXProjectionClipPlanes &operator = ( GXProjectionClipPlanes && ) = default;

        [[maybe_unused]] explicit GXProjectionClipPlanes ( GXMat4 const &src ) noexcept;

        [[maybe_unused]] ~GXProjectionClipPlanes () = default;

        // Normals will be directed inside view volume.
        [[maybe_unused]] GXVoid From ( GXMat4 const &src ) noexcept;

        // Trivial invisibility test.
        [[maybe_unused, nodiscard]] GXBool IsVisible ( GXAABB const &bounds ) const noexcept;

    private:
        [[nodiscard]] GXUByte PlaneTest ( GXFloat x, GXFloat y, GXFloat z ) const noexcept;
};

//---------------------------------------------------------------------------------------------------------------------

[[maybe_unused, nodiscard]] GXFloat GXCALL GXDegToRad ( GXFloat degrees ) noexcept;
[[maybe_unused, nodiscard]] GXFloat GXCALL GXRadToDeg ( GXFloat radians ) noexcept;

[[maybe_unused]] GXVoid GXCALL GXRandomize () noexcept;
[[maybe_unused, nodiscard]] GXFloat GXCALL GXRandomNormalize () noexcept;
[[maybe_unused, nodiscard]] GXFloat GXCALL GXRandomBetween ( GXFloat from, GXFloat to ) noexcept;
[[maybe_unused]] GXVoid GXCALL GXRandomBetween ( GXVec3 &out, GXVec3 const &from, GXVec3 const &to ) noexcept;

[[maybe_unused]] GXVoid GXCALL GXGetTangentBitangent ( GXVec3 &outTangent,
    GXVec3 &outBitangent,
    GXUByte vertexID,
    GXUByte const* vertices,
    GXUPointer vertexStride,
    GXUByte const* uvs,
    GXUPointer uvStride
) noexcept;

[[maybe_unused, nodiscard]] GXFloat GXCALL GXClampf ( GXFloat value, GXFloat minValue, GXFloat maxValue ) noexcept;
[[maybe_unused, nodiscard]] GXInt GXCALL GXClampi ( GXInt value, GXInt minValue, GXInt maxValue ) noexcept;

[[maybe_unused, nodiscard]] GXFloat GXCALL GXMinf ( GXFloat a, GXFloat b ) noexcept;
[[maybe_unused, nodiscard]] GXFloat GXCALL GXMaxf ( GXFloat a, GXFloat b ) noexcept;

[[maybe_unused]] GXVoid GXCALL GXGetBarycentricCoords ( GXVec3 &out,
    GXVec3 const &point,
    GXVec3 const &aPivot,
    GXVec3 const &bPivot,
    GXVec3 const &cPivot
) noexcept;

[[maybe_unused]] GXVoid GXCALL GXGetRayFromViewer ( GXVec3 &origin,
    GXVec3 &direction,
    GXUShort x,
    GXUShort y,
    GXUShort viewportWidth,
    GXUShort viewportHeight,
    GXVec3 const &viewerLocation,
    GXMat4 const &viewProjectionMatrix
) noexcept;


#endif // GX_MATH_HPP
