#include <pbr/div_html5_element.hpp>
#include <pbr/html5_parser.hpp>
#include <pbr/img_html5_element.hpp>
#include <pbr/image_ui_element.hpp>
#include <pbr/script_engine.hpp>
#include <pbr/text_html5_element.hpp>
#include <pbr/text_ui_element.hpp>
#include <pbr/ui_layer.hpp>
#include <pbr/utf8_parser.hpp>
#include <av_assert.hpp>
#include <file.hpp>
#include <logger.hpp>

GX_DISABLE_COMMON_WARNINGS

#include <filesystem>

extern "C" {

#include <lua/lauxlib.h>

} // extern "C"

GX_RESTORE_WARNING_STATE


namespace pbr {

CSSUnitToDevicePixel UILayer::_cssUnitToDevicePixel {};
std::unordered_set<UILayer*> UILayer::_uiLayers {};

UILayer::UILayer ( bool &success, lua_State &vm ) noexcept
{
    constexpr int uiLayerIdx = 1;
    constexpr int uiAssetIdx = 2;
    char const* uiAsset = lua_tostring ( &vm, uiAssetIdx );

    if ( !uiAsset ) [[unlikely]]
    {
        success = false;
        return;
    }

    android_vulkan::File asset ( uiAsset );

    if ( success = asset.LoadContent (); !success ) [[unlikely]]
        return;

    std::vector<uint8_t> &content = asset.GetContent ();
    HTML5Parser html {};

    success = html.Parse ( uiAsset,
        Stream ( Stream::Data ( content.data (), content.size () ), 1U ),
        std::filesystem::path ( uiAsset ).parent_path ().string ().c_str ()
    );

    if ( !success ) [[unlikely]]
        return;

    if ( !lua_checkstack ( &vm, 7 ) ) [[unlikely]]
    {
        android_vulkan::LogWarning ( "pbr::UILayer::UILayer - Stack is too small." );
        success = false;
        return;
    }

    constexpr std::string_view registerNamedElement = "RegisterNamedElement";
    lua_pushlstring ( &vm, registerNamedElement.data (), registerNamedElement.size () );

    if ( success = lua_rawget ( &vm, uiLayerIdx ) == LUA_TFUNCTION; !success ) [[unlikely]]
    {
        android_vulkan::LogWarning ( "pbr::UILayer::UILayer - Can't find 'UILayer:RegisterNamedElement' method." );
        return;
    }

    int const registerNamedElementIdx = lua_gettop ( &vm );
    int const errorHandlerIdx = ScriptEngine::PushErrorHandlerToStack ( vm );

    _css = std::move ( html.GetCSSParser () );
    _body = new DIVUIElement ( success, nullptr, vm, errorHandlerIdx, std::move ( html.GetBodyCSS () ) );

    if ( !success )
    {
        delete _body;
        lua_pop ( &vm, 2 );
        return;
    }

    // Creating '_body' property in 'UILayer' instance (Lua side).
    constexpr std::string_view bodyProp = "_body";
    lua_pushlstring ( &vm, bodyProp.data (), bodyProp.size () );
    lua_pushvalue ( &vm, -2 );
    lua_rawset ( &vm, uiLayerIdx );
    UIElement::AppendElement ( *_body );

    success = RegisterNamedElement ( vm, errorHandlerIdx, uiLayerIdx, registerNamedElementIdx, html.GetBodyID () );

    if ( !success ) [[unlikely]]
    {
        lua_pop ( &vm, 3 );
        return;
    }

    constexpr std::string_view appendChildElement = "AppendChildElement";
    lua_pushlstring ( &vm, appendChildElement.data (), appendChildElement.size () );

    if ( success = lua_rawget ( &vm, -2 ) == LUA_TFUNCTION; !success ) [[unlikely]]
    {
        android_vulkan::LogWarning ( "pbr::UILayer::UILayer - Can't find 'DIVUIElement:AppendChildElement' method." );
        lua_pop ( &vm, 3 );
        return;
    }

    lua_rotate ( &vm, -2, 1 );
    int const appendChildElementIdx = lua_gettop ( &vm ) - 1;

    for ( HTML5Children &children = html.GetBodyChildren (); auto &child : children )
    {
        success = AppendChild ( vm,
            errorHandlerIdx,
            appendChildElementIdx,
            uiLayerIdx,
            registerNamedElementIdx,
            *_body,
            *child
        );

        if ( success )
            continue;

        lua_pop ( &vm, 4 );
        return;
    }

    lua_pop ( &vm, 4 );
}

UILayer::LayoutStatus UILayer::ApplyLayout ( android_vulkan::Renderer &renderer, FontStorage &fontStorage ) noexcept
{
    VkExtent2D const viewport = renderer.GetViewportResolution ();

    _lineHeights.clear ();
    _lineHeights.push_back ( 0.0F );

    UIElement::ApplyInfo info
    {
        ._canvasSize = GXVec2 ( static_cast<float> ( viewport.width ), static_cast<float> ( viewport.height ) ),
        ._cssUnits = &_cssUnitToDevicePixel,
        ._fontStorage = &fontStorage,
        ._hasChanges = false,
        ._lineHeights = &_lineHeights,
        ._pen = GXVec2 ( 0.0F, 0.0F ),
        ._renderer = &renderer,
        ._vertices = 0U
    };

    _body->ApplyLayout ( info );

    return
    {
        ._hasChanges = info._hasChanges,
        ._neededUIVertices = info._vertices
    };
}

void UILayer::Submit ( UIElement::SubmitInfo &info ) noexcept
{
    _body->Submit ( info );
}

bool UILayer::UpdateCache ( FontStorage &fontStorage, VkExtent2D const &viewport ) noexcept
{
    UIElement::UpdateInfo info
    {
        ._cssUnits = &_cssUnitToDevicePixel,
        ._fontStorage = &fontStorage,
        ._line = 0U,
        ._parentLineHeights = _lineHeights.data (),
        ._parentSize = GXVec2 ( static_cast<float> ( viewport.width ), static_cast<float> ( viewport.height ) ),
        ._parentTopLeft = GXVec2 ( 0.0F, 0.0F ),
        ._pen = GXVec2 ( 0.0F, 0.0F )
    };

    return _body->UpdateCache ( info );
}

void UILayer::InitCSSUnitConverter ( float dpi, float comfortableViewDistanceMeters ) noexcept
{
    // See full explanation in documentation: UI system, CSS Units and hardware DPI.
    // docs/ui-system.md#css-units-and-dpi

    constexpr float dpiSpec = 96.0F;
    constexpr float distanceSpec = 28.0F;
    constexpr float meterToInch = 3.93701e+1F;
    constexpr float dpiFactor = meterToInch / ( dpiSpec * distanceSpec );

    _cssUnitToDevicePixel._fromPX = dpi * comfortableViewDistanceMeters * dpiFactor;

    constexpr float inchToPX = 96.0F;
    constexpr float inchToMM = 25.4F;
    constexpr float pxToMM = inchToPX / inchToMM;
    _cssUnitToDevicePixel._fromMM = pxToMM * _cssUnitToDevicePixel._fromPX;

    constexpr float inchToPT = 72.0F;
    constexpr float pxToPT = inchToPX / inchToPT;
    _cssUnitToDevicePixel._fromPT = pxToPT * _cssUnitToDevicePixel._fromPX;
}

void UILayer::InitLuaFrontend ( lua_State &vm ) noexcept
{
    constexpr luaL_Reg const extensions[] =
    {
        {
            .name = "av_UILayerCreate",
            .func = &UILayer::OnCreate
        },
        {
            .name = "av_UILayerCollectGarbage",
            .func = &UILayer::OnGarbageCollected
        },
        {
            .name = "av_UILayerHide",
            .func = &UILayer::OnHide
        },
        {
            .name = "av_UILayerIsVisible",
            .func = &UILayer::OnIsVisible
        },
        {
            .name = "av_UILayerShow",
            .func = &UILayer::OnShow
        }
    };

    for ( auto const &extension : extensions )
    {
        lua_register ( &vm, extension.name, extension.func );
    }
}

void UILayer::Destroy () noexcept
{
    if ( !_uiLayers.empty () )
    {
        android_vulkan::LogWarning ( "pbr::UILayer::Destroy - Memory leak." );
        AV_ASSERT ( false )
    }

    _uiLayers.clear ();
}

// NOLINTNEXTLINE - recursive call chain.
bool UILayer::AppendChild ( lua_State &vm,
    int errorHandlerIdx,
    int appendChildElementIdx,
    int uiLayerIdx,
    int registerNamedElementIdx,
    DIVUIElement &parent,
    HTML5Element &htmlChild
) noexcept
{
    bool success;
    HTML5Tag const tag = htmlChild.GetTag ();

    if ( tag == HTML5Tag::eTag::Text )
    {
        // NOLINTNEXTLINE - downcast.
        auto &text = static_cast<TextHTML5Element &> ( htmlChild );

        auto* t = new TextUIElement ( success, &parent, vm, errorHandlerIdx, std::move ( text.GetText () ) );

        if ( !success ) [[unlikely]]
        {
            delete t;
            return false;
        }

        UIElement::AppendElement ( *t );
        return parent.AppendChildElement ( vm, errorHandlerIdx, appendChildElementIdx, *t );
    }

    if ( tag == HTML5Tag::eTag::IMG )
    {
        // NOLINTNEXTLINE - downcast.
        auto &img = static_cast<IMGHTML5Element &> ( htmlChild );

        auto* i = new ImageUIElement ( success,
            &parent,
            vm,
            errorHandlerIdx,
            std::move ( img.GetAssetPath () ),
            std::move ( img._cssComputedValues )
        );

        if ( !success ) [[unlikely]]
        {
            delete i;
            return false;
        }

        UIElement::AppendElement ( *i );

        if ( !RegisterNamedElement ( vm, errorHandlerIdx, uiLayerIdx, registerNamedElementIdx, img.GetID () ) )
        {
            [[unlikely]]
            lua_pop ( &vm, 1 );
            return false;
        }

        return parent.AppendChildElement ( vm, errorHandlerIdx, appendChildElementIdx, *i );
    }

    if ( tag != HTML5Tag::eTag::DIV ) [[unlikely]]
    {
        android_vulkan::LogWarning ( "pbr::UILayer::AppendChild - Unexpected tag '%s',", tag.ToString () );
        return false;
    }

    // NOLINTNEXTLINE - downcast.
    auto &div = static_cast<DIVHTML5Element &> ( htmlChild );

    auto* d = new DIVUIElement ( success, &parent, vm, errorHandlerIdx, std::move ( div._cssComputedValues ) );

    if ( !success ) [[unlikely]]
    {
        delete d;
        return false;
    }

    UIElement::AppendElement ( *d );

    for ( HTML5Children &children = div.GetChildren (); auto &child : children )
    {
        success = AppendChild ( vm,
            errorHandlerIdx,
            appendChildElementIdx,
            uiLayerIdx,
            registerNamedElementIdx,
            *d,
            *child
        );

        if ( !success ) [[unlikely]]
        {
            lua_pop ( &vm, 1 );
            return false;
        }
    }

    if ( !RegisterNamedElement ( vm, errorHandlerIdx, uiLayerIdx, registerNamedElementIdx, div.GetID () ) ) [[unlikely]]
    {
        lua_pop ( &vm, 1 );
        return false;
    }

    return parent.AppendChildElement ( vm, errorHandlerIdx, appendChildElementIdx, *d );
}

bool UILayer::RegisterNamedElement ( lua_State &vm,
    int errorHandlerIdx,
    int uiLayerIdx,
    int registerNamedElementIdx,
    std::u32string const &id
) noexcept
{
    if ( id.empty () )
        return true;

    lua_pushvalue ( &vm, registerNamedElementIdx );
    lua_pushvalue ( &vm, uiLayerIdx );
    lua_pushvalue ( &vm, -3 );

    auto const name = UTF8Parser::ToUTF8 ( id );
    std::string const &n = *name;
    lua_pushlstring ( &vm, n.c_str (), n.size () );

    if ( lua_pcall ( &vm, 3, 0, errorHandlerIdx ) == LUA_OK ) [[likely]]
        return true;

    android_vulkan::LogWarning ( "pbr::UILayer::RegisterNamedElement - Can't register element '%s'.", n.c_str () );
    AV_ASSERT ( false )
    return false;
}

int UILayer::OnCreate ( lua_State* state )
{
    bool success;
    auto* layer = new UILayer ( success, *state );

    if ( success ) [[likely]]
    {
        _uiLayers.insert ( layer );
        lua_pushlightuserdata ( state, layer );
        return 1;
    }

    delete layer;
    lua_pushnil ( state );

    return 1;
}

int UILayer::OnGarbageCollected ( lua_State* state )
{
    _uiLayers.erase ( static_cast<UILayer*> ( lua_touserdata ( state, 1 ) ) );
    return 0;
}

int UILayer::OnHide ( lua_State* state )
{
    auto &layer = *static_cast<UILayer*> ( lua_touserdata ( state, 1 ) );

    if ( layer._body ) [[likely]]
    {
        layer._body->Hide ();
        return 0;
    }

    android_vulkan::LogDebug ( "pbr::UILayer::OnHide - Layer has no body. Doing nothing." );
    AV_ASSERT ( false )
    return 0;
}

int UILayer::OnIsVisible ( lua_State* state )
{
    if ( !lua_checkstack ( state, 1 ) ) [[unlikely]]
    {
        android_vulkan::LogWarning ( "pbr::UILayer::OnIsVisible - Stack is too small." );
        return 0;
    }

    auto const &layer = *static_cast<UILayer const*> ( lua_touserdata ( state, 1 ) );
    lua_pushboolean ( state, layer._body != nullptr && layer._body->IsVisible () );
    return 1;
}

int UILayer::OnShow ( lua_State* state )
{
    auto &layer = *static_cast<UILayer*> ( lua_touserdata ( state, 1 ) );

    if ( layer._body ) [[likely]]
    {
        layer._body->Show ();
        return 0;
    }

    android_vulkan::LogDebug ( "pbr::UILayer::OnShow - Layer has no body. Doing nothing." );
    AV_ASSERT ( false )
    return 0;
}

} // namespace pbr
