#include <pbr/scene.h>
#include <pbr/coordinate_system.h>
#include <pbr/renderable_component.h>
#include <pbr/script_engine.h>

GX_DISABLE_COMMON_WARNINGS

extern "C" {

#include <lua/lauxlib.h>

} // extern "C"

GX_RESTORE_WARNING_STATE


namespace pbr {

bool Scene::OnInitDevice ( android_vulkan::Physics &physics ) noexcept
{
    _physics = &physics;
    ScriptEngine& scriptEngine = ScriptEngine::GetInstance ();

    if ( !scriptEngine.Init () )
        return false;

    _vm = &scriptEngine.GetVirtualMachine ();

    if ( !lua_checkstack ( _vm, 5 ) )
    {
        android_vulkan::LogError ( "pbr::Scene::OnInitDevice - Stack too small." );
        return false;
    }

    if ( int const type = lua_getglobal ( _vm, "CreateScene" ); type != LUA_TFUNCTION )
        return false;

    lua_pushlightuserdata ( _vm, this );

    if ( lua_pcall ( _vm, 1, 1, ScriptEngine::GetErrorHandlerIndex () ) != LUA_OK )
        return false;

    constexpr luaL_Reg const extentions[] =
    {
        {
            .name = "av_SceneGetPhysicsToRendererScaleFactor",
            .func = &Scene::OnGetPhysicsToRendererScaleFactor
        },

        {
            .name = "av_SceneGetRendererToPhysicsScaleFactor",
            .func = &Scene::OnGetRendererToPhysicsScaleFactor
        }
    };

    for ( auto const& extension : extentions )
        lua_register ( _vm, extension.name, extension.func );

    _sceneHandle = lua_gettop ( _vm );

    auto bind = [ & ] ( std::string_view const &&method, int &ind ) noexcept -> bool {
        lua_pushlstring ( _vm, method.data (), method.size () );

        if ( lua_rawget ( _vm, _sceneHandle ) == LUA_TFUNCTION )
        {
            ind = lua_gettop ( _vm );
            return true;
        }

        android_vulkan::LogError ( "pbr::Scene::OnInitDevice - Can't find %s method.", method );
        return false;
    };

    if ( !bind ( "AppendActor", _appendActorIndex ) )
        return false;

    if ( !bind ( "OnPostPhysics", _onPostPhysicsIndex ) )
        return false;

    if ( !bind ( "OnPrePhysics", _onPrePhysicsIndex ) )
        return false;

    return bind ( "OnUpdate", _onUpdateIndex );
}

void Scene::OnDestroyDevice () noexcept
{
    _freeTransferResourceList.clear ();
    _renderableList.clear ();
    _actors.clear ();

    ScriptEngine::Destroy ();

    _physics = nullptr;
    _vm = nullptr;
    _onPostPhysicsIndex = std::numeric_limits<int>::max ();
    _onPrePhysicsIndex = std::numeric_limits<int>::max ();
    _onUpdateIndex = std::numeric_limits<int>::max ();
    _sceneHandle = std::numeric_limits<int>::max ();
}

bool Scene::OnPrePhysics ( double deltaTime ) noexcept
{
    if ( !lua_checkstack ( _vm, 3 ) )
    {
        android_vulkan::LogError ( "pbr::Scene::OnPrePhysics - Stack too small." );
        return false;
    }

    lua_pushvalue ( _vm, _onPrePhysicsIndex );
    lua_pushvalue ( _vm, _sceneHandle );
    lua_pushnumber ( _vm, deltaTime );
    lua_pcall ( _vm, 2, 0, ScriptEngine::GetErrorHandlerIndex () );

    return true;
}

bool Scene::OnPostPhysics ( double deltaTime ) noexcept
{
    if ( !lua_checkstack ( _vm, 3 ) )
    {
        android_vulkan::LogError ( "pbr::Scene::OnPostPhysics - Stack too small." );
        return false;
    }

    lua_pushvalue ( _vm, _onPostPhysicsIndex );
    lua_pushvalue ( _vm, _sceneHandle );
    lua_pushnumber ( _vm, deltaTime );
    lua_pcall ( _vm, 2, 0, ScriptEngine::GetErrorHandlerIndex () );

    return true;
}

bool Scene::OnUpdate ( double deltaTime ) noexcept
{
    if ( !lua_checkstack ( _vm, 3 ) )
    {
        android_vulkan::LogError ( "pbr::Scene::OnUpdate - Stack too small." );
        return false;
    }

    lua_pushvalue ( _vm, _onUpdateIndex );
    lua_pushvalue ( _vm, _sceneHandle );
    lua_pushnumber ( _vm, deltaTime );
    lua_pcall ( _vm, 2, 0, ScriptEngine::GetErrorHandlerIndex () );

    return true;
}

void Scene::AppendActor ( ActorRef &actor ) noexcept
{
    lua_pushvalue ( _vm, _appendActorIndex );
    lua_pushvalue ( _vm, _sceneHandle );

    _actors.push_back ( actor );
    _actors.back ()->RegisterComponents ( _freeTransferResourceList, _renderableList, *_physics, *_vm );

    lua_pcall ( _vm, 2, 0, ScriptEngine::GetErrorHandlerIndex () );
}

void Scene::FreeTransferResources ( VkDevice device ) noexcept
{
    for ( auto& component : _freeTransferResourceList )
    {
        // NOLINTNEXTLINE - downcast.
        auto& renderableComponent = static_cast<RenderableComponent&> ( *component.get () );
        renderableComponent.FreeTransferResources ( device );
    }

    _renderableList.splice ( _renderableList.cend (), _freeTransferResourceList );
}

void Scene::Submit ( RenderSession &renderSession ) noexcept
{
    for ( auto& component : _renderableList )
    {
        // NOLINTNEXTLINE - downcast.
        auto& renderableComponent = static_cast<RenderableComponent&> ( *component.get () );
        renderableComponent.Submit ( renderSession );
    }
}

int Scene::OnGetPhysicsToRendererScaleFactor ( lua_State* state )
{
    if ( lua_checkstack ( state, 1 ) )
    {
        constexpr auto scaleFactor = static_cast<lua_Number> ( UNITS_IN_METER );
        lua_pushnumber ( state, scaleFactor );
        return 1;
    }

    android_vulkan::LogError ( "pbr::Scene::OnGetPhysicsToRendererScaleFactor - Stack too small." );
    return 0;
}

int Scene::OnGetRendererToPhysicsScaleFactor ( lua_State* state )
{
    if ( lua_checkstack ( state, 1 ) )
    {
        constexpr auto scaleFactor = static_cast<lua_Number> ( METERS_IN_UNIT );
        lua_pushnumber ( state, scaleFactor );
        return 1;
    }

    android_vulkan::LogError ( "pbr::Scene::OnGetPhysicsToRendererScaleFactor - Stack too small." );
    return 0;
}

} // namespace pbr
