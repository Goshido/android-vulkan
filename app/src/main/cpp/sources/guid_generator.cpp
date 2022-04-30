#include <guid_generator.h>

GX_DISABLE_COMMON_WARNINGS

#include <cinttypes>
#include <ctime>
#include <random>
#include <unistd.h>

GX_RESTORE_WARNING_STATE


namespace android_vulkan {

std::string GUID::GenerateAsString ( std::string_view prefix ) noexcept
{
    TimeUUID timeUUID = Unpack ( GenerateUUID () );
    timeUUID._clockSeq = ( timeUUID._clockSeq & UINT16_C ( 0x3FFF ) ) | UINT16_C ( 0x8000 );
    timeUUID._timeHiAndBersion = (timeUUID._timeHiAndBersion & UINT16_C ( 0x0FFF ) ) | UINT16_C ( 0x4000 );

    UUID const final = Pack ( timeUUID );

    // @xxxxxxxx-xxxx-xxxx-xxxx-xxxxxxxxxxxx
    constexpr char const format[] = "@%02" PRIx8 "%02" PRIx8 "%02" PRIx8 "%02" PRIx8
        "-%02" PRIx8 "%02" PRIx8
        "-%02" PRIx8 "%02" PRIx8
        "-%02" PRIx8 "%02" PRIx8
        "-%02" PRIx8 "%02" PRIx8 "%02" PRIx8 "%02" PRIx8 "%02" PRIx8 "%02" PRIx8;

    constexpr size_t bufferSize = 38U;
    char buf[ bufferSize ];

    std::snprintf ( buf,
        bufferSize,
        format,
        final._data[ 0U ],
        final._data[ 1U ],
        final._data[ 2U ],
        final._data[ 3U ],
        final._data[ 4U ],
        final._data[ 5U ],
        final._data[ 6U ],
        final._data[ 7U ],
        final._data[ 8U ],
        final._data[ 9U ],
        final._data[ 10U ],
        final._data[ 11U ],
        final._data[ 12U ],
        final._data[ 13U ],
        final._data[ 14U ],
        final._data[ 15U ]
    );

    std::string result {};
    size_t const prefixSize = prefix.size ();
    size_t const actualSize = prefixSize + bufferSize;
    result.resize ( actualSize );

    char* dst = result.data ();
    std::memcpy ( dst, prefix.data (), prefixSize );
    std::memcpy ( dst + prefixSize, buf, bufferSize );
    result.resize ( actualSize - 1U );

    return result;
}

GUID::UUID GUID::GenerateUUID () noexcept
{
    timeval tv {};
    gettimeofday ( &tv, nullptr );

    uint32_t const seed = ( static_cast<uint32_t> ( getpid () ) << 16U ) ^ static_cast<uint32_t> ( getuid () ) ^
        static_cast<uint32_t> ( tv.tv_sec ) ^ static_cast<uint32_t> ( tv.tv_usec );

    std::srand ( seed );

    gettimeofday ( &tv, nullptr );
    auto const limit = static_cast<uint8_t> ( static_cast<uint32_t> ( tv.tv_sec ^ tv.tv_usec ) & 0x1FU );

    for ( uint8_t i = 0U; i < limit; ++i )
    {
        // NOLINTNEXTLINE - limited randomness, use C++ 11 instead. But the original algorithm requires it.
        std::rand ();
    }

    std::random_device random {};

    UUID result {};
    auto* dst = reinterpret_cast<std::random_device::result_type*> ( result._data );

    static_assert ( sizeof ( std::random_device::result_type ) == 4U );
    dst[ 0U ] = random ();
    dst[ 1U ] = random ();
    dst[ 2U ] = random ();
    dst[ 3U ] = random ();

    for ( auto& b : result._data )
    {
        // NOLINTNEXTLINE - limited randomness, use C++ 11 instead. But the original algorithm requires it.
        b ^= ( static_cast<uint32_t> ( std::rand () ) >> 7U );
    }

    return result;
}

GUID::UUID GUID::Pack ( TimeUUID const &timeUUID ) noexcept
{
    uint32_t tmp;

    UUID result {};
    uint8_t *out = result._data;

    tmp = timeUUID._timeLow;
    out[ 3U ] = static_cast<uint8_t> ( tmp );

    tmp >>= 8U;
    out[ 2U ] = static_cast<uint8_t> ( tmp );

    tmp >>= 8U;
    out[ 1U ] = static_cast<uint8_t> ( tmp );

    tmp >>= 8U;
    out[ 0U ] = static_cast<uint8_t> ( tmp );

    tmp = timeUUID._timeMid;
    out[ 5U ] = static_cast<uint8_t> ( tmp );

    tmp >>= 8U;
    out[ 4U ] = static_cast<uint8_t> ( tmp );

    tmp = timeUUID._timeHiAndBersion;
    out[ 7U ] = static_cast<uint8_t> ( tmp );

    tmp >>= 8U;
    out[ 6U ] = static_cast<uint8_t> ( tmp );

    tmp = timeUUID._clockSeq;
    out[ 9U ] = static_cast<uint8_t> ( tmp );

    tmp >>= 8U;
    out[ 8U ] = static_cast<uint8_t> ( tmp );

    std::memcpy ( out + 10U, timeUUID._node, sizeof ( timeUUID._node ) );
    return result;
}

GUID::TimeUUID GUID::Unpack ( UUID const &uuid ) noexcept
{
    TimeUUID result {};

    uint8_t const* ptr = uuid._data;
    uint32_t tmp;

    tmp = *ptr++;
    tmp = ( tmp << 8U ) | *ptr++;
    tmp = ( tmp << 8U ) | *ptr++;
    tmp = ( tmp << 8U ) | *ptr++;
    result._timeLow = tmp;

    tmp = *ptr++;
    tmp = ( tmp << 8U ) | *ptr++;
    result._timeMid = tmp;

    tmp = *ptr++;
    tmp = ( tmp << 8U ) | *ptr++;
    result._timeHiAndBersion = tmp;

    tmp = *ptr++;
    tmp = ( tmp << 8U ) | *ptr++;
    result._clockSeq = tmp;

    std::memcpy ( result._node, ptr, sizeof ( result._node ) );
    return result;
}

} // namespace android_vulkan
