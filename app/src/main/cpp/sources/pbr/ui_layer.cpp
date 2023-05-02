#include <pbr/ui_layer.h>

GX_DISABLE_COMMON_WARNINGS

extern "C" {

#include <lua/lauxlib.h>

} // extern "C"

GX_RESTORE_WARNING_STATE


namespace pbr {

UILayer::UILayer ( bool &success, std::string &&/*uiAsset*/ ) noexcept
{
    // TODO
    success = true;
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

int UILayer::OnCreate ( lua_State* /*state*/ )
{
    // TODO
    return 0;
}

int UILayer::OnFind ( lua_State* /*state*/ )
{
    // TODO
    return 0;
}

int UILayer::OnGarbageCollected ( lua_State* /*state*/ )
{
    // TODO
    return 0;
}

int UILayer::OnHide ( lua_State* /*state*/ )
{
    // TODO
    return 0;
}

int UILayer::OnIsVisible ( lua_State* /*state*/ )
{
    // TODO
    return 0;
}

int UILayer::OnShow ( lua_State* /*state*/ )
{
    // TODO
    return 0;
}

} // namespace pbr
