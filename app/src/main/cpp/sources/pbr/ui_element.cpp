#include <pbr/div_ui_element.h>
#include <pbr/ui_element.h>
#include <av_assert.h>
#include <logger.h>

GX_DISABLE_COMMON_WARNINGS

extern "C" {

#include <lua/lauxlib.h>

} // extern "C"

GX_RESTORE_WARNING_STATE


namespace pbr {

std::unordered_map<UIElement const*, std::unique_ptr<UIElement>> UIElement::_uiElements {};

void UIElement::AppendElement ( UIElement &element ) noexcept
{
    _uiElements.emplace ( &element, std::unique_ptr<UIElement> ( &element ) );
}

void UIElement::InitCommon ( lua_State &vm ) noexcept
{
    constexpr luaL_Reg const extensions[] =
    {
        {
            .name = "av_UIElementCollectGarbage",
            .func = &UIElement::OnGarbageCollected
        },
        {
            .name = "av_UIElementHide",
            .func = &UIElement::OnHide
        },
        {
            .name = "av_UIElementIsVisible",
            .func = &UIElement::OnIsVisible
        },
        {
            .name = "av_UIElementShow",
            .func = &UIElement::OnShow
        }
    };

    for ( auto const& extension : extensions )
    {
        lua_register ( &vm, extension.name, extension.func );
    }
}

void UIElement::Destroy () noexcept
{
    if ( !_uiElements.empty () )
    {
        android_vulkan::LogWarning ( "pbr::UIElement::Destroy - Memory leak." );
        AV_ASSERT ( false )
    }

    _uiElements.clear ();
}

UIElement::UIElement ( bool visible, UIElement const* parent ) noexcept:
    _visible ( visible ),
    _parent ( parent )
{
    // NOTHING
}

float UIElement::ResolveFontSize ( CSSUnitToDevicePixel const &cssUnits,
    UIElement const &startTraverseElement
) noexcept
{
    LengthValue const* target = nullptr;
    float relativeScale = 1.0F;
    LengthValue::eType type;

    for ( UIElement const* p = &startTraverseElement; p; p = p->_parent )
    {
        // NOLINTNEXTLINE - downcast.
        auto const& div = *static_cast<DIVUIElement const*> ( p );
        LengthValue const& size = div._css._fontSize;
        type = size.GetType ();

        if ( type == LengthValue::eType::EM )
        {
            relativeScale *= size.GetValue ();
            continue;
        }

        if ( type == LengthValue::eType::Percent )
        {
            relativeScale *= 1.0e-2F * size.GetValue ();
            continue;
        }

        if ( type == LengthValue::eType::Auto )
            continue;

        target = &size;
        break;
    }

    AV_ASSERT ( target )

    switch ( type )
    {
        case LengthValue::eType::MM:
        return relativeScale * target->GetValue () * cssUnits._fromMM;

        case LengthValue::eType::PT:
        return relativeScale * target->GetValue () * cssUnits._fromPT;

        case LengthValue::eType::PX:
        return relativeScale * target->GetValue () * cssUnits._fromPX;

        case LengthValue::eType::Percent:
        case LengthValue::eType::EM:
        case LengthValue::eType::Auto:
        default:
            // IMPOSSIBLE
            AV_ASSERT ( false )
        return 0.0F;
    }
}

float UIElement::ResolvePixelLength ( LengthValue const &length,
    float parentLength,
    bool isHeight,
    CSSUnitToDevicePixel const &units
) const noexcept
{
    switch ( length.GetType () )
    {
        case LengthValue::eType::EM:
        return static_cast<float> ( ResolveFontSize ( units, *this ) );

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
                // NOLINTNEXTLINE - downcast.
                auto const& div = *static_cast<DIVUIElement const*> ( _parent );

                LengthValue const* cases[] = { &div._css._width, &div._css._height };
                bool const isAuto = cases[ static_cast<size_t> ( isHeight ) ]->GetType () == LengthValue::eType::Auto;

                if ( isAuto & isHeight )
                {
                    // It's recursion. Doing exactly the same as Google Chrome v114.0.5735.110 does.
                    return 0.0F;
                }
            }

            return 1.0e-2F * parentLength * length.GetValue ();
        }

        case LengthValue::eType::Auto:
        return 0.0F;

        default:
            AV_ASSERT ( false )
        return 0.0F;
    }
}

int UIElement::OnGarbageCollected ( lua_State* state )
{
    auto const* element = static_cast<UIElement const*> ( lua_touserdata ( state, 1 ) );

    if ( auto const findResult = _uiElements.find ( element ); findResult != _uiElements.cend () )
    {
        _uiElements.erase ( findResult );
        return 0;
    }

    android_vulkan::LogWarning ( "pbr::UIElement::OnGarbageCollected - Can't find element." );
    AV_ASSERT ( false )
    return 0;
}

int UIElement::OnHide ( lua_State* state )
{
    auto& item = *static_cast<UIElement*> ( lua_touserdata ( state, 1 ) );
    item._visible = false;
    return 0;
}

int UIElement::OnIsVisible ( lua_State* state )
{
    if ( !lua_checkstack ( state, 1 ) )
    {
        android_vulkan::LogWarning ( "pbr::UIElement::OnIsVisible - Stack is too small." );
        return 0;
    }

    auto const& item = *static_cast<UIElement const*> ( lua_touserdata ( state, 1 ) );
    lua_pushboolean ( state, item._visible );
    return 1;
}

int UIElement::OnShow ( lua_State* state )
{
    auto& item = *static_cast<UIElement*> ( lua_touserdata ( state, 1 ) );
    item._visible = true;
    return 0;
}

} // namespace pbr
