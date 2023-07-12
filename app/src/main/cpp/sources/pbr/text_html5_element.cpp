#include <pbr/text_html5_element.h>
#include <pbr/utf8_parser.h>
#include <pbr/whitespace.h>


namespace pbr {

TextHTML5Element::TextHTML5Element ( std::u32string &&text ) noexcept:
    HTML5Element ( HTML5Tag::eTag::Text ),
    _text ( std::move ( text ) )
{
    // NOTHING
}

std::u32string &TextHTML5Element::GetText () noexcept
{
    return _text;
}

std::optional<HTML5Element::Result> TextHTML5Element::Parse (  char const* html, Stream stream ) noexcept
{
    std::u32string text {};

    for ( ; ; )
    {
        if ( !stream.ExpectNotEmpty ( html, "pbr::TextHTML5Element::Parse" ) )
            return std::nullopt;

        auto const probe = UTF8Parser::Parse ( html, stream );

        if ( !probe )
            return std::nullopt;

        char32_t const c = probe->_character;

        if ( Whitespace::IsWhitespace ( probe->_character ) )
        {
            auto const skip = Whitespace::Skip ( html, probe->_newStream );

            if ( !skip )
                return std::nullopt;

            text.push_back ( U' ' );
            stream = *skip;
            continue;
        }

        if ( c != U'<' )
        {
            text.push_back ( c );
            stream = probe->_newStream;
            continue;
        }

        if ( text.back () == U' ' )
            text.pop_back ();

        return Result
        {
            ._element = std::make_shared<TextHTML5Element> ( std::move ( text ) ),
            ._newStream = stream
        };
    }
}

} // namespace pbr
