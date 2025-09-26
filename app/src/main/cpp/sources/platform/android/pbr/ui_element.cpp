#include <precompiled_headers.hpp>
#include <av_assert.hpp>
#include <logger.hpp>
#include <pbr/css_unit_to_device_pixel.hpp>
#include <platform/android/pbr/ui_element.hpp>


namespace pbr {

void UIElement::Hide () noexcept
{
    _visibilityChanged = _visible;
    _visible = false;
}

void UIElement::Show () noexcept
{
    _visibilityChanged = !_visible;
    _visible = true;
}

bool UIElement::IsVisible () const noexcept
{
    return _visible;
}

CSSComputedValues &UIElement::GetCSS () noexcept
{
    return _css;
}

CSSComputedValues const& UIElement::GetCSS () const noexcept
{
    return _css;
}

std::string_view UIElement::ResolveFont () const noexcept
{
    for ( UIElement const* p = this; p; p = p->_parent )
    {
        if ( std::string const &fontFile = p->GetCSS ()._fontFile; !fontFile.empty () )
        {
            return std::string_view ( fontFile );
        }
    }

    AV_ASSERT ( false )
    return {};
}

float UIElement::ResolveFontSize () const noexcept
{
    LengthValue const* target = nullptr;
    float relativeScale = 1.0F;
    LengthValue::eType type = LengthValue::eType::Auto;

    for ( UIElement const* p = this; ( p != nullptr ) & ( target == nullptr ); p = p->_parent )
    {
        LengthValue const &size = p->_css._fontSize;

        switch ( type = size.GetType (); type )
        {
            case LengthValue::eType::EM:
                relativeScale *= size.GetValue ();
            break;

            case LengthValue::eType::Percent:
                relativeScale *= 1.0e-2F * size.GetValue ();
            break;

            case LengthValue::eType::Inherit:
                [[fallthrough]];
            case LengthValue::eType::Auto:
                // NOTHING
            break;

            case LengthValue::eType::MM:
                [[fallthrough]];
            case LengthValue::eType::PT:
                [[fallthrough]];
            case LengthValue::eType::PX:
                target = &size;
            break;

            case LengthValue::eType::Unitless:
                // IMPOSSIBLE
                AV_ASSERT ( false )
            return 0.0F;
        }
    }

    if ( !target ) [[unlikely]]
    {
        android_vulkan::LogError ( "pbr::UIElement::ResolveFontSize - Everything is inherit." );
        AV_ASSERT ( false )
        return 0.0F;
    }

    CSSUnitToDevicePixel const &cssUnits = CSSUnitToDevicePixel::GetInstance ();

    switch ( type )
    {
        case LengthValue::eType::MM:
        return relativeScale * target->GetValue () * cssUnits._fromMM;

        case LengthValue::eType::PT:
        return relativeScale * target->GetValue () * cssUnits._fromPT;

        case LengthValue::eType::PX:
        return relativeScale * target->GetValue () * cssUnits._fromPX;

        case LengthValue::eType::Auto:
            [[fallthrough]];
        case LengthValue::eType::EM:
            [[fallthrough]];
        case LengthValue::eType::Inherit:
            [[fallthrough]];
        case LengthValue::eType::Percent:
            [[fallthrough]];
        case LengthValue::eType::Unitless:
            [[fallthrough]];
        default:
            // IMPOSSIBLE
            AV_ASSERT ( false )
        return 0.0F;
    }
}

float UIElement::ResolveLineHeight ( FontStorage::Font font) const noexcept
{
    CSSUnitToDevicePixel const &cssUnits = CSSUnitToDevicePixel::GetInstance ();
    float const rootValue = _css._lineHeight.GetValue ();

    for ( UIElement const* p = this; p; p = p->_parent )
    {
        switch ( LengthValue const &lineHeight = p->_css._lineHeight; lineHeight.GetType () )
        {
            case LengthValue::eType::Auto:
            return static_cast<float> ( FontStorage::GetFontPixelMetrics ( font )._baselineToBaseline );

            case LengthValue::eType::EM:
                [[fallthrough]];
            case LengthValue::eType::Unitless:
            return rootValue * ResolveFontSize ();

            case LengthValue::eType::Percent:
            return 1.0e-2F * rootValue * ResolveFontSize ();

            case LengthValue::eType::Inherit:
                // NOTHING
            break;

            case LengthValue::eType::MM:
            return lineHeight.GetValue () * cssUnits._fromMM;

            case LengthValue::eType::PT:
            return lineHeight.GetValue () * cssUnits._fromPT;

            case LengthValue::eType::PX:
            return lineHeight.GetValue () * cssUnits._fromPX;
        }
    }

    android_vulkan::LogError ( "pbr::UIElement::ResolveLineHeight - Everything is inherit." );
    AV_ASSERT ( false )
    return 0.0F;
}

UIElement::UIElement ( bool visible, UIElement const* parent ) noexcept:
    _visible ( visible ),
    _parent ( parent )
{
    // NOTHING
}

UIElement::UIElement ( bool visible, UIElement const* parent, std::string &&name ) noexcept:
    _name ( std::move ( name ) ),
    _visible ( visible ),
    _parent ( parent )
{
    // NOTHING
}

UIElement::UIElement ( bool visible, UIElement const* parent, CSSComputedValues &&css ) noexcept:
    _css ( std::move ( css ) ),
    _visible ( visible ),
    _parent ( parent )
{
    // NOTHING
}

UIElement::UIElement ( bool visible, UIElement const* parent, CSSComputedValues &&css, std::string &&name ) noexcept:
    _css ( std::move ( css ) ),
    _name ( std::move ( name ) ),
    _visible ( visible ),
    _parent ( parent )
{
    // NOTHING
}

float UIElement::ResolvePixelLength ( LengthValue const &length,
    float referenceLength,
    bool isHeight
) const noexcept
{
    CSSUnitToDevicePixel const &units = CSSUnitToDevicePixel::GetInstance ();

    switch ( length.GetType () )
    {
        case LengthValue::eType::EM:
        return ResolveFontSize () * length.GetValue ();

        case LengthValue::eType::MM:
        return units._fromMM * length.GetValue ();

        case LengthValue::eType::PT:
        return units._fromPT * length.GetValue ();

        case LengthValue::eType::PX:
        return units._fromPX * length.GetValue ();

        case LengthValue::eType::Percent:
        {
            if ( _parent )
            {
                UIElement const &div = *_parent;

                LengthValue const* cases[] = { &div._css._width, &div._css._height };
                bool const isAuto = cases[ static_cast<size_t> ( isHeight ) ]->GetType () == LengthValue::eType::Auto;

                if ( isAuto & isHeight )
                {
                    // It's recursion. Doing exactly the same as Google Chrome v114.0.5735.110 does.
                    return 0.0F;
                }
            }

            return 1.0e-2F * referenceLength * length.GetValue ();
        }

        case LengthValue::eType::Unitless:
        return referenceLength * length.GetValue ();

        case LengthValue::eType::Auto:
        return 0.0F;

        case LengthValue::eType::Inherit:
            [[fallthrough]];
        default:
            AV_ASSERT ( false )
        return 0.0F;
    }
}

UIElement::AlignHandler UIElement::ResolveTextAlignment ( UIElement const &parent ) noexcept
{
    for ( UIElement const* p = &parent; p; )
    {
        switch ( p->_css._textAlign )
        {
            case TextAlignProperty::eValue::Center:
            return &UIElement::AlignToCenter;

            case TextAlignProperty::eValue::Left:
            return &UIElement::AlignToStart;

            case TextAlignProperty::eValue::Right:
            return &UIElement::AlignToEnd;

            case TextAlignProperty::eValue::Inherit:
                p = p->_parent;
            break;

            default:
                android_vulkan::LogError ( "pbr::UIElement::ResolveTextAlignment - Unknown alignment." );
                AV_ASSERT ( false )
            break;
        }
    }

    AV_ASSERT ( false )
    return &UIElement::AlignToStart;
}

UIElement::AlignHandler UIElement::ResolveVerticalAlignment ( UIElement const &parent ) noexcept
{
    for ( UIElement const* p = &parent; p; )
    {
        switch ( p->_css._verticalAlign )
        {
            case VerticalAlignProperty::eValue::Bottom:
            return &UIElement::AlignToEnd;

            case VerticalAlignProperty::eValue::Middle:
            return &UIElement::AlignToCenter;

            case VerticalAlignProperty::eValue::Top:
            return &UIElement::AlignToStart;

            case VerticalAlignProperty::eValue::Inherit:
                p = p->_parent;
            break;

            default:
                android_vulkan::LogError ( "pbr::UIElement::ResolveVerticalAlignment - Unknown alignment." );
                AV_ASSERT ( false )
            break;
        }
    }

    AV_ASSERT ( false )
    return &UIElement::AlignToStart;
}

float UIElement::AlignToCenter ( float pen, float parentSize, float lineSize ) noexcept
{
    return pen + 0.5F * ( parentSize - lineSize );
}

float UIElement::AlignToStart ( float pen, float /*parentSize*/, float /*lineSize*/ ) noexcept
{
    return pen;
}

float UIElement::AlignToEnd ( float pen, float parentSize, float lineSize ) noexcept
{
    return pen + parentSize - lineSize;
}

} // namespace pbr
