#include <pbr/div_html5_element.h>
#include <pbr/html5_parser.h>
#include <pbr/img_html5_element.h>
#include <pbr/image_ui_element.h>
#include <pbr/script_engine.h>
#include <pbr/text_html5_element.h>
#include <pbr/text_ui_element.h>
#include <pbr/ui_layer.h>
#include <pbr/utf8_parser.h>
#include <file.h>
#include <logger.h>

GX_DISABLE_COMMON_WARNINGS

#include <filesystem>

extern "C" {

#include <lua/lauxlib.h>

} // extern "C"

GX_RESTORE_WARNING_STATE


namespace pbr {

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

    if ( !lua_checkstack ( &vm, 6 ) )
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
    _css = std::move ( html.GetCSSParser () );
    _body = std::make_unique<DIVUIElement> ( success, vm, html.GetBodyCSS () );

    if ( !success )
    {
        lua_pop ( &vm, 1 );
        return;
    }

    // TODO refactor move to dedicated static method.
    auto const registerElement = [] ( lua_State &vm,
        std::u32string const &id,
        int registerNamedElementIdx
    ) noexcept -> bool {
        if ( id.empty () )
            return true;

        lua_pushvalue ( &vm, registerNamedElementIdx );
        lua_pushvalue ( &vm, uiLayerIdx );
        lua_pushvalue ( &vm, -3 );

        auto const name = UTF8Parser::ToUTF8 ( id );
        std::string const& n = *name;
        lua_pushlstring ( &vm, n.c_str (), n.size () );

        if ( lua_pcall ( &vm, 3, 0, ScriptEngine::GetErrorHandlerIndex () ) == LUA_OK )
            return true;

        android_vulkan::LogWarning ( "pbr::UILayer::UILayer - Can't register named element '%s'", n.c_str () );
        return false;
    };

    if ( success = registerElement ( vm, html.GetBodyID (), registerNamedElementIdx ); !success )
    {
        lua_pop ( &vm, 2 );
        return;
    }

    constexpr std::string_view appendChildElement = "AppendChildElement";
    lua_pushlstring ( &vm, appendChildElement.data (), appendChildElement.size () );

    if ( success = lua_rawget ( &vm, -2 ) == LUA_TFUNCTION; !success )
    {
        android_vulkan::LogWarning ( "pbr::UILayer::UILayer - Can't find 'DIVUIElement:AppendChildElement' method." );
        lua_pop ( &vm, 2 );
        return;
    }

    lua_rotate ( &vm, -2, 1 );
    int const appendChildElementIdx = lua_gettop ( &vm ) - 1;

    // TODO refactor move to dedicated static method.
    // Using recursive lambda trick. Generic lambda. Pay attention to last parameter.
    auto const append = [] ( lua_State &vm,
        DIVUIElement &root,
        HTML5Element &htmlChild,
        int appendChildElementIdx,
        int registerNamedElementIdx,
        auto registerElement,
        auto append
    ) noexcept -> bool
    {
        bool success;
        HTML5Tag const tag = htmlChild.GetTag ();

        if ( tag == HTML5Tag::eTag::Text )
        {
            // NOLINTNEXTLINE - downcast.
            auto& text = static_cast<TextHTML5Element&> ( htmlChild );

            TextUIElement* t = new TextUIElement ( success, vm, std::move ( text.GetText () ) );

            if ( !success )
            {
                delete t;
                return false;
            }

            return root.AppendChildElement ( vm, appendChildElementIdx, std::unique_ptr<UIElement> ( t ) );
        }

        if ( tag == HTML5Tag::eTag::IMG )
        {
            // NOLINTNEXTLINE - downcast.
            auto& img = static_cast<IMGHTML5Element&> ( htmlChild );

            ImageUIElement* i = new ImageUIElement ( success,
                vm,
                std::move ( img.GetAssetPath () ),
                img._cssComputedValues
            );

            if ( !success )
            {
                delete i;
                return false;
            }

            return registerElement ( vm, img.GetID (), registerNamedElementIdx ) &&
                root.AppendChildElement ( vm, appendChildElementIdx, std::unique_ptr<UIElement> ( i ) );
        }

        if ( tag != HTML5Tag::eTag::DIV )
        {
            android_vulkan::LogWarning ( "pbr::UILayer::UILayer - Unexpected tag '%s',", tag.ToString () );
            return false;
        }

        // NOLINTNEXTLINE - downcast.
        auto& div = static_cast<DIVHTML5Element&> ( htmlChild );

        DIVUIElement* d = new DIVUIElement ( success, vm, div._cssComputedValues );

        if ( !success )
        {
            delete d;
            return false;
        }

        for ( HTML5Childs& childs = div.GetChilds (); auto& child : childs )
        {
            if ( !append ( vm, *d, *child, appendChildElementIdx, registerNamedElementIdx, registerElement, append ) )
            {
                lua_pop ( &vm, 1 );
                return false;
            }
        }

        return registerElement ( vm, div.GetID (), registerNamedElementIdx ) &&
            root.AppendChildElement ( vm, appendChildElementIdx, std::unique_ptr<UIElement> ( d ) );
    };

    for ( HTML5Childs& childs = html.GetBodyChilds (); auto& child : childs )
    {
        if ( append ( vm, *_body, *child, appendChildElementIdx, registerNamedElementIdx, registerElement, append ) )
            continue;

        success = false;
        lua_pop ( &vm, 3 );
        return;
    }

    lua_pop ( &vm, 3 );
}

void UILayer::Init ( lua_State &vm ) noexcept
{
    constexpr luaL_Reg const extensions[] =
    {
        {
            .name = "av_UILayerCreate",
            .func = &UILayer::OnCreate
        },
        {
            .name = "av_UILayerFind",
            .func = &UILayer::OnFind
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

[[maybe_unused]] void UILayer::Destroy () noexcept
{
    // TODO
}

int UILayer::OnCreate ( lua_State* state )
{
    bool success;
    UILayer* layer = new UILayer ( success, *state/*, uiAsset*/ );

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

int UILayer::OnFind ( lua_State* /*state*/ )
{
    android_vulkan::LogDebug ( "UILayer::OnFind" );
    // TODO
    return 0;
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
