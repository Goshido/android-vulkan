#ifndef PBR_UI_ELEMENT_BASE_IPP
#define PBR_UI_ELEMENT_BASE_IPP


#include <av_assert.hpp>
#include <logger.hpp>
#include <pbr/css_unit_to_device_pixel.hpp>


namespace pbr {

template<typename F, typename P, typename S>
void UIElementBase<F, P, S>::Hide () noexcept
{
    _visibilityChanged = _visible;
    _visible = false;
}

template<typename F, typename P, typename S>
void UIElementBase<F, P, S>::Show () noexcept
{
    _visibilityChanged = !_visible;
    _visible = true;
}

template<typename F, typename P, typename S>
bool UIElementBase<F, P, S>::IsVisible () const noexcept
{
    return _visible;
}

template<typename F, typename P, typename S>
CSSComputedValues &UIElementBase<F, P, S>::GetCSS () noexcept
{
    return _css;
}

template<typename F, typename P, typename S>
CSSComputedValues const& UIElementBase<F, P, S>::GetCSS () const noexcept
{
    return _css;
}

template<typename F, typename P, typename S>
std::string_view UIElementBase<F, P, S>::ResolveFont () const noexcept
{
    for ( UIElementBase<F, P, S> const* p = this; p; p = p->_parent )
    {
        if ( std::string const &fontFile = p->GetCSS ()._fontFile; !fontFile.empty () )
        {
            return std::string_view ( fontFile );
        }
    }

    AV_ASSERT ( false )
    return {};
}

template<typename F, typename P, typename S>
float UIElementBase<F, P, S>::ResolveFontSize () const noexcept
{
    LengthValue const* target = nullptr;
    float relativeScale = 1.0F;
    LengthValue::eType type = LengthValue::eType::Auto;

    for ( UIElementBase<F, P, S> const* p = this; ( p != nullptr ) & ( target == nullptr ); p = p->_parent )
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
        android_vulkan::LogError ( "pbr::UIElementBase::ResolveFontSize - Everything is inherit." );
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

template<typename F, typename P, typename S>
float UIElementBase<F, P, S>::ResolveLineHeight ( F::Font font) const noexcept
{
    CSSUnitToDevicePixel const &cssUnits = CSSUnitToDevicePixel::GetInstance ();
    float const rootValue = _css._lineHeight.GetValue ();

    for ( UIElementBase<F, P, S> const* p = this; p; p = p->_parent )
    {
        switch ( LengthValue const &lineHeight = p->_css._lineHeight; lineHeight.GetType () )
        {
            case LengthValue::eType::Auto:
            return static_cast<float> ( F::GetFontPixelMetrics ( font )._baselineToBaseline );

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

    android_vulkan::LogError ( "pbr::UIElementBase::ResolveLineHeight - Everything is inherit." );
    AV_ASSERT ( false )
    return 0.0F;
}

template<typename F, typename P, typename S>
UIElementBase<F, P, S>::UIElementBase ( bool visible, UIElementBase<F, P, S> const* parent ) noexcept:
    _visible ( visible ),
    _parent ( parent )
{
    // NOTHING
}

template<typename F, typename P, typename S>
UIElementBase<F, P, S>::UIElementBase ( bool visible,
    UIElementBase<F, P, S> const* parent,
    std::string &&name
) noexcept:
    _name ( std::move ( name ) ),
    _visible ( visible ),
    _parent ( parent )
{
    // NOTHING
}

template<typename F, typename P, typename S>
UIElementBase<F, P, S>::UIElementBase ( bool visible,
    UIElementBase<F, P, S> const* parent,
    CSSComputedValues &&css
) noexcept:
    _css ( std::move ( css ) ),
    _visible ( visible ),
    _parent ( parent )
{
    // NOTHING
}

template<typename F, typename P, typename S>
UIElementBase<F, P, S>::UIElementBase ( bool visible,
    UIElementBase<F, P, S> const* parent,
    CSSComputedValues &&css,
    std::string &&name
) noexcept:
    _css ( std::move ( css ) ),
    _name ( std::move ( name ) ),
    _visible ( visible ),
    _parent ( parent )
{
    // NOTHING
}

template<typename F, typename P, typename S>
float UIElementBase<F, P, S>::ResolvePixelLength ( LengthValue const &length,
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
                UIElementBase<F, P, S> const &div = *_parent;

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

template<typename F, typename P, typename S>
UIElementBase<F, P, S>::AlignHandler UIElementBase<F, P, S>::ResolveTextAlignment (
    UIElementBase<F, P, S> const &parent
) noexcept
{
    for ( UIElementBase<F, P, S> const* p = &parent; p; )
    {
        switch ( p->_css._textAlign )
        {
            case TextAlignProperty::eValue::Center:
            return &UIElementBase<F, P, S>::AlignToCenter;

            case TextAlignProperty::eValue::Left:
            return &UIElementBase<F, P, S>::AlignToStart;

            case TextAlignProperty::eValue::Right:
            return &UIElementBase<F, P, S>::AlignToEnd;

            case TextAlignProperty::eValue::Inherit:
                p = p->_parent;
            break;

            default:
                android_vulkan::LogError ( "pbr::UIElementBase::ResolveTextAlignment - Unknown alignment." );
                AV_ASSERT ( false )
            break;
        }
    }

    AV_ASSERT ( false )
    return &UIElementBase<F, P, S>::AlignToStart;
}

template<typename F, typename P, typename S>
UIElementBase<F, P, S>::AlignHandler UIElementBase<F, P, S>::ResolveVerticalAlignment (
    UIElementBase<F, P, S> const &parent
) noexcept
{
    for ( UIElementBase<F, P, S> const* p = &parent; p; )
    {
        switch ( p->_css._verticalAlign )
        {
            case VerticalAlignProperty::eValue::Bottom:
            return &UIElementBase<F, P, S>::AlignToEnd;

            case VerticalAlignProperty::eValue::Middle:
            return &UIElementBase<F, P, S>::AlignToCenter;

            case VerticalAlignProperty::eValue::Top:
            return &UIElementBase<F, P, S>::AlignToStart;

            case VerticalAlignProperty::eValue::Inherit:
                p = p->_parent;
            break;

            default:
                android_vulkan::LogError ( "pbr::UIElementBase::ResolveVerticalAlignment - Unknown alignment." );
                AV_ASSERT ( false )
            break;
        }
    }

    AV_ASSERT ( false )
    return &UIElementBase<F, P, S>::AlignToStart;
}

template<typename F, typename P, typename S>
float UIElementBase<F, P, S>::AlignToCenter ( float pen, float parentSize, float lineSize ) noexcept
{
    return pen + 0.5F * ( parentSize - lineSize );
}

template<typename F, typename P, typename S>
float UIElementBase<F, P, S>::AlignToStart ( float pen, float /*parentSize*/, float /*lineSize*/ ) noexcept
{
    return pen;
}

template<typename F, typename P, typename S>
float UIElementBase<F, P, S>::AlignToEnd ( float pen, float parentSize, float lineSize ) noexcept
{
    return pen + parentSize - lineSize;
}

} // namespace pbr


#endif // PBR_UI_ELEMENT_BASE_IPP
