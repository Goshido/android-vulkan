#include <pbr/image_ui_element.h>
#include <logger.h>

GX_DISABLE_COMMON_WARNINGS

extern "C" {

#include <lua/lauxlib.h>

} // extern "C"

GX_RESTORE_WARNING_STATE


namespace pbr {

ImageUIElement::ImageUIElement ( bool &success,
    lua_State &vm,
    int errorHandlerIdx,
    std::string &&asset,
    CSSComputedValues const &css
) noexcept:
    UIElement ( true ),
    _asset ( std::move ( asset ) ),
    _css ( css )
{
    if ( success = lua_checkstack ( &vm, 2 ); !success )
    {
        android_vulkan::LogError ( "pbr::ImageUIElement::ImageUIElement - Stack is too small." );
        return;
    }

    if ( success = lua_getglobal ( &vm, "RegisterImageUIElement" ) == LUA_TFUNCTION; !success )
    {
        android_vulkan::LogError ( "pbr::ImageUIElement::ImageUIElement - Can't find register function." );
        return;
    }

    lua_pushlightuserdata ( &vm, this );

    if ( success = lua_pcall ( &vm, 1, 1, errorHandlerIdx ) == LUA_OK; !success )
    {
        android_vulkan::LogWarning ( "pbr::ImageUIElement::ImageUIElement - Can't append element inside Lua VM." );
    }
}

void ImageUIElement::Init ( lua_State &vm ) noexcept
{
    constexpr luaL_Reg const extensions[] =
    {
        {
            .name = "av_ImageUIElementCollectGarbage",
            .func = &ImageUIElement::OnGarbageCollected
        }
    };

    for ( auto const& extension : extensions )
    {
        lua_register ( &vm, extension.name, extension.func );
    }
}

void ImageUIElement::ApplyLayout () noexcept
{
    // TODO
}

void ImageUIElement::Render () noexcept
{
    // TODO
}

int ImageUIElement::OnGarbageCollected ( lua_State* /*state*/ )
{
    // TODO
    return 0;
}

} // namespace pbr
