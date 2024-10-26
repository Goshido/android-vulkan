#include <precompiled_headers.hpp>
#include <logger.hpp>
#include <pbr/utf8_parser.hpp>


namespace pbr {

namespace {

constexpr uint8_t LINE_FEED = UINT8_C ( 0x0A );

} // end of anonymous namespace

std::optional<UTF8Parser::Result> UTF8Parser::Parse ( char const* where, Stream stream ) noexcept
{
    if ( !stream.ExpectNotEmpty ( where, "pbr::UTF8Parser::Parse" ) )
        return std::nullopt;

    uint8_t const octet = stream._data.front ();

    if ( ( octet & UINT8_C ( 0x80 ) ) == UINT8_C ( 0x00 ) )
    {
        return Result
        {
            ._character = static_cast<char32_t> ( octet ),

            ._newStream = Stream ( stream._data.subspan ( 1U ),
                stream._line + static_cast<size_t> ( octet == LINE_FEED )
            )
        };
    }

    size_t const size = stream._data.size ();

    // Common mask for middle and last octets.
    constexpr char32_t m = 0x0000'003FU;

    // Get first octet.
    auto const getF = [ stream ] ( char32_t mask, char32_t shift ) noexcept -> char32_t {
        return ( mask & static_cast<char32_t> ( stream._data.front () ) ) << shift;
    };

    // Get last octet.
    auto const getL = [ stream ] ( size_t idx ) noexcept -> char32_t {
        return m & static_cast<char32_t> ( stream._data[ idx ] );
    };

    auto const checkSize = [ size, where, line = stream._line ] ( size_t requiredSize ) noexcept -> bool {
        if ( size >= requiredSize )
            return true;

        android_vulkan::LogError ( "pbr::UTF8Parser::Parse - %s:%zu: Unexpected end of stream [%zu octet case]!",
            where,
            line,
            requiredSize
        );

        return false;
    };

    if ( ( octet & UINT8_C ( 0xE0 ) ) == UINT8_C ( 0xC0 ) )
    {
        if ( !checkSize ( 2U ) )
            return std::nullopt;

        return Result
        {
            ._character = getF ( 0x0000'001FU, 6U ) | getL ( 1U ),
            ._newStream = Stream ( stream._data.subspan ( 2U ), stream._line )
        };
    }

    // Get middle octet.
    auto const getM = [ stream ] ( size_t idx, char32_t shift ) noexcept -> char32_t {
        return ( m & static_cast<char32_t> ( stream._data[ idx ] ) ) << shift;
    };

    if ( ( octet & UINT8_C ( 0xF0 ) ) == UINT8_C ( 0xE0 ) )
    {
        if ( !checkSize ( 3U ) )
            return std::nullopt;

        return Result
        {
            ._character = getF ( 0x0000'000FU, 12U ) | getM ( 1U, 6U ) | getL ( 2U ),
            ._newStream = Stream ( stream._data.subspan ( 3U ), stream._line )
        };
    }

    if ( ( octet & UINT8_C ( 0xF8 ) ) == UINT8_C ( 0xF0 ) )
    {
        if ( !checkSize ( 4U ) )
            return std::nullopt;

        return Result
        {
            ._character = getF ( 0x0000'0007U, 18U ) | getM ( 1U, 12U ) | getM ( 2U, 6U ) | getL ( 3U ),
            ._newStream = Stream ( stream._data.subspan ( 4U ), stream._line )
        };
    }

    if ( ( octet & UINT8_C ( 0xFC ) ) == UINT8_C ( 0xF8 ) )
    {
        if ( !checkSize ( 5U ) )
            return std::nullopt;

        return Result
        {
            ._character = getF ( 0x0000'0003U, 24U ) |
                getM ( 1U, 18U ) |
                getM ( 2U, 12U ) |
                getM ( 3U, 6U ) |
                getL ( 4U ),

            ._newStream = Stream ( stream._data.subspan ( 5U ), stream._line )
        };
    }

    if ( ( octet & UINT8_C ( 0xFE ) ) == UINT8_C ( 0xFC ) )
    {
        if ( !checkSize ( 6U ) )
            return std::nullopt;

        return Result
        {
            ._character = getF ( 0x0000'0001U, 30U ) |
                getM ( 1U, 24U ) |
                getM ( 2U, 18U ) |
                getM ( 3U, 12U ) |
                getM ( 4U, 6U ) |
                getL ( 5U ),

            ._newStream = Stream ( stream._data.subspan ( 6U ), stream._line )
        };
    }

    android_vulkan::LogError ( "pbr::UTF8Parser::Parse - %s:%zu: Unexpected octet!", where, stream._line );
    return std::nullopt;
}

std::optional<std::u32string> UTF8Parser::ToU32String ( std::string_view string ) noexcept
{
    std::u32string result {};
    result.reserve ( string.size () + 1U );

    auto* str = const_cast<char*> ( string.data () );
    Stream stream ( Stream::Data ( reinterpret_cast<uint8_t*> ( str ), string.size () ), 1U );

    while ( !stream._data.empty () )
    {
        auto const probe = Parse ( "UTF8Parser::ToU32String", stream );

        if ( !probe )
            return nullptr;

        result.push_back ( probe->_character );
        stream = probe->_newStream;
    }

    return std::move ( result );
}

std::optional<std::string> UTF8Parser::ToUTF8 ( std::u32string_view string ) noexcept
{
    std::string result {};
    result.reserve ( string.size () * 6U );

    constexpr uint32_t a = 0x0000'0080U;

    auto const get = [] ( char32_t c, uint32_t prefix, uint32_t suffix, uint32_t shift ) noexcept -> char {
        return static_cast<char> ( prefix | ( ( static_cast<uint32_t> ( c ) & suffix ) >> shift ) );
    };

    auto const get3 = [ get ] ( char32_t c ) noexcept -> char {
        return get ( c, a, 0x00FC'0000U, 18U );
    };

    auto const get2 = [ get ] ( char32_t c ) noexcept -> char {
        return get ( c, a, 0x0003'F000U, 12U );
    };

    auto const get1 = [ get ] ( char32_t c ) noexcept -> char {
        return get ( c, a, 0x0000'0FC0U, 6U );
    };

    auto const get0 = [] ( char32_t c ) noexcept -> char {
        return static_cast<char> ( static_cast<uint32_t> ( a ) | ( c & 0x0000'003FU ) );
    };

    for ( auto const c : string )
    {
        if ( c < 0x0000'0080U )
        {
            result.push_back ( static_cast<char> ( c ) );
            continue;
        }

        if ( c < 0x0000'0800U )
        {
            result.push_back ( get ( c, 0x0000'00C0U, 0x0000'07C0U, 6U ) );
            result.push_back ( get0 ( c ) );
            continue;
        }

        if ( c < 0x0001'0000U )
        {
            result.push_back ( get ( c, 0x0000'00E0U, 0x0000'F000U, 12U ) );
            result.push_back ( get1 ( c ) );
            result.push_back ( get0 ( c ) );
            continue;
        }

        if ( c < 0x0020'0000U )
        {
            result.push_back ( get ( c, 0x0000'00F0U, 0x001C'0000U, 18U ) );
            result.push_back ( get2 ( c ) );
            result.push_back ( get1 ( c ) );
            result.push_back ( get0 ( c ) );
            continue;
        }

        if ( c < 0x0400'0000U )
        {
            result.push_back ( get ( c, 0x0000'00F8U, 0x001C'0000U, 24U ) );
            result.push_back ( get3 ( c ) );
            result.push_back ( get2 ( c ) );
            result.push_back ( get1 ( c ) );
            result.push_back ( get0 ( c ) );
            continue;
        }

        if ( c < 0x8000'0000U )
        {
            result.push_back ( get ( c, 0x0000'00FCU, 0x4000'0000U, 30U ) );
            result.push_back ( get ( c, a, 0x3F00'0000U, 24U ) );
            result.push_back ( get3 ( c ) );
            result.push_back ( get2 ( c ) );
            result.push_back ( get1 ( c ) );
            result.push_back ( get0 ( c ) );
            continue;
        }

        android_vulkan::LogError ( "pbr::ASCIIString::ToUTF8 - Unexpected symbol (code %u).",
            static_cast<uint32_t> ( c ) );

        return std::nullopt;
    }

    return result;
}

} // namespace pbr
