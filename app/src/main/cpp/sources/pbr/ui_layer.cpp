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

    if ( !asset.LoadContent () )
    {
        success = false;
        return;
    }

    std::vector<uint8_t>& content = asset.GetContent ();

    success = _html.Parse ( uiAsset.c_str (),
        Stream ( Stream::Data ( content.data (), content.size () ), 1U ),
        std::filesystem::path ( uiAsset ).parent_path ().string ().c_str ()
    );
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
