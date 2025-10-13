#include <precompiled_headers.hpp>
#include <logger.hpp>
#include <platform/android/pbr/scriptable_image_ui_element.hpp>

GX_DISABLE_COMMON_WARNINGS

extern "C" {

#include <lua/lauxlib.h>

} // extern "C"

GX_RESTORE_WARNING_STATE


namespace pbr {

ScriptableImageUIElement::ScriptableImageUIElement ( bool &success,
    UIElement const* parent,
    lua_State &vm,
    int errorHandlerIdx,
    std::string &&asset,
    CSSComputedValues &&css
) noexcept:
    _image ( success, parent, std::move ( asset ), std::move ( css ) )
{
    if ( !success ) [[unlikely]]
        return;

    if ( success = lua_checkstack ( &vm, 2 ); !success ) [[unlikely]]
    {
        android_vulkan::LogError ( "pbr::ScriptableImageUIElement::ScriptableImageUIElement - Stack is too small." );
        return;
    }

    if ( success = lua_getglobal ( &vm, "RegisterImageUIElement" ) == LUA_TFUNCTION; !success ) [[unlikely]]
    {
        android_vulkan::LogError (
            "pbr::ScriptableImageUIElement::ScriptableImageUIElement - Can't find register function."
        );

        return;
    }

    lua_pushlightuserdata ( &vm, this );

    if ( success = lua_pcall ( &vm, 1, 1, errorHandlerIdx ) == LUA_OK; success ) [[likely]]
        return;

    android_vulkan::LogWarning (
        "pbr::ScriptableImageUIElement::ScriptableImageUIElement - Can't append element inside Lua VM."
    );
}

UIElement &ScriptableImageUIElement::GetElement () noexcept
{
    return _image;
}

UIElement const &ScriptableImageUIElement::GetElement () const noexcept
{
    return _image;
}

} // namespace pbr
