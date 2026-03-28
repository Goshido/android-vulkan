#ifndef IDEAS_HLSL_HPP
#define IDEAS_HLSL_HPP


namespace hlsl {

class uint32_t2 final
{
    public:
        uint32_t    x;
        uint32_t    y;

    public:
        [[maybe_unused]] uint32_t2 () = default;

        [[maybe_unused]] uint32_t2 ( uint32_t2 const & ) = default;
        [[maybe_unused]] uint32_t2 &operator = ( uint32_t2 const & ) = default;

        [[maybe_unused]] uint32_t2 ( uint32_t2 && ) = default;
        [[maybe_unused]] uint32_t2 &operator = ( uint32_t2 && ) = default;

        [[maybe_unused]] constexpr uint32_t2 ( uint32_t xVal, uint32_t yVal ) noexcept:
            x ( xVal ),
            y ( yVal )
        {
            // NOTHING
        }

        [[maybe_unused]] ~uint32_t2 () = default;

        uint32_t2 &operator += ( uint32_t2 const &v ) noexcept;
        uint32_t2 &operator -= ( uint32_t2 const &v ) noexcept;
        uint32_t2 &operator *= ( uint32_t2 const &v ) noexcept;
        uint32_t2 &operator /= ( uint32_t2 const &v ) noexcept;
        uint32_t2 &operator >>= ( uint32_t2 const &v ) noexcept;
        uint32_t2 &operator <<= ( uint32_t2 const &v ) noexcept;
        uint32_t2 &operator &= ( uint32_t2 const &v ) noexcept;
        uint32_t2 &operator |= ( uint32_t2 const &v ) noexcept;
};

[[nodiscard]] uint32_t2 operator + ( uint32_t2 const &a, uint32_t2 const &b ) noexcept;
[[nodiscard]] uint32_t2 operator - ( uint32_t2 const &a, uint32_t2 const &b ) noexcept;
[[nodiscard]] uint32_t2 operator * ( uint32_t2 const &a, uint32_t2 const &b ) noexcept;
[[nodiscard]] uint32_t2 operator / ( uint32_t2 const &a, uint32_t2 const &b ) noexcept;
[[nodiscard]] uint32_t2 operator >> ( uint32_t2 const &a, uint32_t2 const &b ) noexcept;
[[nodiscard]] uint32_t2 operator << ( uint32_t2 const &a, uint32_t2 const &b ) noexcept;
[[nodiscard]] uint32_t2 operator & ( uint32_t2 const &a, uint32_t2 const &b ) noexcept;
[[nodiscard]] uint32_t2 operator | ( uint32_t2 const &a, uint32_t2 const &b ) noexcept;

//----------------------------------------------------------------------------------------------------------------------

class uint32_t3 final
{
    public:
        uint32_t    x;
        uint32_t    y;
        uint32_t    z;

    public:
        [[maybe_unused]] uint32_t3 () = default;

        [[maybe_unused]] uint32_t3 ( uint32_t3 const & ) = default;
        [[maybe_unused]] uint32_t3 &operator = ( uint32_t3 const & ) = default;

        [[maybe_unused]] uint32_t3 ( uint32_t3 && ) = default;
        [[maybe_unused]] uint32_t3 &operator = ( uint32_t3 && ) = default;

        [[maybe_unused]] constexpr uint32_t3 ( uint32_t xVal, uint32_t yVal, uint32_t zVal ) noexcept:
            x ( xVal ),
            y ( yVal ),
            z ( zVal )
        {
            // NOTHING
        }

        [[maybe_unused]] constexpr uint32_t3 ( uint32_t2 const &xy, uint32_t zVal ) noexcept:
            x ( xy.x ),
            y ( xy.y ),
            z ( zVal )
        {
            // NOTHING
        }

        [[maybe_unused]] ~uint32_t3 () = default;

        uint32_t3 &operator += ( uint32_t3 const &v ) noexcept;
        uint32_t3 &operator -= ( uint32_t3 const &v ) noexcept;
        uint32_t3 &operator *= ( uint32_t3 const &v ) noexcept;
        uint32_t3 &operator /= ( uint32_t3 const &v ) noexcept;
        uint32_t3 &operator >>= ( uint32_t3 const &v ) noexcept;
        uint32_t3 &operator <<= ( uint32_t3 const &v ) noexcept;
        uint32_t3 &operator &= ( uint32_t3 const &v ) noexcept;
        uint32_t3 &operator |= ( uint32_t3 const &v ) noexcept;
};

[[nodiscard]] uint32_t3 operator + ( uint32_t3 const &a, uint32_t3 const &b ) noexcept;
[[nodiscard]] uint32_t3 operator - ( uint32_t3 const &a, uint32_t3 const &b ) noexcept;
[[nodiscard]] uint32_t3 operator * ( uint32_t3 const &a, uint32_t3 const &b ) noexcept;
[[nodiscard]] uint32_t3 operator / ( uint32_t3 const &a, uint32_t3 const &b ) noexcept;
[[nodiscard]] uint32_t3 operator >> ( uint32_t3 const &a, uint32_t3 const &b ) noexcept;
[[nodiscard]] uint32_t3 operator << ( uint32_t3 const &a, uint32_t3 const &b ) noexcept;
[[nodiscard]] uint32_t3 operator & ( uint32_t3 const &a, uint32_t3 const &b ) noexcept;
[[nodiscard]] uint32_t3 operator | ( uint32_t3 const &a, uint32_t3 const &b ) noexcept;

//----------------------------------------------------------------------------------------------------------------------

class uint32_t4 final
{
    public:
        uint32_t    x;
        uint32_t    y;
        uint32_t    z;
        uint32_t    w;

    public:
        [[maybe_unused]] uint32_t4 () = default;

        [[maybe_unused]] uint32_t4 ( uint32_t4 const & ) = default;
        [[maybe_unused]] uint32_t4 &operator = ( uint32_t4 const & ) = default;

        [[maybe_unused]] uint32_t4 ( uint32_t4 && ) = default;
        [[maybe_unused]] uint32_t4 &operator = ( uint32_t4 && ) = default;

        [[maybe_unused]] constexpr uint32_t4 ( uint32_t xVal, uint32_t yVal, uint32_t zVal, uint32_t wVal ) noexcept:
            x ( xVal ),
            y ( yVal ),
            z ( zVal ),
            w ( wVal )
        {
            // NOTHING
        }

        [[maybe_unused]] constexpr uint32_t4 ( uint32_t2 const &xy, uint32_t zVal, uint32_t wVal ) noexcept:
            x ( xy.x ),
            y ( xy.y ),
            z ( zVal ),
            w ( wVal )
        {
            // NOTHING
        }

        [[maybe_unused]] constexpr uint32_t4 ( uint32_t2 const &xy, uint32_t2 const &zwVal ) noexcept:
            x ( xy.x ),
            y ( xy.y ),
            z ( zwVal.x ),
            w ( zwVal.y )
        {
            // NOTHING
        }

        [[maybe_unused]] constexpr uint32_t4 ( uint32_t3 const &xyz, uint32_t const &wVal ) noexcept:
            x ( xyz.x ),
            y ( xyz.y ),
            z ( xyz.z ),
            w ( wVal )
        {
            // NOTHING
        }

        [[maybe_unused]] ~uint32_t4 () = default;

        uint32_t4 &operator += ( uint32_t4 const &v ) noexcept;
        uint32_t4 &operator -= ( uint32_t4 const &v ) noexcept;
        uint32_t4 &operator *= ( uint32_t4 const &v ) noexcept;
        uint32_t4 &operator /= ( uint32_t4 const &v ) noexcept;
        uint32_t4 &operator >>= ( uint32_t4 const &v ) noexcept;
        uint32_t4 &operator <<= ( uint32_t4 const &v ) noexcept;
        uint32_t4 &operator &= ( uint32_t4 const &v ) noexcept;
        uint32_t4 &operator |= ( uint32_t4 const &v ) noexcept;
};

[[nodiscard]] uint32_t4 operator + ( uint32_t4 const &a, uint32_t4 const &b ) noexcept;
[[nodiscard]] uint32_t4 operator - ( uint32_t4 const &a, uint32_t4 const &b ) noexcept;
[[nodiscard]] uint32_t4 operator * ( uint32_t4 const &a, uint32_t4 const &b ) noexcept;
[[nodiscard]] uint32_t4 operator / ( uint32_t4 const &a, uint32_t4 const &b ) noexcept;
[[nodiscard]] uint32_t4 operator >> ( uint32_t4 const &a, uint32_t4 const &b ) noexcept;
[[nodiscard]] uint32_t4 operator << ( uint32_t4 const &a, uint32_t4 const &b ) noexcept;
[[nodiscard]] uint32_t4 operator & ( uint32_t4 const &a, uint32_t4 const &b ) noexcept;
[[nodiscard]] uint32_t4 operator | ( uint32_t4 const &a, uint32_t4 const &b ) noexcept;

} // namespace hlsl


#endif // IDEAS_HLSL_HPP
