#include <pbr/scene.h>
#include <pbr/coordinate_system.h>
#include <pbr/mesh_manager.h>
#include <pbr/material_manager.h>
#include <pbr/renderable_component.h>
#include <pbr/scene_desc.h>
#include <pbr/script_engine.h>
#include <pbr/scriptable_gxmat4.h>
#include <pbr/scriptable_gxvec3.h>
#include <pbr/scriptable_material.h>
#include <pbr/scriptable_sweep_test_result.h>
#include <pbr/static_mesh_component.h>
#include <core.h>
#include <file.h>
#include <shape_box.h>
#include <trace.h>

GX_DISABLE_COMMON_WARNINGS

#include <cassert>

extern "C" {

#include <lua/lauxlib.h>

} // extern "C"

GX_RESTORE_WARNING_STATE


namespace pbr {

namespace {

constexpr float DEFAULT_ASPECT_RATIO = 1920.0F / 1080.0F;
constexpr float DEFAULT_FOV = 60.0F;
constexpr GXVec3 DEFAULT_LOCATION ( 0.0F, 0.0F, 0.0F );
constexpr float DEFAULT_Z_NEAR = 1.0e-1F;
constexpr float DEFAULT_Z_FAR = 1.0e+4F;

constexpr size_t INITIAL_PENETRATION_SIZE = 32U;

[[maybe_unused]] constexpr uint32_t SCENE_DESC_FORMAT_VERSION = 3U;

constexpr GXVec3 FUCK_LOCATION ( 479.717F, 190.419F, 134.596F );

} // end of anonymous namespace

//----------------------------------------------------------------------------------------------------------------------

void Scene::DetachRenderable ( RenderableComponent const &component ) noexcept
{
    auto const end = _renderableList.cend ();

    auto const findResult = std::find_if ( _renderableList.cbegin (),
        end,

        [ &component ] ( auto const &e ) noexcept -> bool {
            return &e.get () == &component;
        }
    );

    if ( findResult != end )
    {
        _renderableList.erase ( findResult );
        return;
    }

    android_vulkan::LogError ( "pbr::Scene::DetachRenderable - Can't remove component %s.",
        component.GetName ().c_str ()
    );
}

bool Scene::ExecuteInputEvents () noexcept
{
    AV_TRACE ( "Lua: input events" )
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

android_vulkan::Physics& Scene::GetPhysics () noexcept
{
    assert ( _physics );
    return *_physics;
}

bool Scene::OnInitDevice ( android_vulkan::Renderer &renderer, android_vulkan::Physics &physics ) noexcept
{
    if ( !_soundMixer.Init () )
        return false;

    GXMat3 o {};
    o.SetX ( GXVec3 ( 0.0F, 0.0F, 1.0F ) );
    o.SetY ( GXVec3 ( 0.0F, 1.0F, 0.0F ) );
    o.SetZ ( GXVec3 ( -1.0F, 0.0F, 0.0F ) );

    GXQuat orientation {};
    orientation.FromFast ( o );

    _soundMixer.SetListenerOrientation ( orientation );
    _soundMixer.SetListenerLocation ( FUCK_LOCATION );

    if ( !_soundEmitter.Init ( _soundMixer, android_vulkan::eSoundChannel::Music, FUCK_LOCATION, 10.0F ) )
        return false;

    if ( !_soundEmitter.SetSoundAsset ( _soundStorage, "sounds/sine.wav", true ) )
    //    if ( !_soundEmitter.SetSoundAsset ( _soundStorage, "sounds/sine.ogg", true ) )
    //    if ( !_soundEmitter.SetSoundAsset ( _soundStorage, "sounds/sine_stereo.wav", true ) )
    //    if ( !_soundEmitter.SetSoundAsset ( _soundStorage, "sounds/sine_stereo.ogg", true ) )
//    if ( !_soundEmitter.SetSoundAsset ( _soundStorage, "sounds/Credits.ogg", false ) )
        return false;

    _defaultCamera.SetProjection ( GXDegToRad ( DEFAULT_FOV ), DEFAULT_ASPECT_RATIO, DEFAULT_Z_NEAR, DEFAULT_Z_FAR );
    _penetrations.reserve ( INITIAL_PENETRATION_SIZE );

    _shapeBoxes[ 0U ] = std::make_shared<android_vulkan::ShapeBox> ( 1.0F, 1.0F, 1.0F );
    _shapeBoxes[ 1U ] = std::make_shared<android_vulkan::ShapeBox> ( 1.0F, 1.0F, 1.0F );

    GXMat4 defaultTransform {};
    defaultTransform.Translation ( DEFAULT_LOCATION );
    _defaultCamera.SetLocal ( defaultTransform );
    _camera = &_defaultCamera;

    _physics = &physics;
    ScriptEngine& scriptEngine = ScriptEngine::GetInstance ();

    if ( !scriptEngine.Init ( renderer ) )
        return false;

    _vm = &scriptEngine.GetVirtualMachine ();

    if ( !lua_checkstack ( _vm, 8 ) )
    {
        android_vulkan::LogError ( "pbr::Scene::OnInitDevice - Stack too small." );
        return false;
    }

    if ( lua_getglobal ( _vm, "CreateScene" ) != LUA_TFUNCTION )
        return false;

    lua_pushlightuserdata ( _vm, this );

    if ( lua_pcall ( _vm, 1, 1, ScriptEngine::GetErrorHandlerIndex () ) != LUA_OK )
        return false;

    constexpr luaL_Reg const extensions[] =
    {
        {
            .name = "av_SceneAppendActor",
            .func = &Scene::OnAppendActor
        },
        {
            .name = "av_SceneGetPenetrationBox",
            .func = &Scene::OnGetPenetrationBox
        },
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
            .name = "av_SceneOverlapTestBoxBox",
            .func = &Scene::OnOverlapTestBoxBox
        },
        {
            .name = "av_SceneQuit",
            .func = &Scene::OnQuit
        },
        {
            .name = "av_SceneSetActiveCamera",
            .func = &Scene::OnSetActiveCamera
        },
        {
            .name = "av_SceneSweepTestBox",
            .func = &Scene::OnSweepTestBox
        }
    };

    for ( auto const& extension : extensions )
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

    if ( !bind ( "AppendActorFromNative", _appendActorFromNativeIndex ) )
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

    return _scriptablePenetration.Init ( *_vm ) && ScriptableSweepTestResult::Init ( *_vm ) && _gamepad.Init ( *_vm );
}

void Scene::OnDestroyDevice () noexcept
{
    ScriptableSweepTestResult::Destroy ( *_vm );
    _scriptablePenetration.Destroy ( *_vm );
    _gamepad.Destroy ();
    _renderableList.clear ();
    _actors.clear ();

    ScriptEngine::Destroy ();

    _physics = nullptr;

    _penetrations.clear ();
    _penetrations.shrink_to_fit ();

    _sweepTestResult.clear ();
    _sweepTestResult.shrink_to_fit ();

    _shapeBoxes[ 0U ] = nullptr;
    _shapeBoxes[ 1U ] = nullptr;

    _appendActorFromNativeIndex = std::numeric_limits<int>::max ();
    _onInputIndex = std::numeric_limits<int>::max ();
    _onPostPhysicsIndex = std::numeric_limits<int>::max ();
    _onPrePhysicsIndex = std::numeric_limits<int>::max ();
    _onRenderTargetChangedIndex = std::numeric_limits<int>::max ();
    _onUpdateIndex = std::numeric_limits<int>::max ();

    _sceneHandle = std::numeric_limits<int>::max ();
    _vm = nullptr;

    if ( !_soundEmitter.Destroy () )
        android_vulkan::LogWarning ( "Scene::OnDestroyDevice - Can't destroy sound emitter." );

    _soundMixer.Destroy ();
}

void Scene::OnPause () noexcept
{
    _soundMixer.Pause ();
    _gamepad.ReleaseInput ();
}

void Scene::OnResume () noexcept
{
    _soundMixer.Resume ();
    _gamepad.CaptureInput ();
}

bool Scene::OnPrePhysics ( double deltaTime ) noexcept
{
    AV_TRACE ( "Lua: pre-physics" )

    {
        // TODO REMOVE
        static float a = 0.0F;
        a += deltaTime;

        static struct Once final {
            Once ( android_vulkan::SoundEmitterSpatial &e, android_vulkan::SoundMixer &mixer, float aa ) noexcept
            {
                if ( !e.Play () )
                {
                    android_vulkan::LogWarning ( "Bruh" );
                }

                GXQuat q {};
                q.FromAxisAngle ( 0.0F, 1.0F, 0.0F, aa );
                mixer.SetListenerOrientation ( q );
            }
        } doOnce ( _soundEmitter, _soundMixer, a );

        GXVec3 n = FUCK_LOCATION;
        n._data[ 0U ] += -1.0F;
        _soundEmitter.SetLocation ( n );

        GXQuat q {};
        q.FromAxisAngle ( 0.0F, 1.0F, 0.0F, a );
        _soundMixer.SetListenerOrientation ( q );
    }

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
    AV_TRACE ( "Lua: post-physics" )

    if ( !lua_checkstack ( _vm, 3 ) )
    {
        android_vulkan::LogError ( "pbr::Scene::OnPostPhysics - Stack too small." );
        return false;
    }

    lua_pushvalue ( _vm, _onPostPhysicsIndex );
    lua_pushvalue ( _vm, _sceneHandle );
    lua_pushnumber ( _vm, deltaTime );
    lua_pcall ( _vm, 2, 0, ScriptEngine::GetErrorHandlerIndex () );

    return ScriptableMaterial::Sync () && StaticMeshComponent::Sync ();
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
    AV_TRACE ( "Lua: update" )

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

bool Scene::LoadScene ( android_vulkan::Renderer &renderer, char const* scene, VkCommandPool commandPool ) noexcept
{
    android_vulkan::File file ( scene );

    if ( !file.LoadContent () )
        return false;

    std::vector<uint8_t> const& content = file.GetContent ();
    uint8_t const* data = content.data ();
    auto const& desc = *reinterpret_cast<pbr::SceneDesc const*> ( data );

    // Sanity checks.
    static_assert ( sizeof ( GXVec3 ) == sizeof ( desc._viewerLocation ) );
    assert ( desc._formatVersion == SCENE_DESC_FORMAT_VERSION );

    auto const comBuffs = static_cast<size_t> ( desc._textureCount + desc._meshCount + desc._envMapCount );

    VkCommandBufferAllocateInfo const allocateInfo
    {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
        .pNext = nullptr,
        .commandPool = commandPool,
        .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
        .commandBufferCount = static_cast<uint32_t> ( comBuffs )
    };

    std::vector<VkCommandBuffer> commandBuffers ( comBuffs );
    VkDevice device = renderer.GetDevice ();

    bool result = android_vulkan::Renderer::CheckVkResult (
        vkAllocateCommandBuffers ( device, &allocateInfo, commandBuffers.data () ),
        "pbr::Scene::LoadScene",
        "Can't allocate command buffers"
    );

    if ( !result )
        return false;

    VkCommandBuffer const* cb = commandBuffers.data ();
    uint8_t const* readPointer = data + sizeof ( pbr::SceneDesc );

    size_t consumed = 0U;
    size_t read = 0U;

    auto const actors = static_cast<size_t> ( desc._actorCount );

    for ( size_t actorIdx = 0U; actorIdx < actors; ++actorIdx )
    {
        auto const& actorDesc = *reinterpret_cast<ActorDesc const*> ( readPointer );
        readPointer += sizeof ( ActorDesc );
        auto const components = static_cast<size_t> ( actorDesc._components );

        ActorRef actor = std::make_shared<Actor> ( actorDesc, data );

        for ( size_t componentIdx = 0U; componentIdx < components; ++componentIdx )
        {
            ComponentRef component = Component::Create ( renderer,
                consumed,
                read,
                *reinterpret_cast<ComponentDesc const*> ( readPointer ),
                data,
                cb
            );

            if ( component )
                actor->AppendComponent ( component );

            cb += consumed;
            readPointer += read;
        }

        AppendActor ( actor );
    }

    result = android_vulkan::Renderer::CheckVkResult ( vkQueueWaitIdle ( renderer.GetQueue () ),
        "pbr::Scene::LoadScene",
        "Can't run upload commands"
    );

    if ( !result )
        return false;

    FreeTransferResources ( renderer );
    return true;
}

void Scene::RemoveActor ( Actor const &actor ) noexcept
{
    auto const findResult = std::find_if ( _actors.cbegin (),
        _actors.cend (),

        [ &actor ] ( ActorRef const &e ) noexcept -> bool {
            return e.get () == &actor;
        }
    );

    _actors.erase ( findResult );
}

void Scene::Submit ( android_vulkan::Renderer &renderer, RenderSession &renderSession ) noexcept
{
    AV_TRACE ( "Submit components" )
    FreeTransferResources ( renderer );

    for ( auto& component : _renderableList )
    {
        // NOLINTNEXTLINE - downcast.
        auto& renderableComponent = static_cast<RenderableComponent&> ( component.get () );
        renderableComponent.Submit ( renderSession );
    }
}

void Scene::AppendActor ( ActorRef &actor ) noexcept
{
    if ( !lua_checkstack ( _vm, 2 ) )
    {
        android_vulkan::LogError ( "pbr::Scene::AppendActor - Stack too small." );
        return;
    }

    lua_pushvalue ( _vm, _appendActorFromNativeIndex );
    lua_pushvalue ( _vm, _sceneHandle );

    _actors.push_back ( actor );
    _actors.back ()->RegisterComponentsFromNative ( *this, _renderableList, *_physics, *_vm );

    lua_pcall ( _vm, 2, 0, ScriptEngine::GetErrorHandlerIndex () );
}

int Scene::DoOverlapTestBoxBox ( lua_State &vm,
    GXMat4 const &localA,
    GXVec3 const &sizeA,
    GXMat4 const &localB,
    GXVec3 const &sizeB
) noexcept
{
    if ( !lua_checkstack ( &vm, 1 ) )
    {
        android_vulkan::LogError ( "pbr::Scene::DoOverlapTestBoxBox - Stack too small." );
        return 0;
    }

    android_vulkan::Shape& shapeA = *_shapeBoxes[ 0U ];
    android_vulkan::Shape& shapeB = *_shapeBoxes[ 1U ];

    auto const setup = [] ( android_vulkan::Shape &shape, GXMat4 const &local, GXVec3 const &size ) noexcept {
        // NOLINTNEXTLINE - downcast.
        auto& boxShape = static_cast<android_vulkan::ShapeBox&> ( shape );

        boxShape.Resize ( size );
        boxShape.UpdateCacheData ( local );
    };

    setup ( shapeA, localA, sizeA );
    setup ( shapeB, localB, sizeB );

    if ( !shapeA.GetBoundsWorld ().IsOverlaped ( shapeB.GetBoundsWorld () ) )
    {
        lua_pushboolean ( &vm, static_cast<int> ( false ) );
        return 1;
    }

    android_vulkan::GJK gjk {};
    gjk.Reset ();
    lua_pushboolean ( &vm, static_cast<int> ( gjk.Run ( shapeA, shapeB ) ) );
    return 1;
}

int Scene::DoPenetrationBox ( lua_State &vm, GXMat4 const &local, GXVec3 const &size, uint32_t groups ) noexcept
{
    android_vulkan::ShapeRef& shape = _shapeBoxes[ 0U ];

    // NOLINTNEXTLINE - downcast.
    auto& boxShape = static_cast<android_vulkan::ShapeBox&> ( *shape );

    boxShape.Resize ( size );
    boxShape.UpdateCacheData ( local );
    _physics->PenetrationTest ( _penetrations, _epa, shape, groups );
    return static_cast<int> ( _scriptablePenetration.PublishResult ( vm, _penetrations ) );
}

int Scene::DoSweepTestBox ( lua_State &vm, GXMat4 const &local, GXVec3 const &size, uint32_t groups ) noexcept
{
    android_vulkan::ShapeRef& shape = _shapeBoxes[ 0U ];

    // NOLINTNEXTLINE - downcast.
    auto& boxShape = static_cast<android_vulkan::ShapeBox&> ( *shape );

    boxShape.Resize ( size );
    boxShape.UpdateCacheData ( local );
    _physics->SweepTest ( _sweepTestResult, shape, groups );
    return static_cast<int> ( ScriptableSweepTestResult::PublishResult ( vm, _sweepTestResult ) );
}

void Scene::FreeTransferResources ( android_vulkan::Renderer &renderer ) noexcept
{
    MaterialManager::GetInstance ().FreeTransferResources ( renderer );
    MeshManager::GetInstance ().FreeTransferResources ( renderer );
}

int Scene::OnAppendActor ( lua_State* state )
{
    auto& self = *static_cast<Scene*> ( lua_touserdata ( state, 1 ) );
    self._actors.push_back ( Actor::GetReference ( *static_cast<Actor const*> ( lua_touserdata ( state, 2 ) ) ) );
    self._actors.back ()->RegisterComponentsFromScript ( self, self._renderableList, *self._physics );
    return 0;
}

int Scene::OnGetPenetrationBox ( lua_State* state )
{
    auto& self = *static_cast<Scene*> ( lua_touserdata ( state, 1 ) );

    return self.DoPenetrationBox ( *state,
        ScriptableGXMat4::Extract ( state, 2 ),
        ScriptableGXVec3::Extract ( state, 3 ),
        lua_tointeger ( state, 4 )
    );
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

int Scene::OnOverlapTestBoxBox ( lua_State* state )
{
    auto& self = *static_cast<Scene*> ( lua_touserdata ( state, 1 ) );

    return self.DoOverlapTestBoxBox ( *state,
        ScriptableGXMat4::Extract ( state, 2 ),
        ScriptableGXVec3::Extract ( state, 3 ),
        ScriptableGXMat4::Extract ( state, 4 ),
        ScriptableGXVec3::Extract ( state, 5 )
    );
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

int Scene::OnSweepTestBox ( lua_State* state )
{
    auto& self = *static_cast<Scene*> ( lua_touserdata ( state, 1 ) );

    return self.DoSweepTestBox ( *state,
        ScriptableGXMat4::Extract ( state, 2 ),
        ScriptableGXVec3::Extract ( state, 3 ),
        lua_tointeger ( state, 4 )
    );
}

} // namespace pbr
