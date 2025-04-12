#include <precompiled_headers.hpp>
#include <set_text_event.hpp>


namespace editor {

void SetTextEvent::Apply () noexcept
{
    _element.SetText ( _text );
}

SetTextEvent* SetTextEvent::Create ( pbr::TextUIElement &element, std::string_view text ) noexcept
{
    bool const isEmpty = text.empty ();
    size_t const symbols = text.size ();
    size_t const cases[] = { symbols + 1U, 0U };

    constexpr size_t offset = sizeof ( SetTextEvent );
    auto* data = static_cast<uint8_t*> ( std::malloc ( offset + cases[ static_cast<size_t> ( isEmpty ) ] ) );

    if ( !isEmpty ) [[likely]]
        std::memcpy ( data + offset, text.data (), cases[ 0U ] );

    return new ( data ) SetTextEvent ( element, { reinterpret_cast<char const*> ( data + offset ), symbols } );
}

void SetTextEvent::Destroy ( SetTextEvent &event ) noexcept
{
    std::free ( &event );
}

SetTextEvent::SetTextEvent ( pbr::TextUIElement &element, std::string_view text ) noexcept:
    _element ( element ),
    _text ( text )
{
    // NOTHING
}

} // namespace editor
