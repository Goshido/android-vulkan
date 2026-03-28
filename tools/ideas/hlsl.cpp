#include <precompiled_headers.hpp>
#include "hlsl.hpp"


namespace hlsl {

uint32_t2 &uint32_t2::operator += ( uint32_t2 const &v ) noexcept
{
    x += v.x;
    y += v.y;
    return *this;
}

uint32_t2 &uint32_t2::operator -= ( uint32_t2 const &v ) noexcept
{
    x -= v.x;
    y -= v.y;
    return *this;
}

uint32_t2 &uint32_t2::operator *= ( uint32_t2 const &v ) noexcept
{
    x *= v.x;
    y *= v.y;
    return *this;
}

uint32_t2 &uint32_t2::operator /= ( uint32_t2 const &v ) noexcept
{
    x /= v.x;
    y /= v.y;
    return *this;
}

uint32_t2 &uint32_t2::operator >>= ( uint32_t2 const &v ) noexcept
{
    x >>= v.x;
    y >>= v.y;
    return *this;
}

uint32_t2 &uint32_t2::operator <<= ( uint32_t2 const &v ) noexcept
{
    x <<= v.x;
    y <<= v.y;
    return *this;
}

uint32_t2 &uint32_t2::operator &= ( uint32_t2 const &v ) noexcept
{
    x &= v.x;
    y &= v.y;
    return *this;
}

uint32_t2 &uint32_t2::operator |= ( uint32_t2 const &v ) noexcept
{
    x |= v.x;
    y |= v.y;
    return *this;
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

//----------------------------------------------------------------------------------------------------------------------

uint32_t3 &uint32_t3::operator += ( uint32_t3 const &v ) noexcept
{
    x += v.x;
    y += v.y;
    z += v.z;
    return *this;
}

uint32_t3 &uint32_t3::operator -= ( uint32_t3 const &v ) noexcept
{
    x -= v.x;
    y -= v.y;
    z -= v.z;
    return *this;
}

uint32_t3 &uint32_t3::operator *= ( uint32_t3 const &v ) noexcept
{
    x *= v.x;
    y *= v.y;
    z *= v.z;
    return *this;
}

uint32_t3 &uint32_t3::operator /= ( uint32_t3 const &v ) noexcept
{
    x /= v.x;
    y /= v.y;
    z /= v.z;
    return *this;
}

uint32_t3 &uint32_t3::operator >>= ( uint32_t3 const &v ) noexcept
{
    x >>= v.x;
    y >>= v.y;
    z >>= v.z;
    return *this;
}

uint32_t3 &uint32_t3::operator <<= ( uint32_t3 const &v ) noexcept
{
    x <<= v.x;
    y <<= v.y;
    z <<= v.z;
    return *this;
}

uint32_t3 &uint32_t3::operator &= ( uint32_t3 const &v ) noexcept
{
    x &= v.x;
    y &= v.y;
    z &= v.z;
    return *this;
}

uint32_t3 &uint32_t3::operator |= ( uint32_t3 const &v ) noexcept
{
    x |= v.x;
    y |= v.y;
    z |= v.z;
    return *this;
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

uint32_t4 &uint32_t4::operator += ( uint32_t4 const &v ) noexcept
{
    x += v.x;
    y += v.y;
    z += v.z;
    w += v.w;
    return *this;
}

uint32_t4 &uint32_t4::operator -= ( uint32_t4 const &v ) noexcept
{
    x -= v.x;
    y -= v.y;
    z -= v.z;
    w -= v.w;
    return *this;
}

uint32_t4 &uint32_t4::operator *= ( uint32_t4 const &v ) noexcept
{
    x *= v.x;
    y *= v.y;
    z *= v.z;
    w *= v.w;
    return *this;
}

uint32_t4 &uint32_t4::operator /= ( uint32_t4 const &v ) noexcept
{
    x /= v.x;
    y /= v.y;
    z /= v.z;
    w /= v.w;
    return *this;
}

uint32_t4 &uint32_t4::operator >>= ( uint32_t4 const &v ) noexcept
{
    x >>= v.x;
    y >>= v.y;
    z >>= v.z;
    w >>= v.w;
    return *this;
}

uint32_t4 &uint32_t4::operator <<= ( uint32_t4 const &v ) noexcept
{
    x <<= v.x;
    y <<= v.y;
    z <<= v.z;
    w <<= v.w;
    return *this;
}

uint32_t4 &uint32_t4::operator &= ( uint32_t4 const &v ) noexcept
{
    x &= v.x;
    y &= v.y;
    z &= v.z;
    w &= v.w;
    return *this;
}

uint32_t4 &uint32_t4::operator |= ( uint32_t4 const &v ) noexcept
{
    x |= v.x;
    y |= v.y;
    z |= v.z;
    w |= v.w;
    return *this;
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

} // namespace hlsl
