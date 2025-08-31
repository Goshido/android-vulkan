#include <precompiled_headers.hpp>
#include <set_text_event.hpp>


namespace editor {

void SetTextEvent::Execute () noexcept
{
    if ( std::holds_alternative<std::string_view> ( _text ) )
    {
        _element.SetText ( std::get<std::string_view> ( _text ) );
        return;
    }

    _element.SetText ( std::get<std::u32string_view> ( _text ) );
}

// FUCK - remove namespace
SetTextEvent* SetTextEvent::Create ( pbr::android::TextUIElement &element, std::string_view text ) noexcept
{
    size_t const symbols = text.size ();
    bool const isEmpty = symbols < 1U;

    constexpr size_t eventSize = sizeof ( SetTextEvent );

    size_t const contentSize = symbols + 1U;
    size_t const cases[] = { eventSize + contentSize, eventSize };
    auto* data = static_cast<uint8_t*> ( std::malloc ( cases[ static_cast<size_t> ( isEmpty ) ] ) );
    auto* contentData = reinterpret_cast<char*> ( data + eventSize );

    if ( !isEmpty ) [[likely]]
        std::memcpy ( contentData, text.data (), contentSize );

    return new ( data ) SetTextEvent ( element, { contentData, symbols } );
}

// FUCK - remove namespace
SetTextEvent* SetTextEvent::Create ( pbr::android::TextUIElement &element, std::u32string_view text ) noexcept
{
    size_t const symbols = text.size ();
    bool const isEmpty = symbols < 1U;

    constexpr size_t eventSize = sizeof ( SetTextEvent );

    // Respect hardware natural alignment of the char32_t type.
    constexpr size_t natural = sizeof ( char32_t );
    constexpr size_t alignedOffset = ( eventSize + natural - 1U ) / natural * natural;

    size_t const contentSize = sizeof ( char32_t ) * ( symbols + 1U );
    size_t const cases[] = { alignedOffset + contentSize, eventSize };
    auto* data = static_cast<uint8_t*> ( std::malloc ( cases[ static_cast<size_t> ( isEmpty ) ] ) );
    auto* contentData = reinterpret_cast<char32_t*> ( data + alignedOffset );

    if ( !isEmpty ) [[likely]]
        std::memcpy ( contentData, text.data (), contentSize );

    return new ( data ) SetTextEvent ( element, { contentData, symbols } );
}

void SetTextEvent::Destroy ( SetTextEvent &event ) noexcept
{
    std::free ( &event );
}

// FUCK - remove namespace
SetTextEvent::SetTextEvent ( pbr::android::TextUIElement &element, std::string_view text ) noexcept:
    _element ( element ),
    _text ( text )
{
    // NOTHING
}

// FUCK - remove namespace
SetTextEvent::SetTextEvent ( pbr::android::TextUIElement &element, std::u32string_view text ) noexcept:
    _element ( element ),
    _text ( text )
{
    // NOTHING
}

} // namespace editor
