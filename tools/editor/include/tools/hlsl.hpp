#ifndef IDEAS_HLSL_HPP
#define IDEAS_HLSL_HPP


#include <GXCommon/GXWarning.hpp>

GX_DISABLE_COMMON_WARNINGS

#include <cmath>

GX_RESTORE_WARNING_STATE


#define in

namespace hlsl {

using std::sqrt;
using std::abs;
using float32_t = float;

class uint32_t2;
class uint32_t3;
class uint32_t4;
class uint64_t2;
class uint64_t3;
class uint64_t4;
class float32_t3;
class float32_t4;
class swizzle_uint32_t2;
class swizzle_uint32_t3;
class swizzle_uint32_t4;
class swizzle_uint64_t2;
class swizzle_uint64_t3;
class swizzle_uint64_t4;
class swizzle_float32_t3;
class swizzle_float32_t4;

//----------------------------------------------------------------------------------------------------------------------

class swizzle_uint32_t2 final
{
    friend class uint32_t2;
    friend class uint32_t3;
    friend class uint32_t4;
    friend class swizzle_uint64_t2;

    private:
        uint32_t*       x = nullptr;
        uint32_t*       y = nullptr;

    public:
        [[maybe_unused, nodiscard]] operator uint32_t2 () const noexcept;

        [[maybe_unused]] void operator += ( uint32_t2 const &v ) noexcept;
        [[maybe_unused]] void operator -= ( uint32_t2 const &v ) noexcept;
        [[maybe_unused]] void operator *= ( uint32_t2 const &v ) noexcept;
        [[maybe_unused]] void operator /= ( uint32_t2 const &v ) noexcept;
        [[maybe_unused]] void operator >>= ( uint32_t2 const &v ) noexcept;
        [[maybe_unused]] void operator >>= ( uint32_t v ) noexcept;
        [[maybe_unused]] void operator <<= ( uint32_t2 const &v ) noexcept;
        [[maybe_unused]] void operator <<= ( uint32_t v ) noexcept;
        [[maybe_unused]] void operator &= ( uint32_t2 const &v ) noexcept;
        [[maybe_unused]] void operator |= ( uint32_t2 const &v ) noexcept;

        [[maybe_unused]] void operator += ( swizzle_uint32_t2 const &v ) noexcept;
        [[maybe_unused]] void operator -= ( swizzle_uint32_t2 const &v ) noexcept;
        [[maybe_unused]] void operator *= ( swizzle_uint32_t2 const &v ) noexcept;
        [[maybe_unused]] void operator /= ( swizzle_uint32_t2 const &v ) noexcept;
        [[maybe_unused]] void operator >>= ( swizzle_uint32_t2 const &v ) noexcept;
        [[maybe_unused]] void operator <<= ( swizzle_uint32_t2 const &v ) noexcept;
        [[maybe_unused]] void operator &= ( swizzle_uint32_t2 const &v ) noexcept;
        [[maybe_unused]] void operator |= ( swizzle_uint32_t2 const &v ) noexcept;

    private:
        [[maybe_unused]] swizzle_uint32_t2 () = default;

        [[maybe_unused]] swizzle_uint32_t2 ( swizzle_uint32_t2 const & ) = default;
        [[maybe_unused]] swizzle_uint32_t2 &operator = ( swizzle_uint32_t2 const & ) = default;

        [[maybe_unused]] swizzle_uint32_t2 ( swizzle_uint32_t2 && ) = default;
        [[maybe_unused]] swizzle_uint32_t2 &operator = ( swizzle_uint32_t2 && ) = default;

        [[maybe_unused]] constexpr swizzle_uint32_t2 ( uint32_t* xRef, uint32_t* yRef ) noexcept:
            x ( xRef ),
            y ( yRef )
        {
            // NOTHING
        }

        [[maybe_unused]] ~swizzle_uint32_t2 () = default;
};

//----------------------------------------------------------------------------------------------------------------------

class swizzle_uint32_t3 final
{
    friend class uint32_t2;
    friend class uint32_t3;
    friend class uint32_t4;
    friend class swizzle_uint64_t3;

    private:
        uint32_t*       x = nullptr;
        uint32_t*       y = nullptr;
        uint32_t*       z = nullptr;

    public:
        [[maybe_unused]] ~swizzle_uint32_t3 () = default;

        [[maybe_unused, nodiscard]] operator uint32_t3 () const noexcept;

        [[maybe_unused]] void operator += ( uint32_t3 const &v ) noexcept;
        [[maybe_unused]] void operator -= ( uint32_t3 const &v ) noexcept;
        [[maybe_unused]] void operator *= ( uint32_t3 const &v ) noexcept;
        [[maybe_unused]] void operator /= ( uint32_t3 const &v ) noexcept;
        [[maybe_unused]] void operator >>= ( uint32_t3 const &v ) noexcept;
        [[maybe_unused]] void operator <<= ( uint32_t3 const &v ) noexcept;
        [[maybe_unused]] void operator &= ( uint32_t3 const &v ) noexcept;
        [[maybe_unused]] void operator |= ( uint32_t3 const &v ) noexcept;

        [[maybe_unused]] void operator += ( swizzle_uint32_t3 const &v ) noexcept;
        [[maybe_unused]] void operator -= ( swizzle_uint32_t3 const &v ) noexcept;
        [[maybe_unused]] void operator *= ( swizzle_uint32_t3 const &v ) noexcept;
        [[maybe_unused]] void operator /= ( swizzle_uint32_t3 const &v ) noexcept;
        [[maybe_unused]] void operator >>= ( swizzle_uint32_t3 const &v ) noexcept;
        [[maybe_unused]] void operator <<= ( swizzle_uint32_t3 const &v ) noexcept;
        [[maybe_unused]] void operator &= ( swizzle_uint32_t3 const &v ) noexcept;
        [[maybe_unused]] void operator |= ( swizzle_uint32_t3 const &v ) noexcept;

    private:
        [[maybe_unused]] swizzle_uint32_t3 () = default;

        [[maybe_unused]] swizzle_uint32_t3 ( swizzle_uint32_t3 const & ) = default;
        [[maybe_unused]] swizzle_uint32_t3 &operator = ( swizzle_uint32_t3 const & ) = default;

        [[maybe_unused]] swizzle_uint32_t3 ( swizzle_uint32_t3 && ) = default;
        [[maybe_unused]] swizzle_uint32_t3 &operator = ( swizzle_uint32_t3 && ) = default;

        [[maybe_unused]] constexpr swizzle_uint32_t3 ( uint32_t* xRef, uint32_t* yRef, uint32_t* zRef ) noexcept:
            x ( xRef ),
            y ( yRef ),
            z ( zRef )
        {
            // NOTHING
        }
};

//----------------------------------------------------------------------------------------------------------------------

class swizzle_uint32_t4 final
{
    friend class uint32_t2;
    friend class uint32_t3;
    friend class uint32_t4;

    private:
        uint32_t*       x = nullptr;
        uint32_t*       y = nullptr;
        uint32_t*       z = nullptr;
        uint32_t*       w = nullptr;

    public:
        [[maybe_unused]] ~swizzle_uint32_t4 () = default;

        [[maybe_unused, nodiscard]] operator uint32_t4 () const noexcept;

        [[maybe_unused]] void operator += ( uint32_t4 const &v ) noexcept;
        [[maybe_unused]] void operator -= ( uint32_t4 const &v ) noexcept;
        [[maybe_unused]] void operator *= ( uint32_t4 const &v ) noexcept;
        [[maybe_unused]] void operator /= ( uint32_t4 const &v ) noexcept;
        [[maybe_unused]] void operator >>= ( uint32_t4 const &v ) noexcept;
        [[maybe_unused]] void operator <<= ( uint32_t4 const &v ) noexcept;
        [[maybe_unused]] void operator &= ( uint32_t4 const &v ) noexcept;
        [[maybe_unused]] void operator |= ( uint32_t4 const &v ) noexcept;

        [[maybe_unused]] void operator += ( swizzle_uint32_t4 const &v ) noexcept;
        [[maybe_unused]] void operator -= ( swizzle_uint32_t4 const &v ) noexcept;
        [[maybe_unused]] void operator *= ( swizzle_uint32_t4 const &v ) noexcept;
        [[maybe_unused]] void operator /= ( swizzle_uint32_t4 const &v ) noexcept;
        [[maybe_unused]] void operator >>= ( swizzle_uint32_t4 const &v ) noexcept;
        [[maybe_unused]] void operator <<= ( swizzle_uint32_t4 const &v ) noexcept;
        [[maybe_unused]] void operator &= ( swizzle_uint32_t4 const &v ) noexcept;
        [[maybe_unused]] void operator |= ( swizzle_uint32_t4 const &v ) noexcept;

    private:
        [[maybe_unused]] swizzle_uint32_t4 () = default;

        [[maybe_unused]] swizzle_uint32_t4 ( swizzle_uint32_t4 const & ) = default;
        [[maybe_unused]] swizzle_uint32_t4 &operator = ( swizzle_uint32_t4 const & ) = default;

        [[maybe_unused]] swizzle_uint32_t4 ( swizzle_uint32_t4 && ) = default;
        [[maybe_unused]] swizzle_uint32_t4 &operator = ( swizzle_uint32_t4 && ) = default;

        [[maybe_unused]] constexpr swizzle_uint32_t4 ( uint32_t* xRef,
            uint32_t* yRef,
            uint32_t* zRef,
            uint32_t* wRef
        ) noexcept:
            x ( xRef ),
            y ( yRef ),
            z ( zRef ),
            w ( wRef )
        {
            // NOTHING
        }
};

//----------------------------------------------------------------------------------------------------------------------

class uint32_t2 final
{
    public:
        uint32_t                x;
        uint32_t                y;

        swizzle_uint32_t2       xx;
        swizzle_uint32_t2       yx;
        swizzle_uint32_t2       yy;

        swizzle_uint32_t3       xxx;
        swizzle_uint32_t3       yyy;

    public:
        [[maybe_unused]] uint32_t2 () noexcept;

        [[maybe_unused]] uint32_t2 ( uint32_t2 const &other ) noexcept;
        [[maybe_unused]] uint32_t2 &operator = ( uint32_t2 const &other ) noexcept;

        [[maybe_unused]] uint32_t2 ( uint32_t2 &&other ) noexcept;
        [[maybe_unused]] uint32_t2 &operator = ( uint32_t2 &&other ) noexcept;

        [[maybe_unused]] uint32_t2 ( uint32_t v ) noexcept;
        [[maybe_unused]] uint32_t2 ( uint32_t xVal, uint32_t yVal ) noexcept;

        [[maybe_unused]] ~uint32_t2 () = default;

        [[maybe_unused]] void operator += ( uint32_t2 const &v ) noexcept;
        [[maybe_unused]] void operator -= ( uint32_t2 const &v ) noexcept;
        [[maybe_unused]] void operator *= ( uint32_t2 const &v ) noexcept;
        [[maybe_unused]] void operator /= ( uint32_t2 const &v ) noexcept;
        [[maybe_unused]] void operator >>= ( uint32_t2 const &v ) noexcept;
        [[maybe_unused]] void operator >>= ( uint32_t v ) noexcept;
        [[maybe_unused]] void operator <<= ( uint32_t2 const &v ) noexcept;
        [[maybe_unused]] void operator <<= ( uint32_t v ) noexcept;
        [[maybe_unused]] void operator &= ( uint32_t2 const &v ) noexcept;
        [[maybe_unused]] void operator |= ( uint32_t2 const &v ) noexcept;

    private:
        void InitSwizzle () noexcept;
};

[[maybe_unused, nodiscard]] uint32_t2 operator + ( uint32_t2 const &a, uint32_t2 const &b ) noexcept;
[[maybe_unused, nodiscard]] uint32_t2 operator - ( uint32_t2 const &a, uint32_t2 const &b ) noexcept;
[[maybe_unused, nodiscard]] uint32_t2 operator * ( uint32_t2 const &a, uint32_t2 const &b ) noexcept;
[[maybe_unused, nodiscard]] uint32_t2 operator / ( uint32_t2 const &a, uint32_t2 const &b ) noexcept;
[[maybe_unused, nodiscard]] uint32_t2 operator >> ( uint32_t2 const &a, uint32_t2 const &b ) noexcept;
[[maybe_unused, nodiscard]] uint32_t2 operator >> ( uint32_t2 const &a, uint32_t b ) noexcept;
[[maybe_unused, nodiscard]] uint32_t2 operator << ( uint32_t2 const &a, uint32_t2 const &b ) noexcept;
[[maybe_unused, nodiscard]] uint32_t2 operator << ( uint32_t2 const &a, uint32_t b ) noexcept;
[[maybe_unused, nodiscard]] uint32_t2 operator & ( uint32_t2 const &a, uint32_t2 const &b ) noexcept;
[[maybe_unused, nodiscard]] uint32_t2 operator | ( uint32_t2 const &a, uint32_t2 const &b ) noexcept;

//----------------------------------------------------------------------------------------------------------------------

class uint32_t3 final
{
    public:
        uint32_t                x;
        uint32_t                y;
        uint32_t                z;

        swizzle_uint32_t2       xy;
        swizzle_uint32_t2       yz;

        swizzle_uint32_t3       xxx;
        swizzle_uint32_t3       yyy;
        swizzle_uint32_t3       zzz;

    public:
        [[maybe_unused]] uint32_t3 () noexcept;

        [[maybe_unused]] uint32_t3 ( uint32_t3 const& other ) noexcept;
        [[maybe_unused]] uint32_t3 &operator = ( uint32_t3 const &other ) noexcept;

        [[maybe_unused]] uint32_t3 ( uint32_t3 &&other ) noexcept;
        [[maybe_unused]] uint32_t3 &operator = ( uint32_t3 &&other ) noexcept;

        [[maybe_unused]] uint32_t3 ( uint32_t v ) noexcept;
        [[maybe_unused]] uint32_t3 ( uint32_t xVal, uint32_t yVal, uint32_t zVal ) noexcept;
        [[maybe_unused]] uint32_t3 ( uint32_t xVal, uint32_t2 const &yzVal ) noexcept;
        [[maybe_unused]] uint32_t3 ( uint32_t2 const &xyVal, uint32_t zVal ) noexcept;

        [[maybe_unused]] ~uint32_t3 () = default;

        [[maybe_unused]] void operator += ( uint32_t3 const &v ) noexcept;
        [[maybe_unused]] void operator -= ( uint32_t3 const &v ) noexcept;
        [[maybe_unused]] void operator *= ( uint32_t3 const &v ) noexcept;
        [[maybe_unused]] void operator /= ( uint32_t3 const &v ) noexcept;
        [[maybe_unused]] void operator >>= ( uint32_t3 const &v ) noexcept;
        [[maybe_unused]] void operator <<= ( uint32_t3 const &v ) noexcept;
        [[maybe_unused]] void operator &= ( uint32_t3 const &v ) noexcept;
        [[maybe_unused]] void operator |= ( uint32_t3 const &v ) noexcept;

    private:
        void InitSwizzle () noexcept;
};

[[maybe_unused, nodiscard]] uint32_t3 operator + ( uint32_t3 const &a, uint32_t3 const &b ) noexcept;
[[maybe_unused, nodiscard]] uint32_t3 operator - ( uint32_t3 const &a, uint32_t3 const &b ) noexcept;
[[maybe_unused, nodiscard]] uint32_t3 operator * ( uint32_t3 const &a, uint32_t3 const &b ) noexcept;
[[maybe_unused, nodiscard]] uint32_t3 operator / ( uint32_t3 const &a, uint32_t3 const &b ) noexcept;
[[maybe_unused, nodiscard]] uint32_t3 operator >> ( uint32_t3 const &a, uint32_t3 const &b ) noexcept;
[[maybe_unused, nodiscard]] uint32_t3 operator << ( uint32_t3 const &a, uint32_t3 const &b ) noexcept;
[[maybe_unused, nodiscard]] uint32_t3 operator & ( uint32_t3 const &a, uint32_t3 const &b ) noexcept;
[[maybe_unused, nodiscard]] uint32_t3 operator | ( uint32_t3 const &a, uint32_t3 const &b ) noexcept;

//----------------------------------------------------------------------------------------------------------------------

class uint32_t4 final
{
    public:
        uint32_t                x;
        uint32_t                y;
        uint32_t                z;
        uint32_t                w;

        swizzle_uint32_t2       xy;
        swizzle_uint32_t2       xz;
        swizzle_uint32_t2       zx;

        swizzle_uint32_t3       xxx;
        swizzle_uint32_t3       yyy;
        swizzle_uint32_t3       zzz;
        swizzle_uint32_t3       www;

    public:
        [[maybe_unused]] uint32_t4 () noexcept;

        [[maybe_unused]] uint32_t4 ( uint32_t4 const &other ) noexcept;
        [[maybe_unused]] uint32_t4 &operator = ( uint32_t4 const &other ) noexcept;

        [[maybe_unused]] uint32_t4 ( uint32_t4 &&other ) noexcept;
        [[maybe_unused]] uint32_t4 &operator = ( uint32_t4 &&other ) noexcept;

        [[maybe_unused]] uint32_t4 ( uint32_t v ) noexcept;
        [[maybe_unused]] uint32_t4 ( uint64_t4 const &v ) noexcept;
        [[maybe_unused]] uint32_t4 ( uint32_t xVal, uint32_t yVal, uint32_t zVal, uint32_t wVal ) noexcept;
        [[maybe_unused]] uint32_t4 ( uint32_t xVal, uint32_t yVal, uint32_t2 const &zw ) noexcept;
        [[maybe_unused]] uint32_t4 ( uint32_t2 const &xyVal, uint32_t zVal, uint32_t wVal ) noexcept;
        [[maybe_unused]] uint32_t4 ( uint32_t2 const &xyVal, uint32_t2 const &zwVal ) noexcept;
        [[maybe_unused]] uint32_t4 ( uint32_t3 const &xyzVal, uint32_t wVal ) noexcept;

        [[maybe_unused]] ~uint32_t4 () = default;

        [[maybe_unused]] void operator += ( uint32_t4 const &v ) noexcept;
        [[maybe_unused]] void operator -= ( uint32_t4 const &v ) noexcept;
        [[maybe_unused]] void operator *= ( uint32_t4 const &v ) noexcept;
        [[maybe_unused]] void operator /= ( uint32_t4 const &v ) noexcept;
        [[maybe_unused]] void operator >>= ( uint32_t4 const &v ) noexcept;
        [[maybe_unused]] void operator <<= ( uint32_t4 const &v ) noexcept;
        [[maybe_unused]] void operator &= ( uint32_t4 const &v ) noexcept;
        [[maybe_unused]] void operator |= ( uint32_t4 const &v ) noexcept;

    private:
        void InitSwizzle () noexcept;
};

[[maybe_unused, nodiscard]] uint32_t4 operator + ( uint32_t4 const &a, uint32_t4 const &b ) noexcept;
[[maybe_unused, nodiscard]] uint32_t4 operator - ( uint32_t4 const &a, uint32_t4 const &b ) noexcept;
[[maybe_unused, nodiscard]] uint32_t4 operator * ( uint32_t4 const &a, uint32_t4 const &b ) noexcept;
[[maybe_unused, nodiscard]] uint32_t4 operator / ( uint32_t4 const &a, uint32_t4 const &b ) noexcept;
[[maybe_unused, nodiscard]] uint32_t4 operator >> ( uint32_t4 const &a, uint32_t4 const &b ) noexcept;
[[maybe_unused, nodiscard]] uint32_t4 operator << ( uint32_t4 const &a, uint32_t4 const &b ) noexcept;
[[maybe_unused, nodiscard]] uint32_t4 operator & ( uint32_t4 const &a, uint32_t4 const &b ) noexcept;
[[maybe_unused, nodiscard]] uint32_t4 operator | ( uint32_t4 const &a, uint32_t4 const &b ) noexcept;

//----------------------------------------------------------------------------------------------------------------------

class swizzle_uint64_t2 final
{
    friend class uint64_t2;
    friend class uint64_t3;
    friend class uint64_t4;

    private:
        uint64_t*       x = nullptr;
        uint64_t*       y = nullptr;

    public:
        [[maybe_unused, nodiscard]] operator uint64_t2 () const noexcept;

        [[maybe_unused]] void operator += ( uint64_t2 const &v ) noexcept;
        [[maybe_unused]] void operator -= ( uint64_t2 const &v ) noexcept;
        [[maybe_unused]] void operator *= ( uint64_t2 const &v ) noexcept;
        [[maybe_unused]] void operator /= ( uint64_t2 const &v ) noexcept;
        [[maybe_unused]] void operator >>= ( uint64_t2 const &v ) noexcept;
        [[maybe_unused]] void operator >>= ( uint64_t v ) noexcept;
        [[maybe_unused]] void operator >>= ( uint32_t2 const &v ) noexcept;
        [[maybe_unused]] void operator >>= ( uint32_t v ) noexcept;
        [[maybe_unused]] void operator <<= ( uint64_t2 const &v ) noexcept;
        [[maybe_unused]] void operator <<= ( uint64_t v ) noexcept;
        [[maybe_unused]] void operator <<= ( uint32_t2 const &v ) noexcept;
        [[maybe_unused]] void operator <<= ( uint32_t v ) noexcept;
        [[maybe_unused]] void operator &= ( uint64_t2 const &v ) noexcept;
        [[maybe_unused]] void operator |= ( uint64_t2 const &v ) noexcept;

        [[maybe_unused]] void operator += ( swizzle_uint64_t2 const &v ) noexcept;
        [[maybe_unused]] void operator -= ( swizzle_uint64_t2 const &v ) noexcept;
        [[maybe_unused]] void operator *= ( swizzle_uint64_t2 const &v ) noexcept;
        [[maybe_unused]] void operator /= ( swizzle_uint64_t2 const &v ) noexcept;
        [[maybe_unused]] void operator >>= ( swizzle_uint64_t2 const &v ) noexcept;
        [[maybe_unused]] void operator >>= ( swizzle_uint32_t2 const &v ) noexcept;
        [[maybe_unused]] void operator <<= ( swizzle_uint64_t2 const &v ) noexcept;
        [[maybe_unused]] void operator <<= ( swizzle_uint32_t2 const &v ) noexcept;
        [[maybe_unused]] void operator &= ( swizzle_uint64_t2 const &v ) noexcept;
        [[maybe_unused]] void operator |= ( swizzle_uint64_t2 const &v ) noexcept;

    private:
        [[maybe_unused]] swizzle_uint64_t2 () = default;

        [[maybe_unused]] swizzle_uint64_t2 ( swizzle_uint64_t2 const & ) = default;
        [[maybe_unused]] swizzle_uint64_t2 &operator = ( swizzle_uint64_t2 const & ) = default;

        [[maybe_unused]] swizzle_uint64_t2 ( swizzle_uint64_t2 && ) = default;
        [[maybe_unused]] swizzle_uint64_t2 &operator = ( swizzle_uint64_t2 && ) = default;

        [[maybe_unused]] constexpr swizzle_uint64_t2 ( uint64_t* xRef, uint64_t* yRef ) noexcept:
            x ( xRef ),
            y ( yRef )
        {
            // NOTHING
        }

        [[maybe_unused]] ~swizzle_uint64_t2 () = default;
};

//----------------------------------------------------------------------------------------------------------------------

class swizzle_uint64_t3 final
{
    friend class uint64_t2;
    friend class uint64_t4;

    private:
        uint64_t*       x = nullptr;
        uint64_t*       y = nullptr;
        uint64_t*       z = nullptr;

    public:
        [[maybe_unused]] ~swizzle_uint64_t3 () = default;

        [[maybe_unused, nodiscard]] operator uint64_t3 () const noexcept;

        [[maybe_unused]] void operator += ( uint64_t3 const &v ) noexcept;
        [[maybe_unused]] void operator -= ( uint64_t3 const &v ) noexcept;
        [[maybe_unused]] void operator *= ( uint64_t3 const &v ) noexcept;
        [[maybe_unused]] void operator /= ( uint64_t3 const &v ) noexcept;
        [[maybe_unused]] void operator >>= ( uint64_t3 const &v ) noexcept;
        [[maybe_unused]] void operator >>= ( uint32_t3 const &v ) noexcept;
        [[maybe_unused]] void operator <<= ( uint64_t3 const &v ) noexcept;
        [[maybe_unused]] void operator <<= ( uint32_t3 const &v ) noexcept;
        [[maybe_unused]] void operator &= ( uint64_t3 const &v ) noexcept;
        [[maybe_unused]] void operator |= ( uint64_t3 const &v ) noexcept;

        [[maybe_unused]] void operator += ( swizzle_uint64_t3 const &v ) noexcept;
        [[maybe_unused]] void operator -= ( swizzle_uint64_t3 const &v ) noexcept;
        [[maybe_unused]] void operator *= ( swizzle_uint64_t3 const &v ) noexcept;
        [[maybe_unused]] void operator /= ( swizzle_uint64_t3 const &v ) noexcept;
        [[maybe_unused]] void operator >>= ( swizzle_uint64_t3 const &v ) noexcept;
        [[maybe_unused]] void operator >>= ( swizzle_uint32_t3 const &v ) noexcept;
        [[maybe_unused]] void operator <<= ( swizzle_uint64_t3 const &v ) noexcept;
        [[maybe_unused]] void operator <<= ( swizzle_uint32_t3 const &v ) noexcept;
        [[maybe_unused]] void operator &= ( swizzle_uint64_t3 const &v ) noexcept;
        [[maybe_unused]] void operator |= ( swizzle_uint64_t3 const &v ) noexcept;

    private:
        [[maybe_unused]] swizzle_uint64_t3 () = default;

        [[maybe_unused]] swizzle_uint64_t3 ( swizzle_uint64_t3 const & ) = default;
        [[maybe_unused]] swizzle_uint64_t3 &operator = ( swizzle_uint64_t3 const & ) = default;

        [[maybe_unused]] swizzle_uint64_t3 ( swizzle_uint64_t3 && ) = default;
        [[maybe_unused]] swizzle_uint64_t3 &operator = ( swizzle_uint64_t3 && ) = default;

        [[maybe_unused]] constexpr swizzle_uint64_t3 ( uint64_t* xRef, uint64_t* yRef, uint64_t* zRef ) noexcept:
            x ( xRef ),
            y ( yRef ),
            z ( zRef )
        {
            // NOTHING
        }
};

//----------------------------------------------------------------------------------------------------------------------

class uint64_t2 final
{
    public:
        uint64_t                x;
        uint64_t                y;

        swizzle_uint64_t2       xx;
        swizzle_uint64_t2       yx;
        swizzle_uint64_t2       yy;

        swizzle_uint64_t3       xxx;
        swizzle_uint64_t3       yyy;

    public:
        [[maybe_unused]] uint64_t2 () noexcept;

        [[maybe_unused]] uint64_t2 ( uint64_t2 const &other ) noexcept;
        [[maybe_unused]] uint64_t2 &operator = ( uint64_t2 const &other ) noexcept;

        [[maybe_unused]] uint64_t2 ( uint64_t2 &&other ) noexcept;
        [[maybe_unused]] uint64_t2 &operator = ( uint64_t2 &&other ) noexcept;

        [[maybe_unused]] uint64_t2 ( uint64_t v ) noexcept;
        [[maybe_unused]] uint64_t2 ( uint64_t xVal, uint64_t yVal ) noexcept;
        [[maybe_unused]] uint64_t2 ( uint32_t2 const &v ) noexcept;

        [[maybe_unused]] ~uint64_t2 () = default;

        [[maybe_unused]] void operator += ( uint64_t2 const &v ) noexcept;
        [[maybe_unused]] void operator -= ( uint64_t2 const &v ) noexcept;
        [[maybe_unused]] void operator *= ( uint64_t2 const &v ) noexcept;
        [[maybe_unused]] void operator /= ( uint64_t2 const &v ) noexcept;
        [[maybe_unused]] void operator >>= ( uint64_t2 const &v ) noexcept;
        [[maybe_unused]] void operator >>= ( uint64_t v ) noexcept;
        [[maybe_unused]] void operator >>= ( uint32_t2 const &v ) noexcept;
        [[maybe_unused]] void operator >>= ( uint32_t v ) noexcept;
        [[maybe_unused]] void operator <<= ( uint64_t2 const &v ) noexcept;
        [[maybe_unused]] void operator <<= ( uint64_t v ) noexcept;
        [[maybe_unused]] void operator <<= ( uint32_t2 const &v ) noexcept;
        [[maybe_unused]] void operator <<= ( uint32_t v ) noexcept;
        [[maybe_unused]] void operator &= ( uint64_t2 const &v ) noexcept;
        [[maybe_unused]] void operator |= ( uint64_t2 const &v ) noexcept;

    private:
        void InitSwizzle () noexcept;
};

[[maybe_unused, nodiscard]] uint64_t2 operator + ( uint64_t2 const &a, uint64_t2 const &b ) noexcept;
[[maybe_unused, nodiscard]] uint64_t2 operator - ( uint64_t2 const &a, uint64_t2 const &b ) noexcept;
[[maybe_unused, nodiscard]] uint64_t2 operator * ( uint64_t2 const &a, uint64_t2 const &b ) noexcept;
[[maybe_unused, nodiscard]] uint64_t2 operator / ( uint64_t2 const &a, uint64_t2 const &b ) noexcept;
[[maybe_unused, nodiscard]] uint64_t2 operator >> ( uint64_t2 const &a, uint64_t2 const &b ) noexcept;
[[maybe_unused, nodiscard]] uint64_t2 operator >> ( uint64_t2 const &a, uint64_t b ) noexcept;
[[maybe_unused, nodiscard]] uint64_t2 operator >> ( uint64_t2 const &a, uint32_t2 const &b ) noexcept;
[[maybe_unused, nodiscard]] uint64_t2 operator >> ( uint64_t2 const &a, uint32_t b ) noexcept;
[[maybe_unused, nodiscard]] uint64_t2 operator << ( uint64_t2 const &a, uint64_t2 const &b ) noexcept;
[[maybe_unused, nodiscard]] uint64_t2 operator << ( uint64_t2 const &a, uint64_t b ) noexcept;
[[maybe_unused, nodiscard]] uint64_t2 operator << ( uint64_t2 const &a, uint32_t2 const &b ) noexcept;
[[maybe_unused, nodiscard]] uint64_t2 operator << ( uint64_t2 const &a, uint32_t b ) noexcept;
[[maybe_unused, nodiscard]] uint64_t2 operator & ( uint64_t2 const &a, uint64_t2 const &b ) noexcept;
[[maybe_unused, nodiscard]] uint64_t2 operator | ( uint64_t2 const &a, uint64_t2 const &b ) noexcept;

//----------------------------------------------------------------------------------------------------------------------

class uint64_t3 final
{
    public:
        uint64_t    x;
        uint64_t    y;
        uint64_t    z;

    public:
        [[maybe_unused]] uint64_t3 () = default;

        [[maybe_unused]] uint64_t3 ( uint64_t3 const& other ) noexcept;
        [[maybe_unused]] uint64_t3 &operator = ( uint64_t3 const &other ) noexcept;

        [[maybe_unused]] uint64_t3 ( uint64_t3 &&other ) noexcept;
        [[maybe_unused]] uint64_t3 &operator = ( uint64_t3 &&other ) noexcept;

        [[maybe_unused]] uint64_t3 ( uint64_t v ) noexcept;
        [[maybe_unused]] uint64_t3 ( uint64_t xVal, uint64_t yVal, uint64_t zVal ) noexcept;

        [[maybe_unused]] ~uint64_t3 () = default;

        [[maybe_unused]] void operator += ( uint64_t3 const &v ) noexcept;
        [[maybe_unused]] void operator -= ( uint64_t3 const &v ) noexcept;
        [[maybe_unused]] void operator *= ( uint64_t3 const &v ) noexcept;
        [[maybe_unused]] void operator /= ( uint64_t3 const &v ) noexcept;
        [[maybe_unused]] void operator >>= ( uint64_t3 const &v ) noexcept;
        [[maybe_unused]] void operator >>= ( uint32_t3 const &v ) noexcept;
        [[maybe_unused]] void operator <<= ( uint64_t3 const &v ) noexcept;
        [[maybe_unused]] void operator <<= ( uint32_t3 const &v ) noexcept;
        [[maybe_unused]] void operator &= ( uint64_t3 const &v ) noexcept;
        [[maybe_unused]] void operator |= ( uint64_t3 const &v ) noexcept;
};

[[maybe_unused, nodiscard]] uint64_t3 operator + ( uint64_t3 const &a, uint64_t3 const &b ) noexcept;
[[maybe_unused, nodiscard]] uint64_t3 operator - ( uint64_t3 const &a, uint64_t3 const &b ) noexcept;
[[maybe_unused, nodiscard]] uint64_t3 operator * ( uint64_t3 const &a, uint64_t3 const &b ) noexcept;
[[maybe_unused, nodiscard]] uint64_t3 operator / ( uint64_t3 const &a, uint64_t3 const &b ) noexcept;
[[maybe_unused, nodiscard]] uint64_t3 operator >> ( uint64_t3 const &a, uint64_t3 const &b ) noexcept;
[[maybe_unused, nodiscard]] uint64_t3 operator >> ( uint64_t3 const &a, uint32_t3 const &b ) noexcept;
[[maybe_unused, nodiscard]] uint64_t3 operator << ( uint64_t3 const &a, uint64_t3 const &b ) noexcept;
[[maybe_unused, nodiscard]] uint64_t3 operator << ( uint64_t3 const &a, uint32_t3 const &b ) noexcept;
[[maybe_unused, nodiscard]] uint64_t3 operator & ( uint64_t3 const &a, uint64_t3 const &b ) noexcept;
[[maybe_unused, nodiscard]] uint64_t3 operator | ( uint64_t3 const &a, uint64_t3 const &b ) noexcept;

//----------------------------------------------------------------------------------------------------------------------

class uint64_t4 final
{
    public:
        uint64_t                x;
        uint64_t                y;
        uint64_t                z;
        uint64_t                w;

        swizzle_uint64_t3       xyz;
        swizzle_uint64_t3       yzw;

    public:
        [[maybe_unused]] uint64_t4 () noexcept;

        [[maybe_unused]] uint64_t4 ( uint64_t4 const &other ) noexcept;
        [[maybe_unused]] uint64_t4 &operator = ( uint64_t4 const &other ) noexcept;

        [[maybe_unused]] uint64_t4 ( uint64_t4 &&other ) noexcept;
        [[maybe_unused]] uint64_t4 &operator = ( uint64_t4 &&other ) noexcept;

        [[maybe_unused]] uint64_t4 ( uint64_t v ) noexcept;
        [[maybe_unused]] uint64_t4 ( uint32_t4 const &v ) noexcept;
        [[maybe_unused]] uint64_t4 ( uint64_t xVal, uint64_t yVal, uint64_t zVal, uint64_t wVal ) noexcept;

        [[maybe_unused]] ~uint64_t4 () = default;

        [[maybe_unused]] void operator += ( uint64_t4 const &v ) noexcept;
        [[maybe_unused]] void operator -= ( uint64_t4 const &v ) noexcept;
        [[maybe_unused]] void operator *= ( uint64_t4 const &v ) noexcept;
        [[maybe_unused]] void operator /= ( uint64_t4 const &v ) noexcept;
        [[maybe_unused]] void operator >>= ( uint64_t4 const &v ) noexcept;
        [[maybe_unused]] void operator >>= ( uint32_t4 const &v ) noexcept;
        [[maybe_unused]] void operator <<= ( uint64_t4 const &v ) noexcept;
        [[maybe_unused]] void operator <<= ( uint32_t4 const &v ) noexcept;
        [[maybe_unused]] void operator &= ( uint64_t4 const &v ) noexcept;
        [[maybe_unused]] void operator |= ( uint64_t4 const &v ) noexcept;

    private:
        void InitSwizzle () noexcept;
};

[[maybe_unused, nodiscard]] uint64_t4 operator + ( uint64_t4 const &a, uint64_t4 const &b ) noexcept;
[[maybe_unused, nodiscard]] uint64_t4 operator - ( uint64_t4 const &a, uint64_t4 const &b ) noexcept;
[[maybe_unused, nodiscard]] uint64_t4 operator * ( uint64_t4 const &a, uint64_t4 const &b ) noexcept;
[[maybe_unused, nodiscard]] uint64_t4 operator / ( uint64_t4 const &a, uint64_t4 const &b ) noexcept;
[[maybe_unused, nodiscard]] uint64_t4 operator >> ( uint64_t4 const &a, uint64_t4 const &b ) noexcept;
[[maybe_unused, nodiscard]] uint64_t4 operator >> ( uint64_t4 const &a, uint32_t4 const &b ) noexcept;
[[maybe_unused, nodiscard]] uint64_t4 operator << ( uint64_t4 const &a, uint64_t4 const &b ) noexcept;
[[maybe_unused, nodiscard]] uint64_t4 operator << ( uint64_t4 const &a, uint32_t4 const &b ) noexcept;
[[maybe_unused, nodiscard]] uint64_t4 operator & ( uint64_t4 const &a, uint64_t4 const &b ) noexcept;
[[maybe_unused, nodiscard]] uint64_t4 operator | ( uint64_t4 const &a, uint64_t4 const &b ) noexcept;

//----------------------------------------------------------------------------------------------------------------------

class swizzle_float32_t3 final
{
    friend class float32_t3;
    friend class float32_t4;

    private:
        float32_t*      x = nullptr;
        float32_t*      y = nullptr;
        float32_t*      z = nullptr;

    public:
        [[maybe_unused]] ~swizzle_float32_t3 () = default;

        [[maybe_unused, nodiscard]] operator float32_t3 () const noexcept;

        [[maybe_unused]] void operator += ( float32_t3 const &v ) noexcept;
        [[maybe_unused]] void operator -= ( float32_t3 const &v ) noexcept;
        [[maybe_unused]] void operator *= ( float32_t3 const &v ) noexcept;
        [[maybe_unused]] void operator /= ( float32_t3 const &v ) noexcept;

        [[maybe_unused]] void operator += ( swizzle_float32_t3 const &v ) noexcept;
        [[maybe_unused]] void operator -= ( swizzle_float32_t3 const &v ) noexcept;
        [[maybe_unused]] void operator *= ( swizzle_float32_t3 const &v ) noexcept;
        [[maybe_unused]] void operator /= ( swizzle_float32_t3 const &v ) noexcept;

    private:
        [[maybe_unused]] swizzle_float32_t3 () = default;

        [[maybe_unused]] swizzle_float32_t3 ( swizzle_float32_t3 const & ) = default;
        [[maybe_unused]] swizzle_float32_t3 &operator = ( swizzle_float32_t3 const & ) = default;

        [[maybe_unused]] swizzle_float32_t3 ( swizzle_float32_t3 && ) = default;
        [[maybe_unused]] swizzle_float32_t3 &operator = ( swizzle_float32_t3 && ) = default;

        [[maybe_unused]] constexpr swizzle_float32_t3 ( float32_t* xRef, float32_t* yRef, float32_t* zRef ) noexcept:
            x ( xRef ),
            y ( yRef ),
            z ( zRef )
        {
            // NOTHING
        }
};

//----------------------------------------------------------------------------------------------------------------------

class swizzle_float32_t4 final
{
    friend class float32_t4;

    private:
        float32_t*      x = nullptr;
        float32_t*      y = nullptr;
        float32_t*      z = nullptr;
        float32_t*      w = nullptr;

    public:
        [[maybe_unused]] ~swizzle_float32_t4 () = default;

        [[maybe_unused, nodiscard]] operator float32_t4 () const noexcept;

        [[maybe_unused]] void operator += ( float32_t4 const &v ) noexcept;
        [[maybe_unused]] void operator -= ( float32_t4 const &v ) noexcept;
        [[maybe_unused]] void operator *= ( float32_t4 const &v ) noexcept;
        [[maybe_unused]] void operator /= ( float32_t4 const &v ) noexcept;

        [[maybe_unused]] void operator += ( swizzle_float32_t4 const &v ) noexcept;
        [[maybe_unused]] void operator -= ( swizzle_float32_t4 const &v ) noexcept;
        [[maybe_unused]] void operator *= ( swizzle_float32_t4 const &v ) noexcept;
        [[maybe_unused]] void operator /= ( swizzle_float32_t4 const &v ) noexcept;

    private:
        [[maybe_unused]] swizzle_float32_t4 () = default;

        [[maybe_unused]] swizzle_float32_t4 ( swizzle_float32_t4 const & ) = default;
        [[maybe_unused]] swizzle_float32_t4 &operator = ( swizzle_float32_t4 const & ) = default;

        [[maybe_unused]] swizzle_float32_t4 ( swizzle_float32_t4 && ) = default;
        [[maybe_unused]] swizzle_float32_t4 &operator = ( swizzle_float32_t4 && ) = default;

        [[maybe_unused]] constexpr swizzle_float32_t4 ( float32_t* xRef,
            float32_t* yRef,
            float32_t* zRef,
            float32_t* wRef
        ) noexcept:
            x ( xRef ),
            y ( yRef ),
            z ( zRef ),
            w ( wRef )
        {
            // NOTHING
        }
};

//----------------------------------------------------------------------------------------------------------------------

class float32_t3 final
{
    public:
        float32_t               x;
        float32_t               y;
        float32_t               z;

        swizzle_float32_t3      xxx;
        swizzle_float32_t3      yyy;
        swizzle_float32_t3      zzz;

    public:
        [[maybe_unused]] float32_t3 () noexcept;

        [[maybe_unused]] float32_t3 ( float32_t3 const& other ) noexcept;
        [[maybe_unused]] float32_t3 &operator = ( float32_t3 const &other ) noexcept;

        [[maybe_unused]] float32_t3 ( float32_t3 &&other ) noexcept;
        [[maybe_unused]] float32_t3 &operator = ( float32_t3 &&other ) noexcept;

        [[maybe_unused]] float32_t3 ( float32_t v ) noexcept;
        [[maybe_unused]] float32_t3 ( float32_t xVal, float32_t yVal, float32_t zVal ) noexcept;
        [[maybe_unused]] float32_t3 ( uint32_t3 v ) noexcept;

        [[maybe_unused]] ~float32_t3 () = default;

        [[maybe_unused]] void operator += ( float32_t3 const &v ) noexcept;
        [[maybe_unused]] void operator -= ( float32_t3 const &v ) noexcept;
        [[maybe_unused]] void operator *= ( float32_t3 const &v ) noexcept;
        [[maybe_unused]] void operator /= ( float32_t3 const &v ) noexcept;

    private:
        void InitSwizzle () noexcept;
};

[[maybe_unused, nodiscard]] float32_t3 operator + ( float32_t3 const &a, float32_t3 const &b ) noexcept;
[[maybe_unused, nodiscard]] float32_t3 operator - ( float32_t3 const &a, float32_t3 const &b ) noexcept;
[[maybe_unused, nodiscard]] float32_t3 operator * ( float32_t3 const &a, float32_t3 const &b ) noexcept;
[[maybe_unused, nodiscard]] float32_t3 operator / ( float32_t3 const &a, float32_t3 const &b ) noexcept;

[[maybe_unused, nodiscard]] float32_t dot ( float32_t3 const &a, float32_t3 const &b ) noexcept;

[[maybe_unused, nodiscard]] float32_t3 mad ( float32_t3 const &a, float32_t3 const &b, float32_t c ) noexcept;
[[maybe_unused, nodiscard]] float32_t3 mad ( float32_t3 const &a, float32_t3 const &b, float32_t3 const &c ) noexcept;

//----------------------------------------------------------------------------------------------------------------------

class float32_t4 final
{
    public:
        float32_t               x;
        float32_t               y;
        float32_t               z;
        float32_t               w;

        swizzle_float32_t3      xxx;
        swizzle_float32_t3      yyy;
        swizzle_float32_t3      zzz;
        swizzle_float32_t3      www;

    public:
        [[maybe_unused]] float32_t4 () noexcept;

        [[maybe_unused]] float32_t4 ( float32_t4 const &other ) noexcept;
        [[maybe_unused]] float32_t4 &operator = ( float32_t4 const &other ) noexcept;

        [[maybe_unused]] float32_t4 ( float32_t4 &&other ) noexcept;
        [[maybe_unused]] float32_t4 &operator = ( float32_t4 &&other ) noexcept;

        [[maybe_unused]] float32_t4 ( float32_t v ) noexcept;
        [[maybe_unused]] float32_t4 ( float32_t xVal, float32_t yVal, float32_t zVal, float32_t wVal ) noexcept;
        [[maybe_unused]] float32_t4 ( float32_t3 const &xyzVal, float32_t wVal ) noexcept;
        [[maybe_unused]] float32_t4 ( float32_t x, float32_t3 const &yzwVal ) noexcept;

        [[maybe_unused]] ~float32_t4 () = default;

        [[maybe_unused]] void operator += ( float32_t4 const &v ) noexcept;
        [[maybe_unused]] void operator -= ( float32_t4 const &v ) noexcept;
        [[maybe_unused]] void operator *= ( float32_t4 const &v ) noexcept;
        [[maybe_unused]] void operator /= ( float32_t4 const &v ) noexcept;

    private:
        void InitSwizzle () noexcept;
};

[[maybe_unused, nodiscard]] float32_t4 operator + ( float32_t4 const &a, float32_t4 const &b ) noexcept;
[[maybe_unused, nodiscard]] float32_t4 operator - ( float32_t4 const &a, float32_t4 const &b ) noexcept;
[[maybe_unused, nodiscard]] float32_t4 operator * ( float32_t4 const &a, float32_t4 const &b ) noexcept;
[[maybe_unused, nodiscard]] float32_t4 operator / ( float32_t4 const &a, float32_t4 const &b ) noexcept;

} // namespace hlsl

#endif // IDEAS_HLSL_HPP
