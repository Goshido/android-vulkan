#include <precompiled_headers.hpp>
#include <tools/hlsl.hpp>


namespace hlsl {

swizzle_uint32_t2::operator uint32_t2 () const noexcept
{
    return { *x, *y };
}

void swizzle_uint32_t2::operator >>= ( uint32_t v ) noexcept
{
    *x += v;
    *y += v;
}

void swizzle_uint32_t2::operator += ( uint32_t2 const &v ) noexcept
{
    *x += v.x;
    *y += v.y;
}

void swizzle_uint32_t2::operator -= ( uint32_t2 const &v ) noexcept
{
    *x -= v.x;
    *y -= v.y;
}

void swizzle_uint32_t2::operator *= ( uint32_t2 const &v ) noexcept
{
    *x *= v.x;
    *y *= v.y;
}

void swizzle_uint32_t2::operator /= ( uint32_t2 const &v ) noexcept
{
    *x /= v.x;
    *y /= v.y;
}

void swizzle_uint32_t2::operator >>= ( uint32_t2 const &v ) noexcept
{
    *x >>= v.x;
    *y >>= v.y;
}

void swizzle_uint32_t2::operator <<= ( uint32_t2 const &v ) noexcept
{
    *x <<= v.x;
    *y <<= v.y;
}

void swizzle_uint32_t2::operator &= ( uint32_t2 const &v ) noexcept
{
    *x &= v.x;
    *y &= v.y;
}

void swizzle_uint32_t2::operator |= ( uint32_t2 const &v ) noexcept
{
    *x |= v.x;
    *y |= v.y;
}

void swizzle_uint32_t2::operator += ( swizzle_uint32_t2 const &v ) noexcept
{
    *x += *v.x;
    *y += *v.y;
}

void swizzle_uint32_t2::operator -= ( swizzle_uint32_t2 const &v ) noexcept
{
    *x -= *v.x;
    *y -= *v.y;
}

void swizzle_uint32_t2::operator *= ( swizzle_uint32_t2 const &v ) noexcept
{
    *x *= *v.x;
    *y *= *v.y;
}

void swizzle_uint32_t2::operator /= ( swizzle_uint32_t2 const &v ) noexcept
{
    *x /= *v.x;
    *y /= *v.y;
}

void swizzle_uint32_t2::operator >>= ( swizzle_uint32_t2 const &v ) noexcept
{
    *x >>= *v.x;
    *y >>= *v.y;
}

void swizzle_uint32_t2::operator <<= ( swizzle_uint32_t2 const &v ) noexcept
{
    *x <<= *v.x;
    *y <<= *v.y;
}

void swizzle_uint32_t2::operator &= ( swizzle_uint32_t2 const &v ) noexcept
{
    *x &= *v.x;
    *y &= *v.y;
}

void swizzle_uint32_t2::operator |= ( swizzle_uint32_t2 const &v ) noexcept
{
    *x |= *v.x;
    *y |= *v.y;
}

//----------------------------------------------------------------------------------------------------------------------

swizzle_uint32_t3::operator uint32_t3 () const noexcept
{
    return { *x, *y, *z };
}

void swizzle_uint32_t3::operator += ( uint32_t3 const &v ) noexcept
{
    *x += v.x;
    *y += v.y;
    *z += v.z;
}

void swizzle_uint32_t3::operator -= ( uint32_t3 const &v ) noexcept
{
    *x -= v.x;
    *y -= v.y;
    *z -= v.z;
}

void swizzle_uint32_t3::operator *= ( uint32_t3 const &v ) noexcept
{
    *x *= v.x;
    *y *= v.y;
    *z *= v.z;
}

void swizzle_uint32_t3::operator /= ( uint32_t3 const &v ) noexcept
{
    *x /= v.x;
    *y /= v.y;
    *x /= v.x;
}

void swizzle_uint32_t3::operator >>= ( uint32_t3 const &v ) noexcept
{
    *x >>= v.x;
    *y >>= v.y;
    *z >>= v.z;
}

void swizzle_uint32_t3::operator <<= ( uint32_t3 const &v ) noexcept
{
    *x <<= v.x;
    *y <<= v.y;
    *z <<= v.z;
}

void swizzle_uint32_t3::operator &= ( uint32_t3 const &v ) noexcept
{
    *x &= v.x;
    *y &= v.y;
    *z &= v.z;
}

void swizzle_uint32_t3::operator |= ( uint32_t3 const &v ) noexcept
{
    *x |= v.x;
    *y |= v.y;
    *z |= v.z;
}

void swizzle_uint32_t3::operator += ( swizzle_uint32_t3 const &v ) noexcept
{
    *x += *v.x;
    *y += *v.y;
    *z += *v.z;
}

void swizzle_uint32_t3::operator -= ( swizzle_uint32_t3 const &v ) noexcept
{
    *x -= *v.x;
    *y -= *v.y;
    *z -= *v.z;
}

void swizzle_uint32_t3::operator *= ( swizzle_uint32_t3 const &v ) noexcept
{
    *x *= *v.x;
    *y *= *v.y;
    *z *= *v.z;
}

void swizzle_uint32_t3::operator /= ( swizzle_uint32_t3 const &v ) noexcept
{
    *x /= *v.x;
    *y /= *v.y;
    *z /= *v.z;
}

void swizzle_uint32_t3::operator >>= ( swizzle_uint32_t3 const &v ) noexcept
{
    *x >>= *v.x;
    *y >>= *v.y;
    *z >>= *v.z;
}

void swizzle_uint32_t3::operator <<= ( swizzle_uint32_t3 const &v ) noexcept
{
    *x <<= *v.x;
    *y <<= *v.y;
    *z <<= *v.z;
}

void swizzle_uint32_t3::operator &= ( swizzle_uint32_t3 const &v ) noexcept
{
    *x &= *v.x;
    *y &= *v.y;
    *z &= *v.z;
}

void swizzle_uint32_t3::operator |= ( swizzle_uint32_t3 const &v ) noexcept
{
    *x |= *v.x;
    *y |= *v.y;
    *z |= *v.z;
}

//----------------------------------------------------------------------------------------------------------------------

swizzle_uint32_t4::operator uint32_t4 () const noexcept
{
    return { *x, *y, *z, *w };
}

void swizzle_uint32_t4::operator += ( uint32_t4 const &v ) noexcept
{
    *x += v.x;
    *y += v.y;
    *z += v.z;
    *w += v.w;
}

void swizzle_uint32_t4::operator -= ( uint32_t4 const &v ) noexcept
{
    *x -= v.x;
    *y -= v.y;
    *z -= v.z;
    *w -= v.w;
}

void swizzle_uint32_t4::operator *= ( uint32_t4 const &v ) noexcept
{
    *x *= v.x;
    *y *= v.y;
    *z *= v.z;
    *w *= v.w;
}

void swizzle_uint32_t4::operator /= ( uint32_t4 const &v ) noexcept
{
    *x /= v.x;
    *y /= v.y;
    *x /= v.x;
    *w /= v.w;
}

void swizzle_uint32_t4::operator >>= ( uint32_t4 const &v ) noexcept
{
    *x >>= v.x;
    *y >>= v.y;
    *z >>= v.z;
    *w >>= v.w;
}

void swizzle_uint32_t4::operator <<= ( uint32_t4 const &v ) noexcept
{
    *x <<= v.x;
    *y <<= v.y;
    *z <<= v.z;
    *w <<= v.w;
}

void swizzle_uint32_t4::operator &= ( uint32_t4 const &v ) noexcept
{
    *x &= v.x;
    *y &= v.y;
    *z &= v.z;
    *w &= v.w;
}

void swizzle_uint32_t4::operator |= ( uint32_t4 const &v ) noexcept
{
    *x |= v.x;
    *y |= v.y;
    *z |= v.z;
    *w |= v.w;
}

void swizzle_uint32_t4::operator += ( swizzle_uint32_t4 const &v ) noexcept
{
    *x += *v.x;
    *y += *v.y;
    *z += *v.z;
    *w += *v.w;
}

void swizzle_uint32_t4::operator -= ( swizzle_uint32_t4 const &v ) noexcept
{
    *x -= *v.x;
    *y -= *v.y;
    *z -= *v.z;
    *w -= *v.w;
}

void swizzle_uint32_t4::operator *= ( swizzle_uint32_t4 const &v ) noexcept
{
    *x *= *v.x;
    *y *= *v.y;
    *z *= *v.z;
    *w *= *v.w;
}

void swizzle_uint32_t4::operator /= ( swizzle_uint32_t4 const &v ) noexcept
{
    *x /= *v.x;
    *y /= *v.y;
    *z /= *v.z;
    *w /= *v.w;
}

void swizzle_uint32_t4::operator >>= ( swizzle_uint32_t4 const &v ) noexcept
{
    *x >>= *v.x;
    *y >>= *v.y;
    *z >>= *v.z;
    *w >>= *v.w;
}

void swizzle_uint32_t4::operator <<= ( swizzle_uint32_t4 const &v ) noexcept
{
    *x <<= *v.x;
    *y <<= *v.y;
    *z <<= *v.z;
    *w <<= *v.w;
}

void swizzle_uint32_t4::operator &= ( swizzle_uint32_t4 const &v ) noexcept
{
    *x &= *v.x;
    *y &= *v.y;
    *z &= *v.z;
    *w &= *v.w;
}

void swizzle_uint32_t4::operator |= ( swizzle_uint32_t4 const &v ) noexcept
{
    *x |= *v.x;
    *y |= *v.y;
    *z |= *v.z;
    *w |= *v.w;
}

//----------------------------------------------------------------------------------------------------------------------

uint32_t2::uint32_t2 () noexcept
{
    InitSwizzle ();
}

[[maybe_unused]] uint32_t2::uint32_t2 ( uint32_t2 const &other ) noexcept:
    x ( other.x ),
    y ( other.y )
{
    InitSwizzle ();
}

uint32_t2 &uint32_t2::operator = ( uint32_t2 const &other ) noexcept
{
    x = other.x;
    y = other.y;
    InitSwizzle ();
    return *this;
}

uint32_t2::uint32_t2 ( uint32_t2 &&other ) noexcept:
    x ( other.x ),
    y ( other.y )
{
    InitSwizzle ();
}

uint32_t2 &uint32_t2::operator = ( uint32_t2 &&other ) noexcept
{
    if ( this == &other ) [[unlikely]]
        return *this;

    x = other.x;
    y = other.y;
    InitSwizzle ();
    return *this;
}

uint32_t2::uint32_t2 ( uint32_t v ) noexcept:
    x ( v ),
    y ( v )
{
    InitSwizzle ();
}

uint32_t2::uint32_t2 ( uint32_t xVal, uint32_t yVal ) noexcept:
    x ( xVal ),
    y ( yVal )
{
    InitSwizzle ();
}

void uint32_t2::operator += ( uint32_t2 const &v ) noexcept
{
    x += v.x;
    y += v.y;
}

void uint32_t2::operator -= ( uint32_t2 const &v ) noexcept
{
    x -= v.x;
    y -= v.y;
}

void uint32_t2::operator *= ( uint32_t2 const &v ) noexcept
{
    x *= v.x;
    y *= v.y;
}

void uint32_t2::operator /= ( uint32_t2 const &v ) noexcept
{
    x /= v.x;
    y /= v.y;
}

void uint32_t2::operator >>= ( uint32_t v ) noexcept
{
    x >>= v;
    y >>= v;
}

void uint32_t2::operator >>= ( uint32_t2 const &v ) noexcept
{
    x >>= v.x;
    y >>= v.y;
}

void uint32_t2::operator <<= ( uint32_t2 const &v ) noexcept
{
    x <<= v.x;
    y <<= v.y;
}

void uint32_t2::operator &= ( uint32_t2 const &v ) noexcept
{
    x &= v.x;
    y &= v.y;
}

void uint32_t2::operator |= ( uint32_t2 const &v ) noexcept
{
    x |= v.x;
    y |= v.y;
}

uint32_t2 operator + ( uint32_t2 const &a, uint32_t2 const &b ) noexcept
{
    return { a.x + b.x, a.y + b.y };
}

uint32_t2 operator - ( uint32_t2 const &a, uint32_t2 const &b ) noexcept
{
    return { a.x - b.x, a.y - b.y };
}

uint32_t2 operator * ( uint32_t2 const &a, uint32_t2 const &b ) noexcept
{
    return { a.x * b.x, a.y * b.y };
}

uint32_t2 operator / ( uint32_t2 const &a, uint32_t2 const &b ) noexcept
{
    return { a.x / b.x, a.y / b.y };
}

uint32_t2 operator >> ( uint32_t2 const& a, uint32_t b ) noexcept
{
    return { a.x >> b, a.y >> b };
}

uint32_t2 operator >> ( uint32_t2 const &a, uint32_t2 const &b ) noexcept
{
    return { a.x >> b.x, a.y >> b.y };
}

uint32_t2 operator << ( uint32_t2 const &a, uint32_t2 const &b ) noexcept
{
    return { a.x << b.x, a.y << b.y };
}

uint32_t2 operator & ( uint32_t2 const &a, uint32_t2 const &b ) noexcept
{
    return { a.x & b.x, a.y & b.y };
}

uint32_t2 operator | ( uint32_t2 const &a, uint32_t2 const &b ) noexcept
{
    return { a.x | b.x, a.y | b.y };
}

void uint32_t2::InitSwizzle () noexcept
{
    xx = swizzle_uint32_t2 ( &x, &x );
    yx = swizzle_uint32_t2 ( &y, &x );
    yy = swizzle_uint32_t2 ( &y, &y );
    xxx = swizzle_uint32_t3 ( &x, &x, &x );
    yyy = swizzle_uint32_t3 ( &y, &y, &y );
}

//----------------------------------------------------------------------------------------------------------------------

uint32_t3::uint32_t3 () noexcept
{
    InitSwizzle ();
}

uint32_t3::uint32_t3 ( uint32_t3 const &other ) noexcept:
    x ( other.x ),
    y ( other.y ),
    z ( other.z )
{
    InitSwizzle ();
}

uint32_t3 &uint32_t3::operator = ( uint32_t3 const &other ) noexcept
{
    x = other.x;
    y = other.y;
    z = other.z;
    InitSwizzle ();
    return *this;
}

uint32_t3::uint32_t3 ( uint32_t3 &&other ) noexcept:
    x ( other.x ),
    y ( other.y ),
    z ( other.z )
{
    InitSwizzle ();
}

uint32_t3 &uint32_t3::operator = ( uint32_t3 &&other ) noexcept
{
    if ( this == &other ) [[unlikely]]
        return *this;

    x = other.x;
    y = other.y;
    z = other.z;
    InitSwizzle ();
    return *this;
}

uint32_t3::uint32_t3 ( uint32_t v ) noexcept:
    x ( v ),
    y ( v ),
    z ( v )
{
    InitSwizzle ();
}

uint32_t3::uint32_t3 ( uint32_t xVal, uint32_t yVal, uint32_t zVal ) noexcept:
    x ( xVal ),
    y ( yVal ),
    z ( zVal )
{
    InitSwizzle ();
}

uint32_t3::uint32_t3 ( uint32_t xVal, uint32_t2 const &yzVal ) noexcept:
    x ( xVal ),
    y ( yzVal.x ),
    z ( yzVal.y )
{
    InitSwizzle ();
}

uint32_t3::uint32_t3 ( uint32_t2 const &xyVal, uint32_t zVal ) noexcept:
    x ( xyVal.x ),
    y ( xyVal.y ),
    z ( zVal )
{
    InitSwizzle ();
}

void uint32_t3::operator += ( uint32_t3 const &v ) noexcept
{
    x += v.x;
    y += v.y;
    z += v.z;
}

void uint32_t3::operator -= ( uint32_t3 const &v ) noexcept
{
    x -= v.x;
    y -= v.y;
    z -= v.z;
}

void uint32_t3::operator *= ( uint32_t3 const &v ) noexcept
{
    x *= v.x;
    y *= v.y;
    z *= v.z;
}

void uint32_t3::operator /= ( uint32_t3 const &v ) noexcept
{
    x /= v.x;
    y /= v.y;
    z /= v.z;
}

void uint32_t3::operator >>= ( uint32_t3 const &v ) noexcept
{
    x >>= v.x;
    y >>= v.y;
    z >>= v.z;
}

void uint32_t3::operator <<= ( uint32_t3 const &v ) noexcept
{
    x <<= v.x;
    y <<= v.y;
    z <<= v.z;
}

void uint32_t3::operator &= ( uint32_t3 const &v ) noexcept
{
    x &= v.x;
    y &= v.y;
    z &= v.z;
}

void uint32_t3::operator |= ( uint32_t3 const &v ) noexcept
{
    x |= v.x;
    y |= v.y;
    z |= v.z;
}

void uint32_t3::InitSwizzle () noexcept
{
    xy = swizzle_uint32_t2 ( &x, &y );
    yz = swizzle_uint32_t2 ( &y, &z );

    xxx = swizzle_uint32_t3 ( &x, &x, &x );
    yyy = swizzle_uint32_t3 ( &y, &y, &y );
    zzz = swizzle_uint32_t3 ( &z, &z, &z );
}

uint32_t3 operator + ( uint32_t3 const &a, uint32_t3 const &b ) noexcept
{
    return { a.x + b.x, a.y + b.y, a.z + b.z };
}

uint32_t3 operator - ( uint32_t3 const &a, uint32_t3 const &b ) noexcept
{
    return { a.x - b.x, a.y - b.y, a.z - b.z };
}

uint32_t3 operator * ( uint32_t3 const &a, uint32_t3 const &b ) noexcept
{
    return { a.x * b.x, a.y * b.y, a.z * b.z };
}

uint32_t3 operator / ( uint32_t3 const &a, uint32_t3 const &b ) noexcept
{
    return { a.x / b.x, a.y / b.y, a.z / b.z };
}

uint32_t3 operator >> ( uint32_t3 const &a, uint32_t3 const &b ) noexcept
{
    return { a.x >> b.x, a.y >> b.y, a.z >> b.z };
}

uint32_t3 operator << ( uint32_t3 const &a, uint32_t3 const &b ) noexcept
{
    return { a.x << b.x, a.y << b.y, a.z << b.z };
}

uint32_t3 operator & ( uint32_t3 const &a, uint32_t3 const &b ) noexcept
{
    return { a.x & b.x, a.y & b.y, a.z & b.z };
}

uint32_t3 operator | ( uint32_t3 const &a, uint32_t3 const &b ) noexcept
{
    return { a.x | b.x, a.y | b.y, a.z | b.z };
}

//----------------------------------------------------------------------------------------------------------------------

uint32_t4::uint32_t4 () noexcept
{
    InitSwizzle ();
}

uint32_t4::uint32_t4 ( uint32_t4 const &other ) noexcept:
    x ( other.x ),
    y ( other.y ),
    z ( other.z ),
    w ( other.w )
{
    InitSwizzle ();
}

uint32_t4 &uint32_t4::operator = ( uint32_t4 const &other ) noexcept
{
    x = other.x;
    y = other.y;
    z = other.z;
    w = other.w;
    InitSwizzle ();
    return *this;
}

uint32_t4::uint32_t4 ( uint32_t4 &&other ) noexcept:
    x ( other.x ),
    y ( other.y ),
    z ( other.z ),
    w ( other.w )
{
    InitSwizzle ();
}

uint32_t4 &uint32_t4::operator = ( uint32_t4 &&other ) noexcept
{
    if ( this == &other ) [[unlikely]]
        return *this;

    x = other.x;
    y = other.y;
    z = other.z;
    w = other.w;
    InitSwizzle ();
    return *this;
}

uint32_t4::uint32_t4 ( uint32_t v ) noexcept:
    x ( v ),
    y ( v ),
    z ( v ),
    w ( v )
{
    InitSwizzle ();
}

uint32_t4::uint32_t4 ( uint32_t xVal, uint32_t yVal, uint32_t zVal, uint32_t wVal ) noexcept:
    x ( xVal ),
    y ( yVal ),
    z ( zVal ),
    w ( wVal )
{
    InitSwizzle ();
}

uint32_t4::uint32_t4 ( uint32_t xVal, uint32_t yVal, uint32_t2 const &zwVal ) noexcept:
    x ( xVal ),
    y ( yVal ),
    z ( zwVal.x ),
    w ( zwVal.y )
{
    InitSwizzle ();
}

uint32_t4::uint32_t4 ( uint32_t2 const &xyVal, uint32_t zVal, uint32_t wVal ) noexcept:
    x ( xyVal.x ),
    y ( xyVal.y ),
    z ( zVal ),
    w ( wVal )
{
    InitSwizzle ();
}

uint32_t4::uint32_t4 ( uint32_t2 const &xyVal, uint32_t2 const &zwVal ) noexcept:
    x ( xyVal.x ),
    y ( xyVal.y ),
    z ( zwVal.x ),
    w ( zwVal.y )
{
    InitSwizzle ();
}

uint32_t4::uint32_t4 ( uint32_t3 const &xyzVal, uint32_t wVal ) noexcept:
    x ( xyzVal.x ),
    y ( xyzVal.y ),
    z ( xyzVal.z ),
    w ( wVal )
{
    InitSwizzle ();
}

void uint32_t4::operator += ( uint32_t4 const &v ) noexcept
{
    x += v.x;
    y += v.y;
    z += v.z;
    w += v.w;
}

void uint32_t4::operator -= ( uint32_t4 const &v ) noexcept
{
    x -= v.x;
    y -= v.y;
    z -= v.z;
    w -= v.w;
}

void uint32_t4::operator *= ( uint32_t4 const &v ) noexcept
{
    x *= v.x;
    y *= v.y;
    z *= v.z;
    w *= v.w;
}

void uint32_t4::operator /= ( uint32_t4 const &v ) noexcept
{
    x /= v.x;
    y /= v.y;
    z /= v.z;
    w /= v.w;
}

void uint32_t4::operator >>= ( uint32_t4 const &v ) noexcept
{
    x >>= v.x;
    y >>= v.y;
    z >>= v.z;
    w >>= v.w;
}

void uint32_t4::operator <<= ( uint32_t4 const &v ) noexcept
{
    x <<= v.x;
    y <<= v.y;
    z <<= v.z;
    w <<= v.w;
}

void uint32_t4::operator &= ( uint32_t4 const &v ) noexcept
{
    x &= v.x;
    y &= v.y;
    z &= v.z;
    w &= v.w;
}

void uint32_t4::operator |= ( uint32_t4 const &v ) noexcept
{
    x |= v.x;
    y |= v.y;
    z |= v.z;
    w |= v.w;
}

void uint32_t4::InitSwizzle () noexcept
{
    xy = swizzle_uint32_t2 ( &x, &y );
    xz = swizzle_uint32_t2 ( &x, &z );
    zx = swizzle_uint32_t2 ( &z, &x );
    xxx = swizzle_uint32_t3 ( &x, &x, &x );
    yyy = swizzle_uint32_t3 ( &y, &y, &y );
    zzz = swizzle_uint32_t3 ( &z, &z, &z );
    www = swizzle_uint32_t3 ( &w, &w, &w );
}

uint32_t4 operator + ( uint32_t4 const &a, uint32_t4 const &b ) noexcept
{
    return { a.x + b.x, a.y + b.y, a.z + b.z, a.w + b.w };
}

uint32_t4 operator - ( uint32_t4 const &a, uint32_t4 const &b ) noexcept
{
    return { a.x - b.x, a.y - b.y, a.z - b.z, a.w - b.w };
}

uint32_t4 operator * ( uint32_t4 const &a, uint32_t4 const &b ) noexcept
{
    return { a.x * b.x, a.y * b.y, a.z * b.z, a.w * b.w };
}

uint32_t4 operator / ( uint32_t4 const &a, uint32_t4 const &b ) noexcept
{
    return { a.x / b.x, a.y / b.y, a.z / b.z, a.w / b.w };
}

uint32_t4 operator >> ( uint32_t4 const &a, uint32_t4 const &b ) noexcept
{
    return { a.x >> b.x, a.y >> b.y, a.z >> b.z, a.w >> b.w };
}

uint32_t4 operator << ( uint32_t4 const &a, uint32_t4 const &b ) noexcept
{
    return { a.x << b.x, a.y << b.y, a.z << b.z, a.w << b.w };
}

uint32_t4 operator & ( uint32_t4 const &a, uint32_t4 const &b ) noexcept
{
    return { a.x & b.x, a.y & b.y, a.z & b.z, a.w & b.w };
}

uint32_t4 operator | ( uint32_t4 const &a, uint32_t4 const &b ) noexcept
{
    return { a.x | b.x, a.y | b.y, a.z | b.z, a.w | b.w };
}

//----------------------------------------------------------------------------------------------------------------------

swizzle_float32_t3::operator float32_t3 () const noexcept
{
    return { *x, *y, *z };
}

void swizzle_float32_t3::operator += ( float32_t3 const &v ) noexcept
{
    *x += v.x;
    *y += v.y;
    *z += v.z;
}

void swizzle_float32_t3::operator -= ( float32_t3 const &v ) noexcept
{
    *x -= v.x;
    *y -= v.y;
    *z -= v.z;
}

void swizzle_float32_t3::operator *= ( float32_t3 const &v ) noexcept
{
    *x *= v.x;
    *y *= v.y;
    *z *= v.z;
}

void swizzle_float32_t3::operator /= ( float32_t3 const &v ) noexcept
{
    *x /= v.x;
    *y /= v.y;
    *x /= v.x;
}

void swizzle_float32_t3::operator += ( swizzle_float32_t3 const &v ) noexcept
{
    *x += *v.x;
    *y += *v.y;
    *z += *v.z;
}

void swizzle_float32_t3::operator -= ( swizzle_float32_t3 const &v ) noexcept
{
    *x -= *v.x;
    *y -= *v.y;
    *z -= *v.z;
}

void swizzle_float32_t3::operator *= ( swizzle_float32_t3 const &v ) noexcept
{
    *x *= *v.x;
    *y *= *v.y;
    *z *= *v.z;
}

void swizzle_float32_t3::operator /= ( swizzle_float32_t3 const &v ) noexcept
{
    *x /= *v.x;
    *y /= *v.y;
    *z /= *v.z;
}

//----------------------------------------------------------------------------------------------------------------------

swizzle_float32_t4::operator float32_t4 () const noexcept
{
    return { *x, *y, *z, *w };
}

void swizzle_float32_t4::operator += ( float32_t4 const &v ) noexcept
{
    *x += v.x;
    *y += v.y;
    *z += v.z;
    *w += v.w;
}

void swizzle_float32_t4::operator -= ( float32_t4 const &v ) noexcept
{
    *x -= v.x;
    *y -= v.y;
    *z -= v.z;
    *w -= v.w;
}

void swizzle_float32_t4::operator *= ( float32_t4 const &v ) noexcept
{
    *x *= v.x;
    *y *= v.y;
    *z *= v.z;
    *w *= v.w;
}

void swizzle_float32_t4::operator /= ( float32_t4 const &v ) noexcept
{
    *x /= v.x;
    *y /= v.y;
    *x /= v.x;
    *w /= v.w;
}

void swizzle_float32_t4::operator += ( swizzle_float32_t4 const &v ) noexcept
{
    *x += *v.x;
    *y += *v.y;
    *z += *v.z;
    *w += *v.w;
}

void swizzle_float32_t4::operator -= ( swizzle_float32_t4 const &v ) noexcept
{
    *x -= *v.x;
    *y -= *v.y;
    *z -= *v.z;
    *w -= *v.w;
}

void swizzle_float32_t4::operator *= ( swizzle_float32_t4 const &v ) noexcept
{
    *x *= *v.x;
    *y *= *v.y;
    *z *= *v.z;
    *w *= *v.w;
}

void swizzle_float32_t4::operator /= ( swizzle_float32_t4 const &v ) noexcept
{
    *x /= *v.x;
    *y /= *v.y;
    *z /= *v.z;
    *w /= *v.w;
}

//----------------------------------------------------------------------------------------------------------------------

float32_t3::float32_t3 () noexcept
{
    InitSwizzle ();
}

float32_t3::float32_t3 ( float32_t3 const &other ) noexcept:
    x ( other.x ),
    y ( other.y ),
    z ( other.z )
{
    InitSwizzle ();
}

float32_t3 &float32_t3::operator = ( float32_t3 const &other ) noexcept
{
    x = other.x;
    y = other.y;
    z = other.z;
    InitSwizzle ();
    return *this;
}

float32_t3::float32_t3 ( float32_t3 &&other ) noexcept:
    x ( other.x ),
    y ( other.y ),
    z ( other.z )
{
    InitSwizzle ();
}

float32_t3 &float32_t3::operator = ( float32_t3 &&other ) noexcept
{
    if ( this == &other ) [[unlikely]]
        return *this;

    x = other.x;
    y = other.y;
    z = other.z;
    InitSwizzle ();
    return *this;
}

float32_t3::float32_t3 ( float v ) noexcept:
    x ( v ),
    y ( v ),
    z ( v )
{
    InitSwizzle ();
}

float32_t3::float32_t3 ( float32_t xVal, float32_t yVal, float32_t zVal ) noexcept:
    x ( xVal ),
    y ( yVal ),
    z ( zVal )
{
    InitSwizzle ();
}

float32_t3::float32_t3 ( uint32_t3 v ) noexcept:
    x ( static_cast<float32_t> ( v.x ) ),
    y ( static_cast<float32_t> ( v.y ) ),
    z ( static_cast<float32_t> ( v.z ) )
{
    InitSwizzle ();
}

void float32_t3::operator += ( float32_t3 const &v ) noexcept
{
    x += v.x;
    y += v.y;
    z += v.z;
}

void float32_t3::operator -= ( float32_t3 const &v ) noexcept
{
    x -= v.x;
    y -= v.y;
    z -= v.z;
}

void float32_t3::operator *= ( float32_t3 const &v ) noexcept
{
    x *= v.x;
    y *= v.y;
    z *= v.z;
}

void float32_t3::operator /= ( float32_t3 const &v ) noexcept
{
    x /= v.x;
    y /= v.y;
    z /= v.z;
}

void float32_t3::InitSwizzle () noexcept
{
    xxx = swizzle_float32_t3 ( &x, &x, &x );
    yyy = swizzle_float32_t3 ( &y, &y, &y );
    zzz = swizzle_float32_t3 ( &z, &z, &z );
}

float32_t3 operator + ( float32_t3 const &a, float32_t3 const &b ) noexcept
{
    return { a.x + b.x, a.y + b.y, a.z + b.z };
}

float32_t3 operator - ( float32_t3 const &a, float32_t3 const &b ) noexcept
{
    return { a.x - b.x, a.y - b.y, a.z - b.z };
}

float32_t3 operator * ( float32_t3 const &a, float32_t3 const &b ) noexcept
{
    return { a.x * b.x, a.y * b.y, a.z * b.z };
}

float32_t3 operator / ( float32_t3 const &a, float32_t3 const &b ) noexcept
{
    return { a.x / b.x, a.y / b.y, a.z / b.z };
}

float32_t dot ( float32_t3 const& a, float32_t3 const& b ) noexcept
{
    return a.x * b.x + a.y * b.y + a.z * b.z;
}

float32_t3 mad ( float32_t3 const &a, float32_t3 const &b, float32_t c ) noexcept
{
    return
    {
        a.x * b.x + c,
        a.y * b.y + c,
        a.z * b.z + c
    };
}

float32_t3 mad ( float32_t3 const &a, float32_t3 const &b, float32_t3 const &c ) noexcept
{
    return
    {
        a.x * b.x + c.x,
        a.y * b.y + c.y,
        a.z * b.z + c.z
    };
}

//----------------------------------------------------------------------------------------------------------------------

float32_t4::float32_t4 () noexcept
{
    InitSwizzle ();
}

float32_t4::float32_t4 ( float32_t4 const &other ) noexcept:
    x ( other.x ),
    y ( other.y ),
    z ( other.z ),
    w ( other.w )
{
    InitSwizzle ();
}

float32_t4 &float32_t4::operator = ( float32_t4 const &other ) noexcept
{
    x = other.x;
    y = other.y;
    z = other.z;
    w = other.w;
    InitSwizzle ();
    return *this;
}

float32_t4::float32_t4 ( float32_t4 &&other ) noexcept:
    x ( other.x ),
    y ( other.y ),
    z ( other.z ),
    w ( other.w )
{
    InitSwizzle ();
}

float32_t4 &float32_t4::operator = ( float32_t4 &&other ) noexcept
{
    if ( this == &other ) [[unlikely]]
        return *this;

    x = other.x;
    y = other.y;
    z = other.z;
    w = other.w;
    InitSwizzle ();
    return *this;
}

float32_t4::float32_t4 ( float32_t v ) noexcept:
    x ( v ),
    y ( v ),
    z ( v ),
    w ( v )
{
    InitSwizzle ();
}

float32_t4::float32_t4 ( float32_t xVal, float32_t yVal, float32_t zVal, float32_t wVal ) noexcept:
    x ( xVal ),
    y ( yVal ),
    z ( zVal ),
    w ( wVal )
{
    InitSwizzle ();
}
float32_t4::float32_t4 ( float32_t3 const &xyzVal, float32_t wVal ) noexcept:
    x ( xyzVal.x ),
    y ( xyzVal.y ),
    z ( xyzVal.z ),
    w ( wVal )
{
    InitSwizzle ();
}

float32_t4::float32_t4 ( float32_t x, float32_t3 const &yzwVal ) noexcept:
    x ( x ),
    y ( yzwVal.x ),
    z ( yzwVal.y ),
    w ( yzwVal.z )
{
    InitSwizzle ();
}

void float32_t4::operator += ( float32_t4 const &v ) noexcept
{
    x += v.x;
    y += v.y;
    z += v.z;
    w += v.w;
}

void float32_t4::operator -= ( float32_t4 const &v ) noexcept
{
    x -= v.x;
    y -= v.y;
    z -= v.z;
    w -= v.w;
}

void float32_t4::operator *= ( float32_t4 const &v ) noexcept
{
    x *= v.x;
    y *= v.y;
    z *= v.z;
    w *= v.w;
}

void float32_t4::operator /= ( float32_t4 const &v ) noexcept
{
    x /= v.x;
    y /= v.y;
    z /= v.z;
    w /= v.w;
}

void float32_t4::InitSwizzle () noexcept
{
    xxx = swizzle_float32_t3 ( &x, &x, &x );
    yyy = swizzle_float32_t3 ( &y, &y, &y );
    zzz = swizzle_float32_t3 ( &z, &z, &z );
    www = swizzle_float32_t3 ( &w, &w, &w );
}

float32_t4 operator + ( float32_t4 const &a, float32_t4 const &b ) noexcept
{
    return { a.x + b.x, a.y + b.y, a.z + b.z, a.w + b.w };
}

float32_t4 operator - ( float32_t4 const &a, float32_t4 const &b ) noexcept
{
    return { a.x - b.x, a.y - b.y, a.z - b.z, a.w - b.w };
}

float32_t4 operator * ( float32_t4 const &a, float32_t4 const &b ) noexcept
{
    return { a.x * b.x, a.y * b.y, a.z * b.z, a.w * b.w };
}

float32_t4 operator / ( float32_t4 const &a, float32_t4 const &b ) noexcept
{
    return { a.x / b.x, a.y / b.y, a.z / b.z, a.w / b.w };
}

} // namespace hlsl
