#include <pbr/div_ui_element.h>
#include <file.h>
#include <logger.h>

GX_DISABLE_COMMON_WARNINGS

extern "C" {

#include <lua/lauxlib.h>

} // extern "C"

GX_RESTORE_WARNING_STATE


namespace pbr {

DIVUIElement::DIVUIElement ( bool &success,
    UIElement const* parent,
    lua_State &vm,
    int errorHandlerIdx,
    CSSComputedValues &&css
) noexcept:
    UIElement ( css._display != DisplayProperty::eValue::None, parent ),
    _css ( std::move ( css ) )
{
    _css._fontFile = std::move ( android_vulkan::File ( std::move ( _css._fontFile ) ).GetPath () );

    if ( success = lua_checkstack ( &vm, 2 ); !success )
    {
        android_vulkan::LogError ( "pbr::DIVUIElement::DIVUIElement - Stack is too small." );
        return;
    }

    if ( success = lua_getglobal ( &vm, "RegisterDIVUIElement" ) == LUA_TFUNCTION; !success )
    {
        android_vulkan::LogError ( "pbr::DIVUIElement::DIVUIElement - Can't find register function." );
        return;
    }

    lua_pushlightuserdata ( &vm, this );

    if ( success = lua_pcall ( &vm, 1, 1, errorHandlerIdx ) == LUA_OK; !success )
    {
        android_vulkan::LogWarning ( "pbr::DIVUIElement::DIVUIElement - Can't append element inside Lua VM." );
    }
}

void DIVUIElement::ApplyLayout ( android_vulkan::Renderer &renderer,
    FontStorage &fontStorage,
    CSSUnitToDevicePixel const &cssUnits,
    GXVec2 &penLocation,
    float &lineHeight,
    GXVec2 const &canvasSize,
    float parentLeft,
    float parentWidth
) noexcept
{
    // TODO

    for ( auto* child : _children )
    {
        child->ApplyLayout ( renderer,
            fontStorage,
            cssUnits,
            penLocation,
            lineHeight,
            canvasSize,
            parentLeft,
            parentWidth
        );
    }
}

void DIVUIElement::Render () noexcept
{
    // TODO

    for ( auto* child : _children )
    {
        child->Render ();
    }
}

bool DIVUIElement::AppendChildElement ( lua_State &vm,
    int errorHandlerIdx,
    int appendChildElementIdx,
    UIElement &element
) noexcept
{
    if ( !lua_checkstack ( &vm, 3 ) )
    {
        android_vulkan::LogError ( "pbr::DIVUIElement::AppendChildElement - Stack is too small." );
        return false;
    }

    lua_pushvalue ( &vm, appendChildElementIdx );
    lua_pushvalue ( &vm, -3 );
    lua_rotate ( &vm, -3, -1 );

    if ( lua_pcall ( &vm, 2, 0, errorHandlerIdx ) == LUA_OK )
    {
        _children.emplace_back ( &element );
        return true;
    }

    android_vulkan::LogWarning ( "pbr::DIVUIElement::AppendChildElement - Can't append child element inside Lua VM." );
    return false;
}

} // namespace pbr
