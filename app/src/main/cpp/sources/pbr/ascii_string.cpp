#include <logger.hpp>
#include <pbr/ascii_string.hpp>
#include <pbr/utf8_parser.hpp>

GX_DISABLE_COMMON_WARNINGS

#include <cctype>

GX_RESTORE_WARNING_STATE


namespace pbr {

namespace {

constexpr char32_t A = U'a';
constexpr char32_t Z = U'z';

constexpr char32_t BIG_A = U'A';
constexpr char32_t BIG_Z = U'Z';

constexpr char32_t DASH = U'-';

constexpr char32_t DIGIT_0 = U'0';
constexpr char32_t DIGIT_9 = U'9';

} // end of anonymous namespace

void ASCIIString::ToLower ( std::string_view string ) noexcept
{
    char* n = const_cast<char*> ( string.data () );
    size_t const count = string.size ();

    for ( size_t i = 0U; i < count; ++i )
    {
        char &s = n[ i ];
        s = static_cast<char> ( std::tolower ( static_cast<int> ( s ) ) );
    }
}

std::optional<ASCIIString::Result> ASCIIString::Parse ( char const* where, Stream stream, eParseMode mode ) noexcept
{
    if ( !stream.ExpectNotEmpty ( where, "pbr::ASCIIString::Parse" ) )
        return std::nullopt;

    size_t const l = stream._line;
    auto probe = UTF8Parser::Parse ( where, stream );

    if ( !probe )
        return std::nullopt;

    constexpr CheckHandler const handlers[] =
    {
        &ASCIIString::CheckAlphanumeric,
        &ASCIIString::CheckLetters,
        &ASCIIString::CheckLettersAndDashes
    };

    CheckHandler const check = handlers[ static_cast<size_t> ( mode ) ];

    if ( !check ( probe->_character ) )
    {
        android_vulkan::LogError ( "pbr::ASCIIString::Parse - %s:%zu: Incorrect start.", where, l );
        return std::nullopt;
    }

    size_t size = 1U;
    Stream s ( stream._data.subspan ( 1U ), stream._line );

    for ( ; ; )
    {
        if ( s._data.empty () )
        {
            Stream::Data const n = stream._data.subspan ( 0U, size );

            return Result
            {
                ._newStream = {},
                ._target = std::string_view ( reinterpret_cast<char const*> ( n.data () ), n.size () )
            };
        }

        probe = UTF8Parser::Parse ( where, s );

        if ( !probe )
            return std::nullopt;

        char32_t const c = probe->_character;

        if ( !check ( c ) )
        {
            Stream::Data const n = stream._data.subspan ( 0U, size );

            return Result
            {
                ._newStream = Stream ( stream._data.subspan ( size ), stream._line ),
                ._target = std::string_view ( reinterpret_cast<char const*> ( n.data () ), n.size () )
            };
        }

        ++size;
        s = Stream ( s._data.subspan ( 1U ), stream._line );
    }
}

bool ASCIIString::CheckAlphanumeric ( char32_t c ) noexcept
{
    auto const c1 = static_cast<bool> ( ( c >= A ) & ( c <= Z ) );
    auto const c2 = static_cast<bool> ( ( c >= BIG_A ) & ( c <= BIG_Z ) );
    auto const c3 = static_cast<bool> ( ( c >= DIGIT_0 ) & ( c <= DIGIT_9 ) );
    return static_cast<bool> ( c1 | c2 | c3 );
}

bool ASCIIString::CheckLetters ( char32_t c ) noexcept
{
    auto const c1 = static_cast<bool> ( ( c >= A ) & ( c <= Z ) );
    auto const c2 = static_cast<bool> ( ( c >= BIG_A ) & ( c <= BIG_Z ) );
    return static_cast<bool> ( c1 | c2 );
}

bool ASCIIString::CheckLettersAndDashes ( char32_t c ) noexcept
{
    auto const c1 = static_cast<bool> ( ( c >= A ) & ( c <= Z ) );
    auto const c2 = static_cast<bool> ( ( c >= BIG_A ) & ( c <= BIG_Z ) );
    auto const c3 = c == DASH;
    return static_cast<bool> ( c1 | c2 | c3 );
}

} // namespace pbr
