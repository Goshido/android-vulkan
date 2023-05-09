#include <pbr/div_html5_element.h>
#include <pbr/html5_parser.h>
#include <pbr/img_html5_element.h>
#include <pbr/image_ui_element.h>
#include <pbr/text_html5_element.h>
#include <pbr/text_ui_element.h>
#include <pbr/ui_layer.h>
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

UILayer::UILayer ( bool &success, std::string &&uiAsset ) noexcept
{
    android_vulkan::File asset ( uiAsset );
    success = asset.LoadContent ();

    if ( !success )
        return;

    std::vector<uint8_t>& content = asset.GetContent ();
    HTML5Parser html {};

    success = html.Parse ( uiAsset.c_str (),
        Stream ( Stream::Data ( content.data (), content.size () ), 1U ),
        std::filesystem::path ( uiAsset ).parent_path ().string ().c_str ()
    );

    if ( !success )
        return;

    _css = std::move ( html.GetCSSParser () );
    _body = std::make_unique<DIVUIElement> ( html.GetBodyCSS () );

    if ( std::u32string& id = html.GetBodyID (); !id.empty () )
        _namedElements.emplace ( std::move ( id ), _body.get () );

    // Using recursive lambda trick. Generic lambda. Pay attention to last parameter.
    auto const append = [] ( NamedElements &namedElements,
        UIElement &root,
        HTML5Element &htmlChild,
        auto append
    ) noexcept -> bool
    {
        HTML5Tag const tag = htmlChild.GetTag ();

        if ( tag == HTML5Tag::eTag::Text )
        {
            // NOLINTNEXTLINE - downcast.
            auto& text = static_cast<TextHTML5Element&> ( htmlChild );

            root.AppendChildElement ( std::make_unique<TextUIElement> ( std::move ( text.GetText () ) ) );
            return true;
        }

        if ( tag == HTML5Tag::eTag::IMG )
        {
            // NOLINTNEXTLINE - downcast.
            auto& img = static_cast<IMGHTML5Element&> ( htmlChild );

            ImageUIElement* i = new ImageUIElement ( std::move ( img.GetAssetPath () ), img._cssComputedValues );

            if ( std::u32string& id = img.GetID (); !id.empty () )
                namedElements.emplace ( std::move ( id ), i );

            root.AppendChildElement ( std::unique_ptr<UIElement> ( i ) );
            return true;
        }

        if ( tag != HTML5Tag::eTag::DIV )
        {
            android_vulkan::LogWarning ( "pbr::UILayer::UILayer - Unexpected tag '%s',", tag.ToString () );
            return false;
        }

        // NOLINTNEXTLINE - downcast.
        auto& div = static_cast<DIVHTML5Element&> ( htmlChild );

        DIVUIElement* d = new DIVUIElement ( div._cssComputedValues );

        if ( std::u32string& id = div.GetID (); !id.empty () )
            namedElements.emplace ( std::move ( id ), d );

        root.AppendChildElement ( std::unique_ptr<UIElement> ( d ) );

        for ( HTML5Childs& childs = div.GetChilds (); auto& child : childs )
        {
            if ( !append ( namedElements, *d, *child, append ) )
            {
                return false;
            }
        }

        return true;
    };

    for ( HTML5Childs& childs = html.GetBodyChilds (); auto& child : childs )
    {
        if ( !append ( _namedElements, *_body, *child, append ) )
        {
            success = false;
            return;
        }
    }
}

bool UILayer::Init ( lua_State &vm ) noexcept
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
        lua_register ( &vm, extension.name, extension.func );

    return true;
}

[[maybe_unused]] void UILayer::Destroy () noexcept
{
    // TODO
}

int UILayer::OnCreate ( lua_State* state )
{
    if ( !lua_checkstack ( state, 1 ) )
    {
        android_vulkan::LogWarning ( "pbr::UILayer::OnCreate - Stack too small." );
        return 0;
    }

    char const* uiAsset = lua_tostring ( state, 1 );

    if ( !uiAsset )
    {
        lua_pushnil ( state );
        return 1;
    }

    bool success;
    UILayer* layer = new UILayer ( success, uiAsset );

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
