#include <pbr/div_html5_element.h>
#include <pbr/html5_parser.h>
#include <pbr/img_html5_element.h>
#include <pbr/image_ui_element.h>
#include <pbr/script_engine.h>
#include <pbr/text_html5_element.h>
#include <pbr/text_ui_element.h>
#include <pbr/ui_layer.h>
#include <pbr/utf8_parser.h>
#include <av_assert.h>
#include <file.h>
#include <logger.h>

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

    if ( !uiAsset )
    {
        success = false;
        return;
    }

    android_vulkan::File asset ( uiAsset );

    if ( success = asset.LoadContent (); !success )
        return;

    std::vector<uint8_t>& content = asset.GetContent ();
    HTML5Parser html {};

    success = html.Parse ( uiAsset,
        Stream ( Stream::Data ( content.data (), content.size () ), 1U ),
        std::filesystem::path ( uiAsset ).parent_path ().string ().c_str ()
    );

    if ( !success )
        return;

    if ( !lua_checkstack ( &vm, 7 ) )
    {
        android_vulkan::LogWarning ( "pbr::UILayer::UILayer - Stack is too small." );
        success = false;
        return;
    }

    constexpr std::string_view registerNamedElement = "RegisterNamedElement";
    lua_pushlstring ( &vm, registerNamedElement.data (), registerNamedElement.size () );

    if ( success = lua_rawget ( &vm, uiLayerIdx ) == LUA_TFUNCTION; !success )
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

    if ( !success )
    {
        lua_pop ( &vm, 3 );
        return;
    }

    constexpr std::string_view appendChildElement = "AppendChildElement";
    lua_pushlstring ( &vm, appendChildElement.data (), appendChildElement.size () );

    if ( success = lua_rawget ( &vm, -2 ) == LUA_TFUNCTION; !success )
    {
        android_vulkan::LogWarning ( "pbr::UILayer::UILayer - Can't find 'DIVUIElement:AppendChildElement' method." );
        lua_pop ( &vm, 3 );
        return;
    }

    lua_rotate ( &vm, -2, 1 );
    int const appendChildElementIdx = lua_gettop ( &vm ) - 1;

    for ( HTML5Children& children = html.GetBodyChildren (); auto& child : children )
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

    UIElement::ApplyLayoutInfo info
    {
        ._canvasSize = GXVec2 ( static_cast<float> ( viewport.width ), static_cast<float> ( viewport.height ) ),
        ._cssUnits = &_cssUnitToDevicePixel,
        ._currentLineHeight = 0.0F,
        ._fontStorage = &fontStorage,
        ._newLineHeight = 0.0F,
        ._newLines = 0U,
        ._parentTopLeft = GXVec2 ( 0.0F, 0.0F ),
        ._penLocation = GXVec2 ( 0.0F, 0.0F ),
        ._renderer = &renderer,
        ._vertices = 0U
    };

    _body->ApplyLayout ( info );

    return
    {
        ._needRedraw = true,
        ._neededUIVertices = info._vertices
    };
}

void UILayer::Submit ( UIElement::SubmitInfo &info ) noexcept
{
    _body->Submit ( info );
}

void UILayer::InitCSSUnitConverter ( float dpi, float comfortableViewDistanceMeters ) noexcept
{
    // See full explanation in documentation: UI system, CSS Units and hardware DPI.
    // <repo>/docs/ui-system.md#css-units-and-dpi

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

    for ( auto const& extension : extensions )
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
        auto& text = static_cast<TextHTML5Element&> ( htmlChild );

        auto* t = new TextUIElement ( success, &parent, vm, errorHandlerIdx, std::move ( text.GetText () ) );

        if ( !success )
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
        auto& img = static_cast<IMGHTML5Element&> ( htmlChild );

        auto* i = new ImageUIElement ( success,
            &parent,
            vm,
            errorHandlerIdx,
            std::move ( img.GetAssetPath () ),
            std::move ( img._cssComputedValues )
        );

        if ( !success )
        {
            delete i;
            return false;
        }

        UIElement::AppendElement ( *i );

        if ( !RegisterNamedElement ( vm, errorHandlerIdx, uiLayerIdx, registerNamedElementIdx, img.GetID () ) )
        {
            lua_pop ( &vm, 1 );
            return false;
        }

        return parent.AppendChildElement ( vm, errorHandlerIdx, appendChildElementIdx, *i );
    }

    if ( tag != HTML5Tag::eTag::DIV )
    {
        android_vulkan::LogWarning ( "pbr::UILayer::AppendChild - Unexpected tag '%s',", tag.ToString () );
        return false;
    }

    // NOLINTNEXTLINE - downcast.
    auto& div = static_cast<DIVHTML5Element&> ( htmlChild );

    auto* d = new DIVUIElement ( success, &parent, vm, errorHandlerIdx, std::move ( div._cssComputedValues ) );

    if ( !success )
    {
        delete d;
        return false;
    }

    UIElement::AppendElement ( *d );

    for ( HTML5Children& children = div.GetChildren (); auto& child : children )
    {
        success = AppendChild ( vm,
            errorHandlerIdx,
            appendChildElementIdx,
            uiLayerIdx,
            registerNamedElementIdx,
            *d,
            *child
        );

        if ( !success )
        {
            lua_pop ( &vm, 1 );
            return false;
        }
    }

    if ( !RegisterNamedElement ( vm, errorHandlerIdx, uiLayerIdx, registerNamedElementIdx, div.GetID () ) )
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
    std::string const& n = *name;
    lua_pushlstring ( &vm, n.c_str (), n.size () );

    if ( lua_pcall ( &vm, 3, 0, errorHandlerIdx ) == LUA_OK )
        return true;

    android_vulkan::LogWarning ( "pbr::UILayer::RegisterNamedElement - Can't register element '%s'.", n.c_str () );
    return false;
}

int UILayer::OnCreate ( lua_State* state )
{
    bool success;
    auto* layer = new UILayer ( success, *state );

    if ( success )
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

int UILayer::OnHide ( lua_State* /*state*/ )
{
    android_vulkan::LogDebug ( "UILayer::OnHide" );
    // TODO
    return 0;
}

int UILayer::OnIsVisible ( lua_State* /*state*/ )
{
    android_vulkan::LogDebug ( "UILayer::OnIsVisible" );
    // TODO
    return 0;
}

int UILayer::OnShow ( lua_State* /*state*/ )
{
    android_vulkan::LogDebug ( "UILayer::OnShow" );
    // TODO
    return 0;
}

} // namespace pbr
