#include <pbr/scene.h>
#include <pbr/coordinate_system.h>
#include <pbr/renderable_component.h>
#include <pbr/script_engine.h>
#include <core.h>

GX_DISABLE_COMMON_WARNINGS

extern "C" {

#include <lua/lauxlib.h>

} // extern "C"

GX_RESTORE_WARNING_STATE


namespace pbr {

constexpr static float DEFAULT_ASPECT_RATIO = 1920.0F / 1080.0F;
constexpr static float DEFAULT_FOV = 60.0F;
constexpr static GXVec3 DEFAULT_LOCATION ( 0.0F, 0.0F, 0.0F );
constexpr static float DEFAULT_Z_NEAR = 1.0e-1F;
constexpr static float DEFAULT_Z_FAR = 1.0e+4F;

//----------------------------------------------------------------------------------------------------------------------

bool Scene::ExecuteInputEvents () noexcept
{
    return _gamepad.Execute ( *_vm, _sceneHandle, _onInputIndex );
}

GXMat4 const& Scene::GetActiveCameraLocalMatrix () const noexcept
{
    return _camera->GetLocalMatrix ();
}

GXMat4 const& Scene::GetActiveCameraProjectionMatrix () const noexcept
{
    return _camera->GetProjectionMatrix ();
}

void Scene::OnCaptureInput () noexcept
{
    _gamepad.CaptureInput ();
}

void Scene::OnReleaseInput () const noexcept
{
    _gamepad.ReleaseInput ();
}

bool Scene::OnInitDevice ( android_vulkan::Physics &physics ) noexcept
{
    _defaultCamera.SetProjection ( GXDegToRad ( DEFAULT_FOV ), DEFAULT_ASPECT_RATIO, DEFAULT_Z_NEAR, DEFAULT_Z_FAR );

    GXMat4 defaultTransform {};
    defaultTransform.Translation ( DEFAULT_LOCATION );
    _defaultCamera.SetLocal ( defaultTransform );
    _camera = &_defaultCamera;

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
        },

        {
            .name = "av_SceneGetRenderTargetAspectRatio",
            .func = &Scene::OnGetRenderTargetAspectRatio
        },

        {
            .name = "av_SceneGetRenderTargetWidth",
            .func = &Scene::OnGetRenderTargetWidth
        },

        {
            .name = "av_SceneGetRenderTargetHeight",
            .func = &Scene::OnGetRenderTargetHeight
        },

        {
            .name = "av_SceneQuit",
            .func = &Scene::OnQuit
        },

        {
            .name = "av_SceneSetActiveCamera",
            .func = &Scene::OnSetActiveCamera
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

    if ( !bind ( "OnInput", _onInputIndex ) )
        return false;

    if ( !bind ( "OnPostPhysics", _onPostPhysicsIndex ) )
        return false;

    if ( !bind ( "OnPrePhysics", _onPrePhysicsIndex ) )
        return false;

    if ( !bind ( "OnRenderTargetChanged", _onRenderTargetChangedIndex ) )
        return false;

    if ( !bind ( "OnUpdate", _onUpdateIndex ) )
        return false;

    return _gamepad.Init ( *_vm );
}

void Scene::OnDestroyDevice () noexcept
{
    _gamepad.Destroy ();
    _freeTransferResourceList.clear ();
    _renderableList.clear ();
    _actors.clear ();

    ScriptEngine::Destroy ();

    _physics = nullptr;

    _appendActorIndex = std::numeric_limits<int>::max ();
    _onInputIndex = std::numeric_limits<int>::max ();
    _onPostPhysicsIndex = std::numeric_limits<int>::max ();
    _onPrePhysicsIndex = std::numeric_limits<int>::max ();
    _onRenderTargetChangedIndex = std::numeric_limits<int>::max ();
    _onUpdateIndex = std::numeric_limits<int>::max ();

    _sceneHandle = std::numeric_limits<int>::max ();
    _vm = nullptr;
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

bool Scene::OnResolutionChanged ( VkExtent2D const &resolution, double aspectRatio ) noexcept
{
    auto const w = static_cast<lua_Integer> ( resolution.width );
    auto const h = static_cast<lua_Integer> ( resolution.height );

    if ( ( w == _width ) & ( h == _height ) & ( aspectRatio == _aspectRatio ) )
        return true;

    _aspectRatio = static_cast<lua_Number> ( aspectRatio );
    _width = static_cast<lua_Integer> ( resolution.width );
    _height = static_cast<lua_Integer> ( resolution.height );

    if ( !lua_checkstack ( _vm, 2 ) )
    {
        android_vulkan::LogError ( "pbr::Scene::OnResolutionChanged - Stack too small." );
        return false;
    }

    lua_pushvalue ( _vm, _onRenderTargetChangedIndex );
    lua_pushvalue ( _vm, _sceneHandle );
    lua_pcall ( _vm, 1, 0, ScriptEngine::GetErrorHandlerIndex () );

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

int Scene::OnGetRenderTargetAspectRatio ( lua_State* state )
{
    if ( !lua_checkstack ( state, 1 ) )
    {
        android_vulkan::LogError ( "pbr::Scene::OnGetRenderTargetAspectRatio - Stack too small." );
        return 0;
    }

    auto const& self = *static_cast<Scene const*> ( lua_touserdata ( state, 1 ) );
    lua_pushnumber ( state, self._aspectRatio );
    return 1;
}

int Scene::OnGetRenderTargetWidth ( lua_State* state )
{
    if ( !lua_checkstack ( state, 1 ) )
    {
        android_vulkan::LogError ( "pbr::Scene::OnGetRenderTargetWidth - Stack too small." );
        return 0;
    }

    auto const& self = *static_cast<Scene const*> ( lua_touserdata ( state, 1 ) );
    lua_pushinteger ( state, self._width );
    return 1;
}

int Scene::OnGetRenderTargetHeight ( lua_State* state )
{
    if ( !lua_checkstack ( state, 1 ) )
    {
        android_vulkan::LogError ( "pbr::Scene::OnGetRenderTargetHeight - Stack too small." );
        return 0;
    }

    auto const& self = *static_cast<Scene const*> ( lua_touserdata ( state, 1 ) );
    lua_pushinteger ( state, self._height );
    return 1;
}

int Scene::OnQuit ( lua_State* /*state*/ )
{
    android_vulkan::Core::Quit ();
    return 0;
}

int Scene::OnSetActiveCamera ( lua_State* state )
{
    auto& self = *static_cast<Scene*> ( lua_touserdata ( state, 1 ) );
    self._camera = static_cast<CameraComponent*> ( lua_touserdata ( state, 2 ) );
    return 0;
}

} // namespace pbr
