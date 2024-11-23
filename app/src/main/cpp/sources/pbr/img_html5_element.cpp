#include <precompiled_headers.hpp>
#include <pbr/image_attribute_checker.hpp>
#include <pbr/img_html5_element.hpp>
#include <pbr/set_attribute_checker.hpp>
#include <pbr/unique_attribute_checker.hpp>
#include <pbr/utf8_parser.hpp>
#include <pbr/whitespace.hpp>
#include <logger.hpp>


namespace pbr {

IMGHTML5Element::IMGHTML5Element ( std::u32string &&id,
    std::unordered_set<std::u32string> &&classes,
    std::string &&assetPath
) noexcept:
    HTML5Element ( HTML5Tag::eTag::IMG ),
    _assetPath ( std::move ( assetPath ) ),
    _classes ( std::move ( classes ) ),
    _id ( std::move ( id ) )
{
    _cssComputedValues._backgroundColor = ColorValue ( false, GXColorUNORM ( 0U, 0U, 0U, 0U ) );
    _cssComputedValues._backgroundSize = LengthValue ( LengthValue::eType::Percent, 100.0F );

    _cssComputedValues._bottom = LengthValue ( LengthValue::eType::Auto, 0.0F );
    _cssComputedValues._left = LengthValue ( LengthValue::eType::Auto, 0.0F );
    _cssComputedValues._right = LengthValue ( LengthValue::eType::Auto, 0.0F );
    _cssComputedValues._top = LengthValue ( LengthValue::eType::Auto, 0.0F );

    _cssComputedValues._color = ColorValue ( true, GXColorUNORM ( 0U, 0U, 0U, 0U ) );
    _cssComputedValues._display = DisplayProperty::eValue::InlineBlock;

    _cssComputedValues._fontFile.clear ();
    _cssComputedValues._fontSize = LengthValue ( LengthValue::eType::EM, 1.0F );

    _cssComputedValues._marginBottom = LengthValue ( LengthValue::eType::PX, 0.0F );
    _cssComputedValues._marginLeft = LengthValue ( LengthValue::eType::PX, 0.0F );
    _cssComputedValues._marginRight = LengthValue ( LengthValue::eType::PX, 0.0F );
    _cssComputedValues._marginTop = LengthValue ( LengthValue::eType::PX, 0.0F );

    _cssComputedValues._paddingBottom = LengthValue ( LengthValue::eType::PX, 0.0F );
    _cssComputedValues._paddingLeft = LengthValue ( LengthValue::eType::PX, 0.0F );
    _cssComputedValues._paddingRight = LengthValue ( LengthValue::eType::PX, 0.0F );
    _cssComputedValues._paddingTop = LengthValue ( LengthValue::eType::PX, 0.0F );

    _cssComputedValues._position = PositionProperty::eValue::Static;
    _cssComputedValues._textAlign = TextAlignProperty::eValue::Left;
    _cssComputedValues._verticalAlign = VerticalAlignProperty::eValue::Top;

    _cssComputedValues._width = LengthValue ( LengthValue::eType::Auto, 0.0F );
    _cssComputedValues._height = LengthValue ( LengthValue::eType::Auto, 0.0F );
}

std::string &IMGHTML5Element::GetAssetPath () noexcept
{
    return _assetPath;
}

std::u32string &IMGHTML5Element::GetID () noexcept
{
    return _id;
}

std::optional<HTML5Element::Result> IMGHTML5Element::Parse ( char const* html,
    Stream stream,
    char const* assetRoot,
    std::unordered_set<std::u32string> &idRegistry
) noexcept
{
    std::u32string id {};
    UniqueAttributeChecker idChecker ( html, eAttribute::ID, id, idRegistry );

    std::unordered_set<std::u32string> classes {};
    SetAttributeChecker classChecker ( html, eAttribute::Class, classes );

    std::string assetPath {};
    ImageAttributeChecker imageChecker ( html, assetPath, assetRoot );

    for ( ; ; )
    {
        if ( !stream.ExpectNotEmpty ( html, "pbr::IMGHTML5Element::Parse" ) )
            return std::nullopt;

        auto const skip = Whitespace::Skip ( html, stream );

        if ( !skip )
            return std::nullopt;

        stream = *skip;
        auto att = AttributeParser::Parse ( html, stream );

        if ( !att )
            return std::nullopt;

        eAttribute const a = att->_attribute;

        if ( a == eAttribute::No_Attribute )
            break;

        auto action = idChecker.Process ( *att );

        if ( !action )
            return std::nullopt;

        if ( *action )
        {
            stream = att->_newStream;
            continue;
        }

        action = classChecker.Process ( *att );

        if ( !action )
            return std::nullopt;

        if ( *action )
        {
            stream = att->_newStream;
            continue;
        }

        action = imageChecker.Process ( *att );

        if ( !action )
            return std::nullopt;

        if ( *action )
        {
            stream = att->_newStream;
            continue;
        }

        android_vulkan::LogError ( "pbr::IMGHTML5Element::Parse - %s:%zu: Unsupported attribute detected '%s' for "
            "'img' element",
            html,
            stream._line,
            AttributeParser::ToString ( a )
        );

        return std::nullopt;
    }

    size_t const rest = stream._data.size ();

    if ( rest < 2U )
    {
        android_vulkan::LogError ( "pbr::IMGHTML5Element::Parse - %s:%zu: Unexpected stream end.",
            html,
            stream._line
        );

        return std::nullopt;
    }

    uint8_t const* d = stream._data.data ();
    constexpr char const end[] = { '/', '>' };

    if ( std::memcmp ( d, end, std::size ( end ) ) == 0 )
    {
        return Result
        {
            ._element = std::make_shared<IMGHTML5Element> ( std::move ( id ),
                std::move ( classes ),
                std::move ( assetPath )
            ),

            ._newStream = Stream ( stream._data.subspan ( 2U ), stream._line )
        };
    }

    if ( *d != static_cast<uint8_t> ( '>' ) )
    {
        android_vulkan::LogError ( "pbr::IMGHTML5Element::Parse - %s:%zu: 'img' element should be closed by '>' or "
            "'/>'.",
            html,
            stream._line
        );

        return std::nullopt;
    }

    return Result
    {
        ._element = std::make_shared<IMGHTML5Element> ( std::move ( id ),
            std::move ( classes ),
            std::move ( assetPath )
        ),

        ._newStream = Stream ( stream._data.subspan ( 1U ), stream._line )
    };
}

bool IMGHTML5Element::ApplyCSS ( char const* html, CSSParser const &css ) noexcept
{
    return _cssComputedValues.ApplyCSS ( html, css, _classes, _id );
}

} // namespace pbr
