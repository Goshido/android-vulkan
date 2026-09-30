// version 1.111

#ifndef GX_MATH_HPP
#define GX_MATH_HPP

#include "GXWarning.hpp"

GX_DISABLE_COMMON_WARNINGS

#include <cfloat>
#include <climits>
#include <cmath>

GX_RESTORE_WARNING_STATE


[[maybe_unused]] constexpr float GX_MATH_FLOAT_EPSILON = 1.0e-4F;

[[maybe_unused]] constexpr float GX_MATH_HALF_PI = 1.57079633F;
[[maybe_unused]] constexpr float GX_MATH_PI = 3.14159265F;
[[maybe_unused]] constexpr float GX_MATH_DOUBLE_PI = 6.28318531F;

// 1.0F / 255.0F
[[maybe_unused]] constexpr float GX_MATH_UNORM_FACTOR = 3.92156863e-3F;

//----------------------------------------------------------------------------------------------------------------------

enum class eGXCompareResult : int8_t
{
    Less [[maybe_unused]] = INT8_C ( -1 ),
    Equal [[maybe_unused]] = INT8_C ( 0 ),
    Greater [[maybe_unused]] = INT8_C ( 1 )
};

//----------------------------------------------------------------------------------------------------------------------

// By convention it is row-vertex.
struct [[maybe_unused]] GXVec2 final
{
    // [0.0F, 0.0F]
    [[maybe_unused]] static GXVec2 const    ZERO;

    // Stores vector components in x, y order.
    float                                   _data[ 2U ];

    [[maybe_unused]] GXVec2 () = default;

    [[maybe_unused]] GXVec2 ( GXVec2 const & ) = default;
    [[maybe_unused]] GXVec2 &operator = ( GXVec2 const & ) = default;

    [[maybe_unused]] GXVec2 ( GXVec2 && ) = default;
    [[maybe_unused]] GXVec2 &operator = ( GXVec2 && ) = default;

    // constexpr constructor is implicitly inline
    // see https://timsong-cpp.github.io/cppwp/n4140/dcl.constexpr
    [[maybe_unused]] constexpr GXVec2 ( float x, float y ) noexcept:
        _data { x, y }
    {
        // NOTHING
    }

    [[maybe_unused]] ~GXVec2 () = default;

    [[maybe_unused]] void SetX ( float x ) noexcept;
    [[maybe_unused, nodiscard]] float GetX () const noexcept;

    [[maybe_unused]] void SetY ( float y ) noexcept;
    [[maybe_unused, nodiscard]] float GetY () const noexcept;

    [[maybe_unused]] void Init ( float x, float y ) noexcept;
    [[maybe_unused]] void Normalize () noexcept;
    [[maybe_unused]] void Reverse () noexcept;

    // No normalization
    [[maybe_unused]] void CalculateNormalFast ( GXVec2 const &a, GXVec2 const &b ) noexcept;

    [[maybe_unused]] void CalculateNormal ( GXVec2 const &a, GXVec2 const &b ) noexcept;

    [[maybe_unused]] void Sum ( GXVec2 const &a, GXVec2 const &b ) noexcept;
    [[maybe_unused]] void Sum ( GXVec2 const &a, float bScale, GXVec2 const &b ) noexcept;
    [[maybe_unused]] void Subtract ( GXVec2 const &a, GXVec2 const &b ) noexcept;
    [[maybe_unused]] void Multiply ( GXVec2 const &a, GXVec2 const &b ) noexcept;
    [[maybe_unused]] void Multiply ( GXVec2 const &v, float scale ) noexcept;

    [[maybe_unused, nodiscard]] float DotProduct ( GXVec2 const &other ) const noexcept;
    [[maybe_unused, nodiscard]] float Length () const noexcept;
    [[maybe_unused, nodiscard]] float SquaredLength () const noexcept;

    [[maybe_unused, nodiscard]] bool IsEqual ( GXVec2 const &other ) const noexcept;
};

//----------------------------------------------------------------------------------------------------------------------

enum class eGXLineRelationship : uint8_t
{
    NoIntersection [[maybe_unused]] = UINT8_C ( 0 ),
    Intersection [[maybe_unused]] = UINT8_C ( 1 ),
    Overlap [[maybe_unused]] = UINT8_C ( 2 )
};

[[maybe_unused]] eGXLineRelationship GXLineIntersection2D ( GXVec2 &intersectionPoint,
    GXVec2 const &a0,
    GXVec2 const &a1,
    GXVec2 const &b0,
    GXVec2 const &b1
) noexcept;

//----------------------------------------------------------------------------------------------------------------------

// By convention it is row-vector.
struct [[maybe_unused]] GXVec3 final
{
    // [1.0F, 1.0F, 1.0F]
    [[maybe_unused]] static GXVec3 const    ONE;

    // [0.0F, 0.0F, 0.0F]
    [[maybe_unused]] static GXVec3 const    ZERO;

    // [1.0F, 0.0F, 0.0F]
    [[maybe_unused]] static GXVec3 const    RIGHT;

    // [0.0F, 1.0F, 0.0F]
    [[maybe_unused]] static GXVec3 const    UP;

    // [0.0F, 0.0F, 1.0F]
    [[maybe_unused]] static GXVec3 const    FORWARD;

    // Stores vector components in x, y, z order.
    float                                   _data[ 3U ];

    [[maybe_unused]] GXVec3 () = default;

    [[maybe_unused]] GXVec3 ( GXVec3 const & ) = default;
    [[maybe_unused]] GXVec3 &operator = ( GXVec3 const & ) = default;

    [[maybe_unused]] GXVec3 ( GXVec3 && ) = default;
    [[maybe_unused]] GXVec3 &operator = ( GXVec3 && ) = default;

    // constexpr constructor is implicitly inline
    // see https://timsong-cpp.github.io/cppwp/n4140/dcl.constexpr
    [[maybe_unused]] constexpr GXVec3 ( float x, float y, float z ) noexcept:
        _data { x, y, z }
    {
        // NOTHING
    }

    [[maybe_unused]] ~GXVec3 () = default;

    [[maybe_unused]] void SetX ( float x ) noexcept;
    [[maybe_unused, nodiscard]] float GetX () const noexcept;

    [[maybe_unused]] void SetY ( float y ) noexcept;
    [[maybe_unused, nodiscard]] float GetY () const noexcept;

    [[maybe_unused]] void SetZ ( float z ) noexcept;
    [[maybe_unused, nodiscard]] float GetZ () const noexcept;

    [[maybe_unused]] void Init ( float x, float y, float z ) noexcept;
    [[maybe_unused]] void Normalize () noexcept;
    [[maybe_unused]] void Reverse () noexcept;

    [[maybe_unused]] void Sum ( GXVec3 const &a, GXVec3 const &b ) noexcept;
    [[maybe_unused]] void Sum ( GXVec3 const &a, float bScale, GXVec3 const &b ) noexcept;
    [[maybe_unused]] void Subtract ( GXVec3 const &a, GXVec3 const &b ) noexcept;
    [[maybe_unused]] void Multiply ( GXVec3 const &a, float scale ) noexcept;
    [[maybe_unused]] void Multiply ( GXVec3 const &a, GXVec3 const &b ) noexcept;

    [[maybe_unused, nodiscard]] float DotProduct ( GXVec3 const &other ) const noexcept;
    [[maybe_unused]] void CrossProduct ( GXVec3 const &a, GXVec3 const &b ) noexcept;

    [[maybe_unused, nodiscard]] float Length () const noexcept;
    [[maybe_unused, nodiscard]] float SquaredLength () const noexcept;
    [[maybe_unused, nodiscard]] float Distance ( GXVec3 const &other ) const noexcept;
    [[maybe_unused, nodiscard]] float SquaredDistance ( GXVec3 const &other ) const noexcept;

    [[maybe_unused]] void LinearInterpolation ( GXVec3 const &start,
        GXVec3 const &finish,
        float interpolationFactor
    ) noexcept;

    // Note the axis must be a unit vector.
    [[maybe_unused]] void Project ( GXVec3 const &vector, GXVec3 const &axis ) noexcept;

    [[maybe_unused, nodiscard]] bool IsEqual ( GXVec3 const &other ) noexcept;

    // baseX - correct direction, adjustedY - desirable, adjustedZ - calculated.
    [[maybe_unused]] static void MakeOrthonormalBasis ( GXVec3 &baseX,
        GXVec3 &adjustedY,
        GXVec3 &adjustedZ
    ) noexcept;
};

//----------------------------------------------------------------------------------------------------------------------

[[maybe_unused]] bool GXRayTriangleIntersection3D ( float &outT,
    GXVec3 const &origin,
    GXVec3 const &direction,
    float length,
    GXVec3 const &a,
    GXVec3 const &b,
    GXVec3 const &c
) noexcept;

//----------------------------------------------------------------------------------------------------------------------

struct [[maybe_unused]] GXEuler final
{
    [[maybe_unused]] float      _pitchRadians;
    [[maybe_unused]] float      _yawRadians;
    [[maybe_unused]] float      _rollRadians;

    [[maybe_unused]] GXEuler () = default;

    [[maybe_unused]] GXEuler ( GXEuler const & ) = default;
    [[maybe_unused]] GXEuler &operator = ( GXEuler const & ) = default;

    [[maybe_unused]] GXEuler ( GXEuler && ) = default;
    [[maybe_unused]] GXEuler &operator = ( GXEuler && ) = default;

    // constexpr constructor is implicitly inline
    // see https://timsong-cpp.github.io/cppwp/n4140/dcl.constexpr
    [[maybe_unused]] constexpr GXEuler ( float pitchRadians,
        float yawRadians,
        float rollRadians
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
    float       _data[ 4U ];

    [[maybe_unused]] GXVec4 () = default;

    [[maybe_unused]] GXVec4 ( GXVec4 const & ) = default;
    [[maybe_unused]] GXVec4 &operator = ( GXVec4 const & ) = default;

    [[maybe_unused]] GXVec4 ( GXVec4 && ) = default;
    [[maybe_unused]] GXVec4 &operator = ( GXVec4 && ) = default;

    [[maybe_unused]] constexpr GXVec4 ( GXVec3 const &vector, float w ) noexcept:
        _data { vector._data[ 0U ], vector._data[ 1U ], vector._data[ 2U ], w }
    {
        // NOTHING
    }

    // constexpr constructor is implicitly inline
    // see https://timsong-cpp.github.io/cppwp/n4140/dcl.constexpr
    [[maybe_unused]] constexpr GXVec4 ( float x, float y, float z, float w ) noexcept:
        _data { x, y, z, w }
    {
        // NOTHING
    }

    [[maybe_unused]] ~GXVec4 () = default;

    [[maybe_unused]] void Init ( float x, float y, float z, float w ) noexcept;

    [[maybe_unused]] void SetX ( float x ) noexcept;
    [[maybe_unused, nodiscard]] float GetX () const noexcept;

    [[maybe_unused]] void SetY ( float y ) noexcept;
    [[maybe_unused, nodiscard]] float GetY () const noexcept;

    [[maybe_unused]] void SetZ ( float z ) noexcept;
    [[maybe_unused, nodiscard]] float GetZ () const noexcept;

    [[maybe_unused]] void SetW ( float w ) noexcept;
    [[maybe_unused, nodiscard]] float GetW () const noexcept;

    [[maybe_unused]] void Sum ( GXVec4 const &a, GXVec4 const &b ) noexcept;
    [[maybe_unused]] void Sum ( GXVec4 const &a, float bScale, GXVec4 const &b ) noexcept;
    [[maybe_unused]] void Subtract ( GXVec4 const &a, GXVec4 const &b ) noexcept;
    [[maybe_unused]] void Multiply ( GXVec4 const &a, float scale ) noexcept;
    [[maybe_unused]] void Multiply ( GXVec4 const &a, GXVec4 const &b ) noexcept;

    [[maybe_unused, nodiscard]] float DotProduct ( GXVec4 const &other ) const noexcept;

    [[maybe_unused, nodiscard]] float Length () const noexcept;
    [[maybe_unused, nodiscard]] float SquaredLength () const noexcept;
};

//----------------------------------------------------------------------------------------------------------------------

struct [[maybe_unused]] GXVec6 final
{
    float       _data[ 6U ];

    [[maybe_unused]] GXVec6 () = default;

    [[maybe_unused]] GXVec6 ( GXVec6 const & ) = default;
    [[maybe_unused]] GXVec6 &operator = ( GXVec6 const & ) = default;

    [[maybe_unused]] GXVec6 ( GXVec6 && ) = default;
    [[maybe_unused]] GXVec6 &operator = ( GXVec6 && ) = default;

    // constexpr constructor is implicitly inline
    // see https://timsong-cpp.github.io/cppwp/n4140/dcl.constexpr
    [[maybe_unused]] constexpr GXVec6 ( float a1,
        float a2,
        float a3,
        float a4,
        float a5,
        float a6
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

    [[maybe_unused]] void Init ( float a1, float a2, float a3, float a4, float a5, float a6 ) noexcept;
    [[maybe_unused]] void From ( GXVec3 const &v1, GXVec3 const &v2 ) noexcept;

    [[maybe_unused, nodiscard]] float DotProduct ( GXVec6 const &other ) const noexcept;
    [[maybe_unused]] void Sum ( GXVec6 const &a, GXVec6 const &b ) noexcept;
    [[maybe_unused]] void Sum ( GXVec6 const &a, float bScale, GXVec6 const &b ) noexcept;
    [[maybe_unused]] void Multiply ( GXVec6 const &a, float factor ) noexcept;
};

//----------------------------------------------------------------------------------------------------------------------

struct [[maybe_unused]] GXColorUNORM final
{
    // Stores components in red, green, blue, alpha order.
    uint8_t     _data[ 4U ];

    [[maybe_unused]] GXColorUNORM () = default;

    [[maybe_unused]] GXColorUNORM ( GXColorUNORM const & ) = default;
    [[maybe_unused]] GXColorUNORM &operator = ( GXColorUNORM const & ) = default;

    [[maybe_unused]] GXColorUNORM ( GXColorUNORM && ) = default;
    [[maybe_unused]] GXColorUNORM &operator = ( GXColorUNORM && ) = default;

    [[maybe_unused]] constexpr GXColorUNORM ( uint32_t red, uint32_t green, uint32_t blue, uint32_t alpha ) noexcept:
        _data
        {
            static_cast<uint8_t> ( red ),
            static_cast<uint8_t> ( green ),
            static_cast<uint8_t> ( blue ),
            static_cast<uint8_t> ( alpha )
        }
    {
        // NOTHING
    }

    [[maybe_unused]] constexpr GXColorUNORM ( uint8_t red, uint8_t green, uint8_t blue, uint8_t alpha ) noexcept:
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
    float       _data[ 4U ];

    [[maybe_unused]] GXColorRGB () = default;

    [[maybe_unused]] GXColorRGB ( GXColorRGB const & ) = default;
    [[maybe_unused]] GXColorRGB &operator = ( GXColorRGB const & ) = default;

    [[maybe_unused]] GXColorRGB ( GXColorRGB && ) = default;
    [[maybe_unused]] GXColorRGB &operator = ( GXColorRGB && ) = default;

    // constexpr constructor is implicitly inline
    // see https://timsong-cpp.github.io/cppwp/n4140/dcl.constexpr
    [[maybe_unused]] constexpr GXColorRGB ( float red, float green, float blue, float alpha ) noexcept:
        _data { red, green, blue, alpha }
    {
        // NOTHING
    }

    [[maybe_unused]] constexpr GXColorRGB ( uint32_t red, uint32_t green, uint32_t blue, float alpha ) noexcept:
        _data
        {
            GX_MATH_UNORM_FACTOR * static_cast<float> ( red ),
            GX_MATH_UNORM_FACTOR * static_cast<float> ( green ),
            GX_MATH_UNORM_FACTOR * static_cast<float> ( blue ),
            alpha
        }
    {
        // NOTHING
    }

    [[maybe_unused]] GXColorRGB ( uint8_t red, uint8_t green, uint8_t blue, float alpha ) noexcept;
    [[maybe_unused]] explicit GXColorRGB ( GXColorHSV const &color ) noexcept;
    [[maybe_unused]] explicit GXColorRGB ( GXColorUNORM color ) noexcept;

    [[maybe_unused]] ~GXColorRGB () = default;

    [[maybe_unused]] void Init ( float red, float green, float blue, float alpha ) noexcept;

    // [0.0F +inf)
    [[maybe_unused]] void SetRed ( float red ) noexcept;
    [[maybe_unused, nodiscard]] float GetRed () const noexcept;

    // [0.0F +inf)
    [[maybe_unused]] void SetGreen ( float green ) noexcept;
    [[maybe_unused, nodiscard]] float GetGreen () const noexcept;

    // [0.0F +inf)
    [[maybe_unused]] void SetBlue ( float blue ) noexcept;
    [[maybe_unused, nodiscard]] float GetBlue () const noexcept;

    // [0.0f 1.0F]
    [[maybe_unused]] void SetAlpha ( float alpha ) noexcept;
    [[maybe_unused, nodiscard]] float GetAlpha () const noexcept;

    [[maybe_unused]] void From ( uint8_t red, uint8_t green, uint8_t blue, float alpha ) noexcept;
    [[maybe_unused]] void From ( uint32_t red, uint32_t green, uint32_t blue, float alpha ) noexcept;
    [[maybe_unused]] void From ( GXColorHSV const &color ) noexcept;

    // It is assumed that current color space is sRGB.
    [[maybe_unused, nodiscard]] GXColorRGB ToLinearSpace () const noexcept;

    // It is assumed that current color space is linear space.
    [[maybe_unused, nodiscard]] GXColorRGB ToSRGB () const noexcept;

    [[maybe_unused, nodiscard]] GXColorUNORM ToColorUNORM () const noexcept;

    [[maybe_unused]] void ConvertToUByte ( uint8_t &red,
        uint8_t &green,
        uint8_t &blue,
        uint8_t &alpha
    ) const noexcept;
};

//----------------------------------------------------------------------------------------------------------------------

struct [[maybe_unused]] GXColorHSV final
{
    // Stores components in hue, saturation, value, alpha order.
    float       _data[ 4U ];

    [[maybe_unused]] GXColorHSV () = default;

    [[maybe_unused]] GXColorHSV ( GXColorHSV const & ) = default;
    [[maybe_unused]] GXColorHSV &operator = ( GXColorHSV const & ) = default;

    [[maybe_unused]] GXColorHSV ( GXColorHSV && ) = default;
    [[maybe_unused]] GXColorHSV &operator = ( GXColorHSV && ) = default;

    // constexpr constructor is implicitly inline
    // see https://timsong-cpp.github.io/cppwp/n4140/dcl.constexpr
    [[maybe_unused]] constexpr GXColorHSV ( float hue,
        float saturation,
        float value,
        float alpha
    ) noexcept:
        _data { hue, saturation, value, alpha }
    {
        // NOTHING
    }

    [[maybe_unused]] explicit GXColorHSV ( GXColorRGB const &color ) noexcept;

    [[maybe_unused]] ~GXColorHSV () = default;

    // [0.0F 360.0F]
    [[maybe_unused]] void SetHue ( float hue ) noexcept;
    [[maybe_unused, nodiscard]] float GetHue () const noexcept;

    // [0.0F 100.0F]
    [[maybe_unused]] void SetSaturation ( float saturation ) noexcept;
    [[maybe_unused, nodiscard]] float GetSaturation () const noexcept;

    // [0.0F 100.0F]
    [[maybe_unused]] void SetValue ( float value ) noexcept;
    [[maybe_unused, nodiscard]] float GetValue () const noexcept;

    // [0.0F 100.0F]
    [[maybe_unused]] void SetAlpha ( float alpha ) noexcept;
    [[maybe_unused, nodiscard]] float GetAlpha () const noexcept;

    [[maybe_unused]] void From ( GXColorRGB const &color ) noexcept;
};

//----------------------------------------------------------------------------------------------------------------------

struct [[maybe_unused]] GXPreciseComplex final
{
    double      _r;
    double      _i;

    [[maybe_unused]] GXPreciseComplex () = default;

    [[maybe_unused]] GXPreciseComplex ( GXPreciseComplex const &other ) = default;
    [[maybe_unused]] GXPreciseComplex &operator = ( GXPreciseComplex const &other ) = default;

    [[maybe_unused]] GXPreciseComplex ( GXPreciseComplex && ) = default;
    [[maybe_unused]] GXPreciseComplex &operator = ( GXPreciseComplex && ) = default;

    // constexpr constructor is implicitly inline
    // see https://timsong-cpp.github.io/cppwp/n4140/dcl.constexpr
    [[maybe_unused]] constexpr GXPreciseComplex ( double real, double imaginary ) noexcept:
        _r ( real ),
        _i ( imaginary )
    {
        // NOTHING
    }

    [[maybe_unused]] ~GXPreciseComplex () = default;

    [[maybe_unused]] void Init ( double real, double imaginary ) noexcept;

    [[maybe_unused, nodiscard]] double Length () const noexcept;
    [[maybe_unused, nodiscard]] double SquaredLength () const noexcept;

    // Method returns GX_FALSE if ( 0.0 + 0.0i ) ^ 0 will happen.
    [[maybe_unused]] bool Power ( uint32_t power ) noexcept;

    [[maybe_unused]] GXPreciseComplex operator + ( GXPreciseComplex const &other ) const noexcept;
    [[maybe_unused]] GXPreciseComplex operator - ( GXPreciseComplex const &other ) const noexcept;
    [[maybe_unused]] GXPreciseComplex operator * ( GXPreciseComplex const &other ) const noexcept;
    [[maybe_unused]] GXPreciseComplex operator * ( double a ) const noexcept;
    [[maybe_unused]] GXPreciseComplex operator / ( double a ) const noexcept;
};

//----------------------------------------------------------------------------------------------------------------------

struct GXMat3;
struct GXMat4;

// Quaternion representation: r + ai + bj + ck.
// By convention stores only orientation without any scale.
struct [[maybe_unused]] GXQuat final
{
    [[maybe_unused]] static GXQuat const    IDENTITY;

    // Stores quaternion components in r, a, b, c order.
    float                                   _data[ 4U ];

    [[maybe_unused]] GXQuat () = default;

    [[maybe_unused]] GXQuat ( GXQuat const & ) = default;
    [[maybe_unused]] GXQuat &operator = ( GXQuat const & ) = default;

    [[maybe_unused]] GXQuat ( GXQuat && ) = default;
    [[maybe_unused]] GXQuat &operator = ( GXQuat && ) = default;

    // constexpr constructor is implicitly inline
    // see https://timsong-cpp.github.io/cppwp/n4140/dcl.constexpr
    [[maybe_unused]] constexpr GXQuat ( float r, float a, float b, float c ) noexcept:
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
    [[maybe_unused, nodiscard]] uint32_t ToTBN32 ( bool reflectBitangent ) const noexcept;

    // "Real" component could be restored using unit quaternion property. It's guarantee to be positive real component
    // eliminating quaternion duality flaw.
    // bits 0-20: a component
    // bits 21-41: b component
    // bits 42-63: c component
    [[maybe_unused, nodiscard]] uint64_t ToTBN64 () const noexcept;

    // Packing TBN basis into R16G16B16A16_UNORM format.
    // bits 0-15: r component
    // bits 16-31: a component
    // bits 32-47: b component
    // bits 48-63: c component
    [[maybe_unused, nodiscard]] uint64_t ToQuat64 () const noexcept;

    [[maybe_unused]] void Init ( float r, float a, float b, float c ) noexcept;

    [[maybe_unused]] void SetR ( float r ) noexcept;
    [[maybe_unused, nodiscard]] float GetR () const noexcept;

    [[maybe_unused]] void SetA ( float a ) noexcept;
    [[maybe_unused, nodiscard]] float GetA () const noexcept;

    [[maybe_unused]] void SetB ( float b ) noexcept;
    [[maybe_unused, nodiscard]] float GetB () const noexcept;

    [[maybe_unused]] void SetC ( float c ) noexcept;
    [[maybe_unused, nodiscard]] float GetC () const noexcept;

    [[maybe_unused]] void Identity () noexcept;
    [[maybe_unused]] void Normalize () noexcept;
    [[maybe_unused]] void Inverse ( GXQuat const &q ) noexcept;

    // Result is valid if "unitQuaternion" is normalized.
    [[maybe_unused]] void InverseFast ( GXQuat const &unitQuaternion ) noexcept;

    [[maybe_unused]] void FromAxisAngle ( float x, float y, float z, float angle ) noexcept;
    [[maybe_unused]] void FromAxisAngle ( GXVec3 const &axis, float angle ) noexcept;

    // Result is valid if rotationMatrix is rotation matrix. Any scale will be ignored.
    [[maybe_unused]] void From ( GXMat3 const &rotationMatrix ) noexcept;

    // Result is valid if rotationMatrix is rotation matrix. Any scale will be ignored.
    [[maybe_unused]] void From ( GXMat4 const &rotationMatrix ) noexcept;

    // Result is valid if forward is unit vector.
    [[maybe_unused]] void From ( GXVec3 const &forward, GXVec3 const &up ) noexcept;

    // Result is valid if pureRotationMatrix is not scaled rotation matrix.
    [[maybe_unused]] void FromFast ( GXMat3 const &pureRotationMatrix ) noexcept;

    // Result is valid if pureRotationMatrix is not scaled rotation matrix.
    [[maybe_unused]] void FromFast ( GXMat4 const &pureRotationMatrix ) noexcept;

    [[maybe_unused]] void Multiply ( GXQuat const &a, GXQuat const &b ) noexcept;
    [[maybe_unused]] void Multiply ( GXQuat const &q, float scale ) noexcept;
    [[maybe_unused]] void Sum ( GXQuat const &a, GXQuat const &b ) noexcept;
    [[maybe_unused]] void Subtract ( GXQuat const &a, GXQuat const &b ) noexcept;

    [[maybe_unused]] void SphericalLinearInterpolation ( GXQuat const &start,
        GXQuat const &finish,
        float interpolationFactor
    ) noexcept;

    [[maybe_unused]] void GetAxisAngle ( GXVec3 &axis, float &angle ) const noexcept;
    [[maybe_unused]] void Transform ( GXVec3 &out, GXVec3 const &v ) const noexcept;

    // X axis of corresponding 3x3 matrix.
    // Result is valid if quaternion is normalized.
    [[maybe_unused]] void GetRight ( GXVec3 &out ) const noexcept;

    // Y axis of corresponding 3x3 matrix.
    // Result is valid if quaternion is normalized.
    [[maybe_unused]] void GetUp ( GXVec3 &out ) const noexcept;

    // Z axis of corresponding 3x3 matrix.
    // Result is valid if quaternion is normalized.
    [[maybe_unused]] void GetForward ( GXVec3 &out ) const noexcept;

    // Result is valid if quaternion is normalized.
    [[maybe_unused]] void TransformFast ( GXVec3 &out, GXVec3 const &v ) const noexcept;
};

//----------------------------------------------------------------------------------------------------------------------

struct [[maybe_unused]] GXMat3 final
{
    float       _data[ 3U ][ 3U ];

    [[maybe_unused]] GXMat3 () = default;

    [[maybe_unused]] GXMat3 ( GXMat3 const & ) = default;
    [[maybe_unused]] GXMat3 &operator = ( GXMat3 const & ) = default;

    [[maybe_unused]] GXMat3 ( GXMat3 && ) = default;
    [[maybe_unused]] GXMat3 &operator = ( GXMat3 && ) = default;

    [[maybe_unused]] explicit GXMat3 ( GXMat4 const &matrix ) noexcept;

    [[maybe_unused]] ~GXMat3 () = default;

    [[maybe_unused]] void From ( GXQuat const &quaternion ) noexcept;
    [[maybe_unused]] void From ( GXMat4 const &matrix ) noexcept;

    // Constructs orthonormal basis. Result is valid if zDirection is unit vector.
    [[maybe_unused]] void From ( GXVec3 const &zDirection ) noexcept;

    // Constructs orthonormal basis. Result is valid if forward is unit vector.
    [[maybe_unused]] void From ( GXVec3 const &forward, GXVec3 const &up ) noexcept;

    // Result is valid if quaternion is normalized.
    [[maybe_unused]] void FromFast ( GXQuat const &quaternion ) noexcept;

    [[maybe_unused]] void SetX ( GXVec3 const &x ) noexcept;
    [[maybe_unused]] void GetX ( GXVec3 &x ) const noexcept;

    [[maybe_unused]] void SetY ( GXVec3 const &y ) noexcept;
    [[maybe_unused]] void GetY ( GXVec3 &y ) const noexcept;

    [[maybe_unused]] void SetZ ( GXVec3 const &z ) noexcept;
    [[maybe_unused]] void GetZ ( GXVec3 &z ) const noexcept;

    [[maybe_unused, nodiscard]] GXVec3 const &Right () const noexcept;
    [[maybe_unused, nodiscard]] GXVec3 &Right () noexcept;

    [[maybe_unused, nodiscard]] GXVec3 const &Up () const noexcept;
    [[maybe_unused, nodiscard]] GXVec3 &Up () noexcept;

    [[maybe_unused, nodiscard]] GXVec3 const &Forward () const noexcept;
    [[maybe_unused, nodiscard]] GXVec3 &Forward () noexcept;

    [[maybe_unused]] void Identity () noexcept;
    [[maybe_unused]] void Zeros () noexcept;

    [[maybe_unused]] void Inverse ( GXMat3 const &sourceMatrix ) noexcept;
    [[maybe_unused]] void Transpose ( GXMat3 const &sourceMatrix ) noexcept;
    [[maybe_unused]] void ClearRotation ( GXMat3 const &sourceMatrix ) noexcept;
    [[maybe_unused]] void ClearRotation ( GXMat4 const &sourceMatrix ) noexcept;

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
    [[maybe_unused]] void SkewSymmetric ( GXVec3 const &base ) noexcept;

    [[maybe_unused]] void Sum ( GXMat3 const &a, GXMat3 const &b ) noexcept;
    [[maybe_unused]] void Subtract ( GXMat3 const &a, GXMat3 const &b ) noexcept;
    [[maybe_unused]] void Multiply ( GXMat3 const &a, GXMat3 const &b ) noexcept;

    [[maybe_unused]] void MultiplyVectorMatrix ( GXVec3 &out, GXVec3 const &v ) const noexcept;
    [[maybe_unused]] void MultiplyMatrixVector ( GXVec3 &out, GXVec3 const &v ) const noexcept;

    [[maybe_unused]] void Multiply ( GXMat3 const &a, float factor ) noexcept;
};

//----------------------------------------------------------------------------------------------------------------------

struct [[maybe_unused]] GXMat4 final
{
    [[maybe_unused]] static GXMat4 const    IDENTITY;

    float                                   _data[ 4U ][ 4U ];

    [[maybe_unused]] GXMat4 () = default;

    [[maybe_unused]] GXMat4 ( GXMat4 const & ) = default;
    [[maybe_unused]] GXMat4 &operator = ( GXMat4 const & ) = default;

    [[maybe_unused]] GXMat4 ( GXMat4 && ) = default;
    [[maybe_unused]] GXMat4 &operator = ( GXMat4 && ) = default;

    // constexpr constructor is implicitly inline
    // see https://timsong-cpp.github.io/cppwp/n4140/dcl.constexpr
    [[maybe_unused]] constexpr GXMat4 ( float m00,
        float m01,
        float m02,
        float m03,
        float m10,
        float m11,
        float m12,
        float m13,
        float m20,
        float m21,
        float m22,
        float m23,
        float m30,
        float m31,
        float m32,
        float m33
    ) noexcept:
        _data { { m00, m01, m02, m03 }, { m10, m11, m12, m13 }, { m20, m21, m22, m23 }, { m30, m31, m32, m33 } }
    {
        // NOTHING
    }

    [[maybe_unused]] ~GXMat4 () = default;

    [[maybe_unused]] void SetRotation ( GXQuat const &quaternion ) noexcept;

    // Result is valid if quaternion is normalized.
    [[maybe_unused]] void SetRotationFast ( GXQuat const &quaternion ) noexcept;

    [[maybe_unused]] void SetOrigin ( GXVec3 const &origin ) noexcept;
    [[maybe_unused]] void From ( GXQuat const &quaternion, GXVec3 const &origin ) noexcept;
    [[maybe_unused]] void From ( GXMat3 const &rotation, GXVec3 const &origin ) noexcept;
    [[maybe_unused]] void From ( GXVec3 const &zDirection, GXVec3 const &origin ) noexcept;

    // Result is valid if quaternion is normalized.
    [[maybe_unused]] void FromFast ( GXQuat const &quaternion, GXVec3 const &origin ) noexcept;

    [[maybe_unused]] void SetX ( GXVec3 const &x ) noexcept;
    [[maybe_unused]] void GetX ( GXVec3 &x ) const noexcept;

    [[maybe_unused]] void SetY ( GXVec3 const &y ) noexcept;
    [[maybe_unused]] void GetY ( GXVec3 &y ) const noexcept;

    [[maybe_unused]] void SetZ ( GXVec3 const &z ) noexcept;
    [[maybe_unused]] void GetZ ( GXVec3 &z ) const noexcept;

    [[maybe_unused]] void SetW ( GXVec3 const &w ) noexcept;
    [[maybe_unused]] void GetW ( GXVec3 &w ) const noexcept;

    [[maybe_unused, nodiscard]] GXVec3 const &Right () const noexcept;
    [[maybe_unused, nodiscard]] GXVec3 &Right () noexcept;

    [[maybe_unused, nodiscard]] GXVec3 const &Up () const noexcept;
    [[maybe_unused, nodiscard]] GXVec3 &Up () noexcept;

    [[maybe_unused, nodiscard]] GXVec3 const &Forward () const noexcept;
    [[maybe_unused, nodiscard]] GXVec3 &Forward () noexcept;

    [[maybe_unused, nodiscard]] GXVec3 const &Location () const noexcept;
    [[maybe_unused, nodiscard]] GXVec3 &Location () noexcept;

    [[maybe_unused]] void Identity () noexcept;

    // Reverse Z, infinite far plane projection matrix.
    // 2026/09/29 - near is declared as define in minwindef.h on Windows platform. Need to avoid that naming.
    [[maybe_unused]] void Perspective ( float fieldOfViewYRadians, float aspectRatio, float zNear ) noexcept;

    // 2026/09/29 - near and far are declared as defines in minwindef.h on Windows platform. Need to avoid that naming.
    [[maybe_unused]] void Ortho ( float width, float height, float zNear, float zFar ) noexcept;

    [[maybe_unused]] void Translation ( float x, float y, float z ) noexcept;
    [[maybe_unused]] void Translation ( GXVec3 const &location ) noexcept;

    [[maybe_unused]] void TranslateTo ( float x, float y, float z ) noexcept;
    [[maybe_unused]] void TranslateTo ( GXVec3 const &location ) noexcept;

    [[maybe_unused]] void RotationX ( float angle ) noexcept;
    [[maybe_unused]] void RotationY ( float angle ) noexcept;
    [[maybe_unused]] void RotationZ ( float angle ) noexcept;
    [[maybe_unused]] void RotationXY ( float pitchRadians, float yawRadians ) noexcept;
    [[maybe_unused]] void RotationXYZ ( float pitchRadians, float yawRadians, float rollRadians ) noexcept;
    [[maybe_unused]] void ClearRotation ( GXMat3 const &sourceMatrix ) noexcept;
    [[maybe_unused]] void ClearRotation ( GXMat4 const &sourceMatrix ) noexcept;

    [[maybe_unused]] void Scale ( float x, float y, float z ) noexcept;
    [[maybe_unused]] void ClearScale ( GXVec3 &scale ) const noexcept;

    [[maybe_unused]] void Inverse ( GXMat4 const &sourceMatrix ) noexcept;

    [[maybe_unused]] void Multiply ( GXMat4 const &a, GXMat4 const &b ) noexcept;

    // Multiply row-vector [1x4] by own matrix [4x4].
    [[maybe_unused]] void MultiplyVectorMatrix ( GXVec4 &out, GXVec4 const &v ) const noexcept;

    // Multiply own matrix [4x4] by column-vector [4x1].
    [[maybe_unused]] void MultiplyMatrixVector ( GXVec4 &out, GXVec4 const &v ) const noexcept;

    // Multiply row-vector [1x3] by own matrix sub matrix [3x3].
    [[maybe_unused]] void MultiplyAsNormal ( GXVec3 &out, GXVec3 const &v ) const noexcept;

    // Multiply row-vector [1x3] by own matrix sub matrix [3x3] and add own w-vector.
    [[maybe_unused]] void MultiplyAsPoint ( GXVec3 &out, GXVec3 const &v ) const noexcept;
};

//----------------------------------------------------------------------------------------------------------------------

struct [[maybe_unused]] GXAABB final
{
    uint8_t     _vertices;

    GXVec3      _min;
    GXVec3      _max;

    [[maybe_unused]] constexpr GXAABB () noexcept:
        _vertices ( UINT8_C ( 0 ) ),

        _min (
            std::numeric_limits<float>::max (),
            std::numeric_limits<float>::max (),
            std::numeric_limits<float>::max ()
        ),

        _max (
            -std::numeric_limits<float>::max (),
            -std::numeric_limits<float>::max (),
            -std::numeric_limits<float>::max ()
        )
    {
        // NOTHING
    }

    [[maybe_unused]] GXAABB ( GXAABB const & ) = default;
    [[maybe_unused]] GXAABB &operator = ( GXAABB const & ) = default;

    [[maybe_unused]] GXAABB ( GXAABB && ) = default;
    [[maybe_unused]] GXAABB &operator = ( GXAABB && ) = default;

    [[maybe_unused]] ~GXAABB () = default;

    [[maybe_unused]] void Empty () noexcept;

    [[maybe_unused]] void Transform ( GXAABB &bounds, GXMat4 const &transform ) const noexcept;
    [[maybe_unused]] void AddVertex ( GXVec3 const &vertex ) noexcept;
    [[maybe_unused]] void AddVertex ( float x, float y, float z ) noexcept;

    [[maybe_unused, nodiscard]] bool IsOverlapped ( GXAABB const &other ) const noexcept;
    [[maybe_unused, nodiscard]] bool IsOverlapped ( GXVec3 const &point ) const noexcept;
    [[maybe_unused, nodiscard]] bool IsOverlapped ( float x, float y, float z ) const noexcept;

    [[maybe_unused]] void GetCenter ( GXVec3 &center ) const noexcept;
    [[maybe_unused, nodiscard]] float GetWidth () const noexcept;
    [[maybe_unused, nodiscard]] float GetHeight () const noexcept;
    [[maybe_unused, nodiscard]] float GetDepth () const noexcept;
    [[maybe_unused, nodiscard]] float GetSphereRadius () const noexcept;
};

//----------------------------------------------------------------------------------------------------------------------

enum class eGXPlaneClassifyVertex : uint8_t
{
    InFront = UINT8_C ( 0 ),
    On = UINT8_C ( 1 ),
    Behind = UINT8_C ( 2 )
};

struct [[maybe_unused]] GXPlane final
{
    float       _a;
    float       _b;
    float       _c;
    float       _d;

    [[maybe_unused]] GXPlane () = default;

    [[maybe_unused]] GXPlane ( GXPlane const & ) = default;
    [[maybe_unused]] GXPlane &operator = ( GXPlane const & ) = default;

    [[maybe_unused]] GXPlane ( GXPlane && ) = default;
    [[maybe_unused]] GXPlane &operator = ( GXPlane && ) = default;

    [[maybe_unused]] ~GXPlane () = default;

    [[maybe_unused]] void From ( GXVec3 const &pointA, GXVec3 const &pointB, GXVec3 const &pointC ) noexcept;

    [[maybe_unused]] void FromLineToPoint ( GXVec3 const &lineStart,
        GXVec3 const &lineEnd,
        GXVec3 const &point
    ) noexcept;

    [[maybe_unused]] void Normalize () noexcept;
    [[maybe_unused]] void Flip () noexcept;

    [[maybe_unused, nodiscard]] eGXPlaneClassifyVertex ClassifyVertex ( GXVec3 const &vertex ) const noexcept;
    [[maybe_unused, nodiscard]] eGXPlaneClassifyVertex ClassifyVertex ( float x, float y, float z ) const noexcept;
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
        [[maybe_unused]] void From ( GXMat4 const &src ) noexcept;

        // Trivial invisibility test.
        [[maybe_unused, nodiscard]] bool IsVisible ( GXAABB const &bounds ) const noexcept;

    private:
        [[nodiscard]] uint8_t PlaneTest ( float x, float y, float z ) const noexcept;
};

//---------------------------------------------------------------------------------------------------------------------

class [[maybe_unused]] GXProjectionInfiniteFarClipPlanes final
{
    private:
        GXPlane     _planes[ 5U ];

    public:
        [[maybe_unused]] GXProjectionInfiniteFarClipPlanes () = default;

        [[maybe_unused]] GXProjectionInfiniteFarClipPlanes ( GXProjectionInfiniteFarClipPlanes const & ) = default;

        [[maybe_unused]] GXProjectionInfiniteFarClipPlanes &operator = (
            GXProjectionInfiniteFarClipPlanes const &
        ) = default;

        [[maybe_unused]] GXProjectionInfiniteFarClipPlanes ( GXProjectionInfiniteFarClipPlanes && ) = default;

        [[maybe_unused]] GXProjectionInfiniteFarClipPlanes &operator = (
            GXProjectionInfiniteFarClipPlanes &&
        ) = default;

        [[maybe_unused]] explicit GXProjectionInfiniteFarClipPlanes ( GXMat4 const &src ) noexcept;

        [[maybe_unused]] ~GXProjectionInfiniteFarClipPlanes () = default;

        // Normals will be directed inside view volume.
        [[maybe_unused]] void From ( GXMat4 const &src ) noexcept;

        // Trivial invisibility test.
        [[maybe_unused, nodiscard]] bool IsVisible ( GXAABB const &bounds ) const noexcept;

    private:
        [[nodiscard]] uint8_t PlaneTest ( float x, float y, float z ) const noexcept;
};

//---------------------------------------------------------------------------------------------------------------------

[[maybe_unused, nodiscard]] constexpr float GXDegToRad ( float degrees ) noexcept
{
    constexpr float toRadians = 1.74532925e-2F;
    return degrees * toRadians;
}

[[maybe_unused, nodiscard]] constexpr float GXRadToDeg ( float radians ) noexcept
{
    constexpr float toDegrees = 5.72957795e+1F;
    return radians * toDegrees;
}

[[maybe_unused]] void GXRandomize () noexcept;
[[maybe_unused, nodiscard]] float GXRandomNormalize () noexcept;
[[maybe_unused, nodiscard]] float GXRandomBetween ( float from, float to ) noexcept;
[[maybe_unused]] void GXRandomBetween ( GXVec3 &out, GXVec3 const &from, GXVec3 const &to ) noexcept;

[[maybe_unused]] void GXGetTangentBitangent ( GXVec3 &outTangent,
    GXVec3 &outBitangent,
    uint8_t vertexID,
    uint8_t const* vertices,
    size_t vertexStride,
    uint8_t const* uvs,
    size_t uvStride
) noexcept;

[[maybe_unused, nodiscard]] float GXClampf ( float value, float minValue, float maxValue ) noexcept;
[[maybe_unused, nodiscard]] int32_t GXClampi ( int32_t value, int32_t minValue, int32_t maxValue ) noexcept;

[[maybe_unused]] void GXGetBarycentricCoords ( GXVec3 &out,
    GXVec3 const &point,
    GXVec3 const &aPivot,
    GXVec3 const &bPivot,
    GXVec3 const &cPivot
) noexcept;

[[maybe_unused]] void GXGetRayFromViewer ( GXVec3 &origin,
    GXVec3 &direction,
    uint16_t x,
    uint16_t y,
    uint16_t viewportWidth,
    uint16_t viewportHeight,
    GXVec3 const &viewerLocation,
    GXMat4 const &viewProjectionMatrix
) noexcept;


#endif // GX_MATH_HPP
