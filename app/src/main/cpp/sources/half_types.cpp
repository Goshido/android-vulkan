#include <precompiled_headers.hpp>
#include <half_types.hpp>


namespace android_vulkan {

[[maybe_unused]] Half::Half ( float value ) noexcept:
    _data ( Convert ( value ) )
{
    // NOTHING
}

Half &Half::operator = ( float v ) noexcept
{
    _data = Convert ( v );
    return *this;
}

uint16_t Half::Convert ( float value ) noexcept
{
    // see https://en.wikipedia.org/wiki/Single-precision_floating-point_format
    // see https://en.wikipedia.org/wiki/Half-precision_floating-point_format
    // see https://en.wikipedia.org/wiki/NaN
    // see https://en.wikipedia.org/wiki/IEEE_754-1985#Positive_and_negative_infinity

    uint32_t const from = *reinterpret_cast<uint32_t*> ( &value );

    uint32_t const mantissa = from & 0x007FFFFFU;
    uint32_t const sign = from & 0x80000000U;
    uint32_t const exponent = from & 0x7F800000U;

    // checking special cases: zeros, NaNs and INFs

    if ( mantissa == 0 && exponent == 0 )
    {
        // positive|negative zero branch
        return static_cast<uint16_t> ( sign >> 16U );
    }

    if ( exponent == 0xFF )
    {
        if ( mantissa == 0 )
        {
            // INF branch
            return static_cast<uint16_t> ( ( sign >> 16U ) | 0x00007C00U );
        }

        // NaN branches

        if ( mantissa & 0x400000U )
        {
            // signaling NaN
            return static_cast<uint16_t> ( ( sign >> 16U ) | 0x00007C00U );
        }

        // quiet NaN
        return static_cast<uint16_t> ( ( sign >> 16U ) | 0x00007E00U );
    }

    auto const exponentRaw = static_cast<uint8_t> ( exponent >> 23U );

    // removing exponent bias (subtract 127)
    // see https://en.wikipedia.org/wiki/Single-precision_floating-point_format
    auto restoredExponent = static_cast<int16_t> ( exponentRaw - 0x7F );

    if ( restoredExponent >= 0 )
    {
        // positive exponent

        if ( restoredExponent > 0x000F )
        {
            // exponent is bigger than float16 can represent -> INF
            return static_cast<uint16_t> ( ( sign >> 16U ) | 0x0000FC00U );
        }
    }
    else
    {
        // negative exponent

        if ( restoredExponent < -0x000E )
        {
            // exponent is less than float16 can represent -> zero
            return static_cast<uint16_t> ( sign >> 16U );
        }
    }

    // biasing exponent (add 15).
    // see https://en.wikipedia.org/wiki/Half-precision_floating-point_format
    restoredExponent += 0x000F;

    // input number is normalized by design. reassemble it
    return static_cast<uint16_t> (
        ( sign >> 16U ) | ( static_cast<uint32_t> ( restoredExponent ) << 10U ) | ( mantissa >> 13U )
    );
}

//----------------------------------------------------------------------------------------------------------------------

[[maybe_unused]] Half2::Half2 ( float component0, float component1 ) noexcept:
    _data { Half::Convert ( component0 ), Half::Convert ( component1 ) }
{
    // NOTHING
}

[[maybe_unused]] Half2::Half2 ( GXVec2 const &v ) noexcept:
    _data { Half::Convert ( v._data[ 0U ] ), Half::Convert ( v._data[ 1U ] ) }
{
    // NOTHING
}

//----------------------------------------------------------------------------------------------------------------------

[[maybe_unused]] Half3::Half3 ( float component0, float component1, float component2 ) noexcept:
    _data { Half::Convert ( component0 ), Half::Convert ( component1 ), Half::Convert ( component2 ) }
{
    // NOTHING
}

Half3 &Half3::operator = ( GXVec3 const &other ) noexcept
{
    _data[ 0U ] = Half::Convert ( other._data[ 0U ] );
    _data[ 1U ] = Half::Convert ( other._data[ 0U ] );
    _data[ 2U ] = Half::Convert ( other._data[ 0U ] );

    return *this;
}

//----------------------------------------------------------------------------------------------------------------------

Half4::Half4 ( float component0, float component1, float component2, float component3 ) noexcept:
    _data
    {
        Half::Convert ( component0 ),
        Half::Convert ( component1 ),
        Half::Convert ( component2 ),
        Half::Convert ( component3 )
    }
{
    // NOTHING
}

void Half4::From ( uint8_t component0, uint8_t component1, uint8_t component2, uint8_t component3 ) noexcept
{
    constexpr float unormToFloat = 1.0F / 255.0F;

    _data[ 0U ] = Half::Convert ( static_cast<float> ( component0 ) * unormToFloat );
    _data[ 1U ] = Half::Convert ( static_cast<float> ( component1 ) * unormToFloat );
    _data[ 2U ] = Half::Convert ( static_cast<float> ( component2 ) * unormToFloat );
    _data[ 3U ] = Half::Convert ( static_cast<float> ( component3 ) * unormToFloat );
}

} // namespace android_vulkan
