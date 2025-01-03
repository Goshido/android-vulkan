#include <precompiled_headers.hpp>
#include <pbr/script_engine.hpp>
#include <pbr/animation_blend_node.hpp>
#include <pbr/animation_graph.hpp>
#include <pbr/animation_player_node.hpp>
#include <pbr/actor.hpp>
#include <pbr/bit_field.hpp>
#include <pbr/camera_component.hpp>
#include <pbr/component.hpp>
#include <pbr/rigid_body_component.hpp>
#include <pbr/script_component.hpp>
#include <pbr/scriptable_gxmat3.hpp>
#include <pbr/scriptable_gxmat4.hpp>
#include <pbr/scriptable_gxquat.hpp>
#include <pbr/scriptable_gxvec3.hpp>
#include <pbr/scriptable_gxvec4.hpp>
#include <pbr/scriptable_logger.hpp>
#include <pbr/scriptable_material.hpp>
#include <pbr/scriptable_text_ui_element.hpp>
#include <pbr/skeletal_mesh_component.hpp>
#include <pbr/sound_emitter_global_component.hpp>
#include <pbr/sound_emitter_spatial_component.hpp>
#include <pbr/static_mesh_component.hpp>
#include <pbr/transform_component.hpp>
#include <pbr/ui_layer.hpp>
#include <av_assert.hpp>
#include <file.hpp>
#include <logger.hpp>

GX_DISABLE_COMMON_WARNINGS

extern "C" {

#include <lua/lualib.h>
#include <lua/lauxlib.h>

} // extern "C"

GX_RESTORE_WARNING_STATE


namespace pbr {

namespace {

constexpr char const SCRIPT[] = "pbr/engine/scene.lua";

} // end of anonymous namespace

//----------------------------------------------------------------------------------------------------------------------

ScriptEngine* ScriptEngine::_instance = nullptr;

lua_State &ScriptEngine::GetVirtualMachine () noexcept
{
    return *_vm;
}

bool ScriptEngine::Init ( android_vulkan::Renderer &renderer, android_vulkan::SoundMixer &soundMixer ) noexcept
{
    if ( !InitLua () )
        return false;

    if ( !LoadScript ( _vm.get (), SCRIPT, SCRIPT ) )
        return false;

    return InitInterfaceFunctions ( renderer, soundMixer );
}

ScriptEngine &ScriptEngine::GetInstance () noexcept
{
    if ( !_instance )
        _instance = new ScriptEngine ();

    return *_instance;
}

void ScriptEngine::Destroy () noexcept
{
    if ( !_instance )
        return;

    delete _instance;
    _instance = nullptr;
}

int ScriptEngine::PushErrorHandlerToStack ( lua_State &vm ) noexcept
{
    lua_pushcfunction ( &vm, &ScriptEngine::OnErrorHandler );
    return lua_gettop ( &vm );
}

bool ScriptEngine::ExtendFrontend ( android_vulkan::Renderer &renderer,
    android_vulkan::SoundMixer &soundMixer
) const noexcept
{
    lua_State &vm = *_vm;

    AnimationGraph::Init ( vm, renderer );
    AnimationPlayerNode::Init ( vm );
    AnimationBlendNode::Init ( vm );
    BitField::Init ( vm );
    ScriptableGXMat3::Init ( vm );
    ScriptableGXMat4::Init ( vm );
    ScriptableGXQuat::Init ( vm );
    ScriptableGXVec3::Init ( vm );
    ScriptableGXVec4::Init ( vm );

    UILayer::InitLuaFrontend ( vm );
    ScriptableUIElement::InitCommon ( vm );
    ScriptableTextUIElement::Init ( vm );

    Component::Register ( vm );
    ScriptableLogger::Register ( vm );

    return ScriptComponent::Init ( vm ) &&
        Actor::Init ( vm ) &&
        RigidBodyComponent::Init ( vm ) &&
        CameraComponent::Init ( vm ) &&
        TransformComponent::Init ( vm ) &&
        StaticMeshComponent::Init ( vm, renderer ) &&
        ScriptableMaterial::Init ( vm, renderer ) &&
        SoundEmitterGlobalComponent::Init ( vm, soundMixer ) &&
        SoundEmitterSpatialComponent::Init ( vm, soundMixer ) &&
        SkeletalMeshComponent::Init ( vm, renderer );
}

bool ScriptEngine::InitInterfaceFunctions ( android_vulkan::Renderer &renderer,
    android_vulkan::SoundMixer &soundMixer
) noexcept
{
    lua_State* vm = _vm.get ();

    if ( int code = lua_pcall ( vm, 0, 0, 0 ); code != LUA_OK )
    {
        constexpr char const* cases[] =
        {
            "LUA_OK",
            "LUA_YIELD",
            "LUA_ERRRUN",
            "LUA_ERRSYNTAX",
            "LUA_ERRMEM",
            "LUA_ERRERR"
        };

        android_vulkan::LogError ( "pbr::ScriptEngine::InitInterfaceFunctions - Can't init script [%s].",
            cases[ code ]
        );

        return false;
    }

    lua_pushcfunction ( vm, &ScriptEngine::OnErrorHandler );
    return ExtendFrontend ( renderer, soundMixer );
}

bool ScriptEngine::InitLua () noexcept
{
    _vm.reset ( lua_newstate ( &ScriptEngine::OnAlloc, nullptr, 0U ) );

    if ( !_vm )
    {
        android_vulkan::LogError ( "pbr::ScriptEngine::InitLua - Can't create Lua VM." );
        return false;
    }

    InitLogFacility ();
    InitLibraries ();
    return InitCustomLoader ();
}

void ScriptEngine::InitLibraries () const noexcept
{
    // The idea is taken from luaL_openlibs implementation.
    // Commit SHA-1: 8426d9b4d4df1da3c5b2d759e509ae1c50a86667

    constexpr luaL_Reg const libs[] =
    {
        {
            .name = LUA_GNAME,
            .func = &luaopen_base
        },
        {
            .name = LUA_LOADLIBNAME,
            .func = &luaopen_package
        },
        {
            .name = LUA_MATHLIBNAME,
            .func = &luaopen_math
        },
        {
            .name = LUA_STRLIBNAME,
            .func = &luaopen_string
        },
        {
            .name = LUA_TABLIBNAME,
            .func = &luaopen_table
        }
    };

    lua_State* vm = _vm.get ();

    for ( auto const &lib : libs )
        luaL_requiref ( vm, lib.name, lib.func, 1 );

    lua_pop ( vm, static_cast<int> ( std::size ( libs ) ) );

    BanFunctions ();
}

void ScriptEngine::InitLogFacility () const noexcept
{
    lua_State* vm = _vm.get ();
    lua_atpanic ( vm, &ScriptEngine::OnPanic );
    lua_setwarnf ( vm, &ScriptEngine::OnWarning, vm );
}

bool ScriptEngine::InitCustomLoader () const noexcept
{
    // The idea is taken from here
    // https://gamedev.net/forums/topic/661707-solved-lua-34require34-other-files-when-loaded-from-memory/5185707/

    lua_State* vm = _vm.get ();

    if ( int const type = lua_getglobal ( vm, "package" ); type != LUA_TTABLE )
    {
        android_vulkan::LogError ( "pbr::ScriptEngine::InitCustomLoader - Can't locate package table." );
        lua_pop ( vm, 1 );
        return false;
    }

    if ( int const type = lua_getfield ( vm, -1, "searchers" ); type != LUA_TTABLE )
    {
        android_vulkan::LogError ( "pbr::ScriptEngine::InitCustomLoader - Can't locate searchers table." );
        lua_pop ( vm, 2 );
        return false;
    }

    lua_remove ( vm, -2 );

    // Note Lua uses indexing from 1.
    lua_rawgeti ( vm, -1, 1 );

    // Making our custom searcher the first Lua searcher.
    lua_pushcfunction ( vm, &ScriptEngine::OnRequire );
    lua_rawseti ( vm, -3, 1 );

    // Restoring the old first searcher as last searcher.
    lua_rawseti ( vm, -2, static_cast<lua_Integer> ( lua_rawlen ( vm, -2 ) ) + 1 );

    lua_pop ( vm, 1 );
    return true;
}

void ScriptEngine::BanFunctions () const noexcept
{
    lua_State* vm = _vm.get ();

    constexpr char const* baseLibrary[] =
    {
        "dofile",
        "print",
        "xpcall"
    };

    for ( auto const* f : baseLibrary )
    {
        lua_pushnil ( vm );
        lua_setglobal ( vm, f );
    }
}

void ScriptEngine::Free ( lua_State* state ) noexcept
{
    lua_close ( state );

    AnimationBlendNode::Destroy ();
    AnimationPlayerNode::Destroy ();
    AnimationGraph::Destroy ();
    SkeletalMeshComponent::Destroy ();
    SoundEmitterSpatialComponent::Destroy ();
    SoundEmitterGlobalComponent::Destroy ();
    ScriptableGXVec4::Destroy ();
    ScriptableGXVec3::Destroy ();
    ScriptableGXQuat::Destroy ();
    ScriptableGXMat4::Destroy ();
    ScriptableGXMat3::Destroy ();
    BitField::Destroy ();
    RigidBodyComponent::Destroy ();
    StaticMeshComponent::Destroy ();
    ScriptComponent::Destroy ();
    ScriptableMaterial::Destroy ();
    Actor::Destroy ();
    ScriptableUIElement::Destroy ();
    UILayer::Destroy ();
}

bool ScriptEngine::LoadScript ( lua_State* vm,
    std::string_view const &physicalPath,
    std::string_view const &logicalPath
) noexcept
{
    android_vulkan::File file ( physicalPath );

    if ( !file.LoadContent () )
        return false;

    std::vector<uint8_t> const &data = file.GetContent ();

    int const result = luaL_loadbufferx ( vm,
        reinterpret_cast<char const*> ( data.data () ),
        data.size (),
        logicalPath.data (),
        nullptr
    );

    if ( result == LUA_OK )
        return true;

    constexpr char const format[] =
R"__(pbr::ScriptEngine::LoadScript - Can't upload script chunk.
>>> [Lua VM error]:
%s)__";

    android_vulkan::LogError ( format, lua_tostring ( vm, -1 ) );
    lua_pop ( vm, 1 );
    return false;
}

void* ScriptEngine::OnAlloc ( void* /*ud*/, void* ptr, size_t /*osize*/, size_t nsize )
{
    if ( !nsize )
    {
        std::free ( ptr );
        return nullptr;
    }

    return std::realloc ( ptr, nsize );
}

int ScriptEngine::OnErrorHandler ( lua_State* state )
{
    // The error string from Lua VM is the first parameter in the stack.

    luaL_traceback ( state, state, lua_tostring ( state, -1 ), 1 );
    android_vulkan::LogError ( "pbr::ScriptEngine::OnErrorHandler - Error: %s", lua_tostring ( state, -1 ) );
    lua_pop ( state, 1 );

    AV_ASSERT ( false )
    return 0;
}

int ScriptEngine::OnPanic ( lua_State* state )
{
    char const* message = lua_tostring ( state, -1 );

    if ( !message )
        message = "Error object is not a string";

    constexpr char const format[] =
R"__(pbr::ScriptEngine::OnPanic - Lua VM:
%s)__";


    android_vulkan::LogError ( format, message );
    AV_ASSERT ( false )

    // Abort Lua VW
    return 0;
}

int ScriptEngine::OnRequire ( lua_State* state )
{
    if ( int const argc = lua_gettop ( state ); argc != 1 )
    {
        android_vulkan::LogError ( "pbr::ScriptEngine::OnRequire - Unexpected amount arguments: %d [must be 1].",
            argc
        );

        return 0;
    }

    std::string_view const m ( lua_tostring ( state, -1 ) );
    android_vulkan::LogDebug ( "pbr::ScriptEngine::OnRequire - Got '%s'.", m.data () );

    constexpr std::string_view prefix ( "av://" );

    if ( !m.starts_with ( prefix ) )
    {
        lua_pushnil ( state );
        return 1;
    }

    std::string path = std::string ( "pbr/" ) + m.substr ( prefix.size () ).data ();

    if ( !android_vulkan::File ( path.c_str () ).IsExist () )
    {
        lua_pushnil ( state );
        return 1;
    }

    constexpr char separator = '>';

    constexpr auto loader = [] ( lua_State* state ) -> int {
        if ( int const argc = lua_gettop ( state ); argc != 2 )
        {
            android_vulkan::LogError (
                "pbr::ScriptEngine::OnRequire::loader - Unexpected amount arguments: %d [must be 2].",
                argc
            );

            return 0;
        }

        // Decode physical path and logical path (which is used in the script) from one big string.
        std::string enc ( lua_tostring ( state, 2 ) );
        auto const sep = enc.find ( separator );
        enc[ sep ] = '\0';
        auto const factor = sep + 1U;
        std::string_view const logicalPath = std::string_view ( enc.data () + factor, enc.size () - factor );

        if ( LoadScript ( state, std::string_view ( enc.data (), factor ), logicalPath ) )
        {
            lua_pcall ( state, 0, 1, GetErrorHandlerIndex () );
            return 1;
        }

        android_vulkan::LogError ( "pbr::ScriptEngine::OnRequire::loader - Failed [%s].", logicalPath.data () );
        return 0;
    };

    // Encode physical path and logical path (which is used in the script) in one big string.
    path += separator;
    path += m;

    lua_pushcfunction ( state, loader );
    lua_pushstring ( state, path.c_str () );
    return 2;
}

void ScriptEngine::OnWarning ( void* /*ud*/, char const* message, int /*tocont*/ )
{
    // Note the implementation does not follow Lua conventions about control messages in logs for simplicity sake.
    // See https://www.lua.org/manual/5.4/manual.html#pdf-warn

    constexpr char const format[] =
R"__(pbr::ScriptEngine::OnWarning - Lua VM:
%s)__";

    android_vulkan::LogWarning ( format, message );
    AV_ASSERT ( false )
}

} // namespace pbr
