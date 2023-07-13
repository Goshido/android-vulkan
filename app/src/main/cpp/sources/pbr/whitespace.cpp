#include <pbr/whitespace.hpp>
#include <pbr/utf8_parser.hpp>


namespace pbr {

namespace {

constexpr char32_t CARRIAGE_RETURN = 0x0000'000DU;
constexpr char32_t FORM_FEED = 0x0000'000CU;
constexpr char32_t LINE_FEED = 0x0000'000AU;
constexpr char32_t TAB = 0x0000'0009U;
constexpr char32_t SPACE = 0x0000'0020U;

} // end of anonymous namespace

std::unordered_set<char32_t> const Whitespace::_whitespaces =
{
    CARRIAGE_RETURN,
    FORM_FEED,
    LINE_FEED,
    TAB,
    SPACE
};

bool Whitespace::IsWhitespace ( char32_t character ) noexcept
{
    return _whitespaces.count ( character ) > 0U;
}

ParseResult Whitespace::Skip ( char const* where, Stream stream ) noexcept
{
    for ( ; ; )
    {
        if ( stream._data.empty () )
            return stream;

        auto const probe = UTF8Parser::Parse ( where, stream );

        if ( !probe )
            return std::nullopt;

        if ( _whitespaces.count ( probe->_character ) < 1U )
            return stream;

        stream = probe->_newStream;
    }
}

std::u32string_view Whitespace::Skip ( std::u32string_view stream ) noexcept
{
    for ( ; ; )
    {
        if ( stream.empty () )
            return stream;

        if ( _whitespaces.count ( stream.front () ) < 1U )
            return stream;

        stream = stream.substr ( 1U );
    }
}

} // namespace pbr
