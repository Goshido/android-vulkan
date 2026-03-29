#ifndef IDEAS_HLSL_HPP
#define IDEAS_HLSL_HPP


namespace hlsl {

class uint32_t2;

class swizzle_uint32_t2 final
{
    private:
        uint32_t*       x = nullptr;
        uint32_t*       y = nullptr;

    public:
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

        [[maybe_unused, nodiscard]] operator uint32_t2 () const noexcept;

        [[maybe_unused]] void operator += ( uint32_t2 const &v ) noexcept;
        [[maybe_unused]] void operator -= ( uint32_t2 const &v ) noexcept;
        [[maybe_unused]] void operator *= ( uint32_t2 const &v ) noexcept;
        [[maybe_unused]] void operator /= ( uint32_t2 const &v ) noexcept;
        [[maybe_unused]] void operator >>= ( uint32_t2 const &v ) noexcept;
        [[maybe_unused]] void operator <<= ( uint32_t2 const &v ) noexcept;
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
};

//----------------------------------------------------------------------------------------------------------------------

class uint32_t3;

class swizzle_uint32_t3 final
{
    private:
        uint32_t*       x = nullptr;
        uint32_t*       y = nullptr;
        uint32_t*       z = nullptr;

    public:
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
};

//----------------------------------------------------------------------------------------------------------------------

class uint32_t4;

class swizzle_uint32_t4 final
{
    private:
        uint32_t*       x = nullptr;
        uint32_t*       y = nullptr;
        uint32_t*       z = nullptr;
        uint32_t*       w = nullptr;

    public:
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
        [[maybe_unused]] void operator <<= ( uint32_t2 const &v ) noexcept;
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
[[maybe_unused, nodiscard]] uint32_t2 operator << ( uint32_t2 const &a, uint32_t2 const &b ) noexcept;
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
        [[maybe_unused]] uint32_t3 ( uint32_t2 const &xy, uint32_t zVal ) noexcept;

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
        uint32_t    x;
        uint32_t    y;
        uint32_t    z;
        uint32_t    w;

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
        [[maybe_unused]] uint32_t4 ( uint32_t xVal, uint32_t yVal, uint32_t zVal, uint32_t wVal ) noexcept;
        [[maybe_unused]] uint32_t4 ( uint32_t2 const &xy, uint32_t zVal, uint32_t wVal ) noexcept;
        [[maybe_unused]] uint32_t4 ( uint32_t2 const &xy, uint32_t2 const &zwVal ) noexcept;
        [[maybe_unused]] uint32_t4 ( uint32_t3 const &xyz, uint32_t wVal ) noexcept;

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

} // namespace hlsl


#endif // IDEAS_HLSL_HPP
