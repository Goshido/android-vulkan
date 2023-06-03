#include <av_assert.h>
#include <logger.h>
#include <pbr/color_property.h>
#include <pbr/css_computed_values.h>
#include <pbr/font_family_property.h>
#include <pbr/length_property.h>
#include <pbr/utf8_parser.h>


namespace pbr {

namespace {

class ApplyHandlers final
{
    private:
        using Handler = bool ( * ) ( char const* html,
            CSSComputedValues &target,
            Property const& property,
            char const* kind,
            std::u32string const &name,
            CSSParser const &css
        ) noexcept;

    private:
        static Handler      _handlers[ Property::GetTypeCount () ];

    public:
        ApplyHandlers () noexcept;

        ApplyHandlers ( ApplyHandlers const & ) = delete;
        ApplyHandlers& operator = ( ApplyHandlers const & ) = delete;

        ApplyHandlers ( ApplyHandlers && ) = delete;
        ApplyHandlers& operator = ( ApplyHandlers && ) = delete;

        ~ApplyHandlers () = default;

        [[nodiscard]] static bool Handle ( char const* html,
            CSSComputedValues &target,
            Property const& property,
            char const* kind,
            std::u32string const &name,
            CSSParser const &css
        ) noexcept;

    private:
        [[nodiscard]] static bool HandleBackgroundColor ( char const* html,
            CSSComputedValues &target,
            Property const &property,
            char const* kind,
            std::u32string const &name,
            CSSParser const &css
        ) noexcept;

        [[nodiscard]] static bool HandleBackgroundSize ( char const* html,
            CSSComputedValues &target,
            Property const &property,
            char const* kind,
            std::u32string const &name,
            CSSParser const &css
        ) noexcept;

        [[nodiscard]] static bool HandleColor ( char const* html,
            CSSComputedValues &target,
            Property const &property,
            char const* kind,
            std::u32string const &name,
            CSSParser const &css
        ) noexcept;

        [[nodiscard]] static bool HandleDisplay ( char const* html,
            CSSComputedValues &target,
            Property const &property,
            char const* kind,
            std::u32string const &name,
            CSSParser const &css
        ) noexcept;

        [[nodiscard]] static bool HandleFail ( char const* html,
            CSSComputedValues &target,
            Property const &property,
            char const* kind,
            std::u32string const &name,
            CSSParser const &css
        ) noexcept;

        [[nodiscard]] static bool HandleFontFamily ( char const* html,
            CSSComputedValues &target,
            Property const &property,
            char const* kind,
            std::u32string const &name,
            CSSParser const &css
        ) noexcept;

        [[nodiscard]] static bool HandleFontSize ( char const* html,
            CSSComputedValues &target,
            Property const &property,
            char const* kind,
            std::u32string const &name,
            CSSParser const &css
        ) noexcept;

        [[nodiscard]] static bool HandleHeight ( char const* html,
            CSSComputedValues &target,
            Property const &property,
            char const* kind,
            std::u32string const &name,
            CSSParser const &css
        ) noexcept;

        [[nodiscard]] static bool HandleMarginBottom ( char const* html,
            CSSComputedValues &target,
            Property const &property,
            char const* kind,
            std::u32string const &name,
            CSSParser const &css
        ) noexcept;

        [[nodiscard]] static bool HandleMarginLeft ( char const* html,
            CSSComputedValues &target,
            Property const &property,
            char const* kind,
            std::u32string const &name,
            CSSParser const &css
        ) noexcept;

        [[nodiscard]] static bool HandleMarginRight ( char const* html,
            CSSComputedValues &target,
            Property const &property,
            char const* kind,
            std::u32string const &name,
            CSSParser const &css
        ) noexcept;

        [[nodiscard]] static bool HandleMarginTop ( char const* html,
            CSSComputedValues &target,
            Property const &property,
            char const* kind,
            std::u32string const &name,
            CSSParser const &css
        ) noexcept;

        [[nodiscard]] static bool HandlePaddingBottom ( char const* html,
            CSSComputedValues &target,
            Property const &property,
            char const* kind,
            std::u32string const &name,
            CSSParser const &css
        ) noexcept;

        [[nodiscard]] static bool HandlePaddingLeft ( char const* html,
            CSSComputedValues &target,
            Property const &property,
            char const* kind,
            std::u32string const &name,
            CSSParser const &css
        ) noexcept;

        [[nodiscard]] static bool HandlePaddingRight ( char const* html,
            CSSComputedValues &target,
            Property const &property,
            char const* kind,
            std::u32string const &name,
            CSSParser const &css
        ) noexcept;

        [[nodiscard]] static bool HandlePaddingTop ( char const* html,
            CSSComputedValues &target,
            Property const &property,
            char const* kind,
            std::u32string const &name,
            CSSParser const &css
        ) noexcept;

        [[nodiscard]] static bool HandlePosition ( char const* html,
            CSSComputedValues &target,
            Property const &property,
            char const* kind,
            std::u32string const &name,
            CSSParser const &css
        ) noexcept;

        [[nodiscard]] static bool HandleTextAlign ( char const* html,
            CSSComputedValues &target,
            Property const &property,
            char const* kind,
            std::u32string const &name,
            CSSParser const &css
        ) noexcept;

        [[nodiscard]] static bool HandleVerticalAlign ( char const* html,
            CSSComputedValues &target,
            Property const &property,
            char const* kind,
            std::u32string const &name,
            CSSParser const &css
        ) noexcept;

        [[nodiscard]] static bool HandleWidth ( char const* html,
            CSSComputedValues &target,
            Property const &property,
            char const* kind,
            std::u32string const &name,
            CSSParser const &css
        ) noexcept;
};

ApplyHandlers::Handler ApplyHandlers::_handlers[ Property::GetTypeCount () ] = {};

ApplyHandlers::ApplyHandlers () noexcept
{
    _handlers[ static_cast<size_t> ( Property::eType::BackgroundColor ) ] = &ApplyHandlers::HandleBackgroundColor;
    _handlers[ static_cast<size_t> ( Property::eType::BackgroundSize ) ] = &ApplyHandlers::HandleBackgroundSize;
    _handlers[ static_cast<size_t> ( Property::eType::Color ) ] = &ApplyHandlers::HandleColor;
    _handlers[ static_cast<size_t> ( Property::eType::Display ) ] = &ApplyHandlers::HandleDisplay;
    _handlers[ static_cast<size_t> ( Property::eType::FontFamily ) ] = &ApplyHandlers::HandleFontFamily;
    _handlers[ static_cast<size_t> ( Property::eType::FontSize ) ] = &ApplyHandlers::HandleFontSize;
    _handlers[ static_cast<size_t> ( Property::eType::Height ) ] = &ApplyHandlers::HandleHeight;
    _handlers[ static_cast<size_t> ( Property::eType::Margin ) ] = &ApplyHandlers::HandleFail;
    _handlers[ static_cast<size_t> ( Property::eType::MarginBottom ) ] = &ApplyHandlers::HandleMarginBottom;
    _handlers[ static_cast<size_t> ( Property::eType::MarginLeft ) ] = &ApplyHandlers::HandleMarginLeft;
    _handlers[ static_cast<size_t> ( Property::eType::MarginRight ) ] = &ApplyHandlers::HandleMarginRight;
    _handlers[ static_cast<size_t> ( Property::eType::MarginTop ) ] = &ApplyHandlers::HandleMarginTop;
    _handlers[ static_cast<size_t> ( Property::eType::Padding ) ] = &ApplyHandlers::HandleFail;
    _handlers[ static_cast<size_t> ( Property::eType::PaddingBottom ) ] = &ApplyHandlers::HandlePaddingBottom;
    _handlers[ static_cast<size_t> ( Property::eType::PaddingLeft ) ] = &ApplyHandlers::HandlePaddingLeft;
    _handlers[ static_cast<size_t> ( Property::eType::PaddingRight ) ] = &ApplyHandlers::HandlePaddingRight;
    _handlers[ static_cast<size_t> ( Property::eType::PaddingTop ) ] = &ApplyHandlers::HandlePaddingTop;
    _handlers[ static_cast<size_t> ( Property::eType::Position ) ] = &ApplyHandlers::HandlePosition;
    _handlers[ static_cast<size_t> ( Property::eType::SRC ) ] = &ApplyHandlers::HandleFail;
    _handlers[ static_cast<size_t> ( Property::eType::TextAlign ) ] = &ApplyHandlers::HandleTextAlign;
    _handlers[ static_cast<size_t> ( Property::eType::VerticalAlign ) ] = &ApplyHandlers::HandleVerticalAlign;
    _handlers[ static_cast<size_t> ( Property::eType::Width ) ] = &ApplyHandlers::HandleWidth;
}

bool ApplyHandlers::Handle ( char const* html,
    CSSComputedValues &target,
    Property const& property,
    char const* kind,
    std::u32string const &name,
    CSSParser const &css
) noexcept
{
    return _handlers[ static_cast<size_t> ( property.GetType () ) ] ( html, target, property, kind, name, css );
}

bool ApplyHandlers::HandleBackgroundColor ( char const* /*html*/,
    CSSComputedValues &target,
    Property const &property,
    char const* /*kind*/,
    std::u32string const &/*name*/,
    CSSParser const &/*css*/
) noexcept
{
    // NOLINTNEXTLINE - downcast.
    target._backgroundColor = static_cast<ColorProperty const&> ( property ).GetValue ();
    return true;
}

bool ApplyHandlers::HandleBackgroundSize ( char const* /*html*/,
    CSSComputedValues &target,
    Property const &property,
    char const* /*kind*/,
    std::u32string const &/*name*/,
    CSSParser const &/*css*/
) noexcept
{
    // NOLINTNEXTLINE - downcast.
    target._backgroundSize = static_cast<LengthProperty const&> ( property ).GetValue ();
    return true;
}

bool ApplyHandlers::HandleColor ( char const* /*html*/,
    CSSComputedValues &target,
    Property const &property,
    char const* /*kind*/,
    std::u32string const &/*name*/,
    CSSParser const &/*css*/
) noexcept
{
    // NOLINTNEXTLINE - downcast.
    target._color = static_cast<ColorProperty const&> ( property ).GetValue ();
    return true;
}

bool ApplyHandlers::HandleDisplay ( char const* /*html*/,
    CSSComputedValues &target,
    Property const &property,
    char const* /*kind*/,
    std::u32string const &/*name*/,
    CSSParser const &/*css*/
) noexcept
{
    // NOLINTNEXTLINE - downcast.
    target._display = static_cast<DisplayProperty const&> ( property ).GetValue ();
    return true;
}

bool ApplyHandlers::HandleFail ( char const* html,
    CSSComputedValues &/*target*/,
    Property const &property,
    char const* kind,
    std::u32string const &name,
    CSSParser const &css
) noexcept
{
    android_vulkan::LogError ( "pbr::ApplyHandlers::HandleFail - %s: Shouild not be possible to get '%zu' "
        "property type ID here. Check CSS %s '%s' inside file '%s'. Check ApplyHandlers implementaion. "
        "Maybe missing something?",
        html,
        static_cast<size_t> ( property.GetType () ),
        kind,
        UTF8Parser::ToUTF8 ( name )->c_str (),
        css.GetSource ().c_str ()
    );

    assert ( false );
    return false;
}

bool ApplyHandlers::HandleFontFamily ( char const* html,
    CSSComputedValues &target,
    Property const &property,
    char const* kind,
    std::u32string const &name,
    CSSParser const &css
) noexcept
{
    // NOLINTNEXTLINE - downcast.
    std::u32string const& fontFamily = static_cast<FontFamilyProperty const&> ( property ).GetValue ();

    auto const& fontFile = css.FindFontFile ( fontFamily );

    if ( !fontFile )
    {
        android_vulkan::LogError ( "pbr::ApplyHandlers::HandleFontFamily - %s: Element references to '%s' "
            "font family which does not exist. Check CSS %s '%s' inside file '%s'.",
            html,
            UTF8Parser::ToUTF8 ( fontFamily )->c_str (),
            kind,
            UTF8Parser::ToUTF8 ( name )->c_str (),
            css.GetSource ().c_str ()
        );

        return false;
    }

    target._fontFile = *fontFile.value ();
    return true;
}

bool ApplyHandlers::HandleFontSize ( char const* /*html*/,
    CSSComputedValues &target,
    Property const &property,
    char const* /*kind*/,
    std::u32string const &/*name*/,
    CSSParser const &/*css*/
) noexcept
{
    // NOLINTNEXTLINE - downcast.
    target._fontSize = static_cast<LengthProperty const&> ( property ).GetValue ();
    return true;
}

bool ApplyHandlers::HandleHeight ( char const* /*html*/,
    CSSComputedValues &target,
    Property const &property,
    char const* /*kind*/,
    std::u32string const &/*name*/,
    CSSParser const &/*css*/
) noexcept
{
    // NOLINTNEXTLINE - downcast.
    target._height = static_cast<LengthProperty const&> ( property ).GetValue ();
    return true;
}

bool ApplyHandlers::HandleMarginBottom ( char const* /*html*/,
    CSSComputedValues &target,
    Property const &property,
    char const* /*kind*/,
    std::u32string const &/*name*/,
    CSSParser const &/*css*/
) noexcept
{
    // NOLINTNEXTLINE - downcast.
    target._marginBottom = static_cast<LengthProperty const&> ( property ).GetValue ();
    return true;
}

bool ApplyHandlers::HandleMarginLeft ( char const* /*html*/,
    CSSComputedValues &target,
    Property const &property,
    char const* /*kind*/,
    std::u32string const &/*name*/,
    CSSParser const &/*css*/
) noexcept
{
    // NOLINTNEXTLINE - downcast.
    target._marginLeft = static_cast<LengthProperty const&> ( property ).GetValue ();
    return true;
}

bool ApplyHandlers::HandleMarginRight ( char const* /*html*/,
    CSSComputedValues &target,
    Property const &property,
    char const* /*kind*/,
    std::u32string const &/*name*/,
    CSSParser const &/*css*/
) noexcept
{
    // NOLINTNEXTLINE - downcast.
    target._marginRight = static_cast<LengthProperty const&> ( property ).GetValue ();
    return true;
}

bool ApplyHandlers::HandleMarginTop ( char const* /*html*/,
    CSSComputedValues &target,
    Property const &property,
    char const* /*kind*/,
    std::u32string const &/*name*/,
    CSSParser const &/*css*/
) noexcept
{
    // NOLINTNEXTLINE - downcast.
    target._marginTop = static_cast<LengthProperty const&> ( property ).GetValue ();
    return true;
}

bool ApplyHandlers::HandlePaddingBottom ( char const* /*html*/,
    CSSComputedValues &target,
    Property const &property,
    char const* /*kind*/,
    std::u32string const &/*name*/,
    CSSParser const &/*css*/
) noexcept
{
    // NOLINTNEXTLINE - downcast.
    target._paddingBottom = static_cast<LengthProperty const&> ( property ).GetValue ();
    return true;
}

bool ApplyHandlers::HandlePaddingLeft ( char const* /*html*/,
    CSSComputedValues &target,
    Property const &property,
    char const* /*kind*/,
    std::u32string const &/*name*/,
    CSSParser const &/*css*/
) noexcept
{
    // NOLINTNEXTLINE - downcast.
    target._paddingLeft = static_cast<LengthProperty const&> ( property ).GetValue ();
    return true;
}

bool ApplyHandlers::HandlePaddingRight ( char const* /*html*/,
    CSSComputedValues &target,
    Property const &property,
    char const* /*kind*/,
    std::u32string const &/*name*/,
    CSSParser const &/*css*/
) noexcept
{
    // NOLINTNEXTLINE - downcast.
    target._paddingRight = static_cast<LengthProperty const&> ( property ).GetValue ();
    return true;
}

bool ApplyHandlers::HandlePaddingTop ( char const* /*html*/,
    CSSComputedValues &target,
    Property const &property,
    char const* /*kind*/,
    std::u32string const &/*name*/,
    CSSParser const &/*css*/
) noexcept
{
    // NOLINTNEXTLINE - downcast.
    target._paddingTop = static_cast<LengthProperty const&> ( property ).GetValue ();
    return true;
}

bool ApplyHandlers::HandlePosition ( char const* /*html*/,
    CSSComputedValues &target,
    Property const &property,
    char const* /*kind*/,
    std::u32string const &/*name*/,
    CSSParser const &/*css*/
) noexcept
{
    // NOLINTNEXTLINE - downcast.
    target._position = static_cast<PositionProperty const&> ( property ).GetValue ();
    return true;
}

bool ApplyHandlers::HandleTextAlign ( char const* /*html*/,
    CSSComputedValues &target,
    Property const &property,
    char const* /*kind*/,
    std::u32string const &/*name*/,
    CSSParser const &/*css*/
) noexcept
{
    // NOLINTNEXTLINE - downcast.
    target._textAlign = static_cast<TextAlignProperty const&> ( property ).GetValue ();
    return true;
}

bool ApplyHandlers::HandleVerticalAlign ( char const* /*html*/,
    CSSComputedValues &target,
    Property const &property,
    char const* /*kind*/,
    std::u32string const &/*name*/,
    CSSParser const &/*css*/
) noexcept
{
    // NOLINTNEXTLINE - downcast.
    target._verticalAlign = static_cast<VerticalAlignProperty const&> ( property ).GetValue ();
    return true;
}

bool ApplyHandlers::HandleWidth ( char const* /*html*/,
    CSSComputedValues &target,
    Property const &property,
    char const* /*kind*/,
    std::u32string const &/*name*/,
    CSSParser const &/*css*/
) noexcept
{
    // NOLINTNEXTLINE - downcast.
    target._width = static_cast<LengthProperty const&> ( property ).GetValue ();
    return true;
}

ApplyHandlers const g_ApplyHandlers {};

} // end of anonymous namespace

//----------------------------------------------------------------------------------------------------------------------

bool CSSComputedValues::ApplyCSS ( char const* html,
    CSSParser const &css,
    std::unordered_set<std::u32string> const &classes,
    std::u32string const &id
) noexcept
{
    for ( auto const& cls : classes )
    {
        auto const target = css.FindClass ( cls );

        if ( !target )
        {
            android_vulkan::LogError ( "pbr::CSSComputedValues::ApplyCSS - %s: Element references to '%s' CSS class "
                "which does not exist. Check CSS file '%s'",
                html,
                UTF8Parser::ToUTF8 ( cls )->c_str (),
                css.GetSource ().c_str ()
            );

            return false;
        }

        auto const& props = *target.value ();

        for ( auto const& prop : props )
        {
            if ( !g_ApplyHandlers.Handle ( html, *this, *prop, "class", cls, css ) )
            {
                return false;
            }
        }
    }

    auto const target = css.FindID ( id );

    if ( !target )
        return true;

    auto const& props = *target.value ();

    for ( auto const& prop : props )
    {
        if ( !g_ApplyHandlers.Handle ( html, *this, *prop, "ID", id, css ) )
        {
            return false;
        }
    }

    return true;
}

} // namespace pbr
