#include <precompiled_headers.hpp>
#include <logger.hpp>
#include <pbr/html5_tag.hpp>
#include <pbr/ascii_string.hpp>


namespace pbr {

std::unordered_map<std::string_view, HTML5Tag> const HTML5Tag::_tags =
{
    { "body", HTML5Tag ( eTag::Body ) },
    { "div", HTML5Tag ( eTag::DIV ) },
    { "doctype", HTML5Tag ( eTag::Doctype ) },
    { "html", HTML5Tag ( eTag::HTML ) },
    { "head", HTML5Tag ( eTag::Head ) },
    { "img", HTML5Tag ( eTag::IMG ) },
    { "link", HTML5Tag ( eTag::Link ) }
};

std::unordered_map<HTML5Tag::eTag, std::string_view> const HTML5Tag::_names =
{
    { HTML5Tag::eTag::Body, "body" },
    { HTML5Tag::eTag::DIV, "div" },
    { HTML5Tag::eTag::Doctype, "doctype" },
    { HTML5Tag::eTag::HTML, "html" },
    { HTML5Tag::eTag::Head, "head" },
    { HTML5Tag::eTag::IMG, "img" },
    { HTML5Tag::eTag::Link, "link" },
    { HTML5Tag::eTag::Text, "text" }
};

bool HTML5Tag::operator == ( eTag tag ) const noexcept
{
    return _tag == tag;
}

std::optional<HTML5Tag> HTML5Tag::Parse ( std::string_view string ) noexcept
{
    ASCIIString::ToLower ( string );
    auto const i = _tags.find ( string );

    if ( i == _tags.cend () )
        return std::nullopt;

    return HTML5Tag ( i->second );
}

char const* HTML5Tag::ToString () const noexcept
{
    return _names.find ( _tag )->second.data ();
}

} // namespace pbr
