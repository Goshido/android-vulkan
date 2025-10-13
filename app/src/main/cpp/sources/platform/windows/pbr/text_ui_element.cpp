#include <precompiled_headers.hpp>
#include <platform/windows/pbr/text_ui_element.hpp>


namespace pbr {

TextUIElement::TextUIElement ( bool visible, UIElement const* parent, std::u32string &&text ) noexcept:
    TextUIElementBase ( visible, parent, std::move ( text ) )
{
    // NOTHING
}

TextUIElement::TextUIElement ( bool visible,
    UIElement const* parent,
    std::u32string &&text,
    std::string &&name
) noexcept:
    TextUIElementBase ( visible, parent, std::move ( text ), std::move ( name ) )
{
    // NOTHING
}

TextUIElement::TextUIElement ( bool visible, UIElement const* parent, std::string_view text ) noexcept:
    TextUIElementBase ( visible, parent, text )
{
    // NOTHING
}

TextUIElement::TextUIElement ( bool visible,
    UIElement const* parent,
    std::string_view text,
    std::string &&name
) noexcept:
    TextUIElementBase ( visible, parent, std::move ( text ), std::move ( name ) )
{
    // NOTHING
}

void TextUIElement::OnCacheUpdated ( std::span<TextGlyph> glyphs ) noexcept
{
    if ( _atlasPromise.empty () ) [[likely]]
        return;

    TextGlyph* dst = glyphs.data ();
    size_t const count = glyphs.size ();
    uint16_t const** promise = _atlasPromise.data ();

    for ( size_t i = 0U; i < count; ++i )
    {
        dst[ i ]._atlasPage = *promise[ i ];
    }
}

void TextUIElement::OnGlyphAdded ( GlyphInfo const &glyphInfo ) noexcept
{
    _atlasPromise.emplace_back ( &glyphInfo._pageID );
}

void TextUIElement::OnGlyphCleared () noexcept
{
    _atlasPromise.clear ();
}

void TextUIElement::OnGlyphResized ( size_t count ) noexcept
{
    _atlasPromise.resize ( count );
}

} // namespace pbr
