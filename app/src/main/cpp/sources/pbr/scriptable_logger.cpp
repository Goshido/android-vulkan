#include <pbr/scriptable_logger.h>
#include <logger.h>

GX_DISABLE_COMMON_WARNINGS

extern "C" {

#include <lua/lauxlib.h>

} // extern "C"


GX_RESTORE_WARNING_STATE


namespace pbr {

void ScriptableLogger::Register ( lua_State &vm ) noexcept
{
    constexpr luaL_Reg const extensions[] =
    {
        {
            .name = "av_LogD",
            .func = &ScriptableLogger::OnLogDebug
        },

        {
            .name = "av_LogE",
            .func = &ScriptableLogger::OnLogError
        },

        {
            .name = "av_LogI",
            .func = &ScriptableLogger::OnLogInfo
        },

        {
            .name = "av_LogW",
            .func = &ScriptableLogger::OnLogWarning
        }
    };

    for ( auto const &extension : extensions )
    {
        lua_register ( &vm, extension.name, extension.func );
    }
}

int ScriptableLogger::OnLogDebug ( lua_State* vm )
{
    android_vulkan::LogDebug ( "[Lua]: %s", lua_tostring ( vm, 1 ) );
    return 0;
}

int ScriptableLogger::OnLogError ( lua_State* vm )
{
    android_vulkan::LogError ( "[Lua]: %s", lua_tostring ( vm, 1 ) );
    return 0;
}

int ScriptableLogger::OnLogInfo ( lua_State* vm )
{
    android_vulkan::LogInfo ( "[Lua]: %s", lua_tostring ( vm, 1 ) );
    return 0;
}

int ScriptableLogger::OnLogWarning ( lua_State* vm )
{
    android_vulkan::LogWarning ( "[Lua]: %s", lua_tostring ( vm, 1 ) );
    return 0;
}

} // namespace pbr
