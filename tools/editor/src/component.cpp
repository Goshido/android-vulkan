#include <precompiled_headers.hpp>
#include <camera_component.hpp>
#include <point_light_component.hpp>
#include <reflection_component.hpp>
#include <rigid_body_component.hpp>
#include <script_component.hpp>
#include <skeletal_mesh_component.hpp>
#include <sound_emitter_component.hpp>
#include <static_mesh_component.hpp>
#include <transform_component.hpp>


namespace editor {

namespace {

constexpr std::string_view VERSION_KEY = "version";
constexpr uint32_t DEFAULT_VERSION = 0U;

constexpr std::string_view DEFAULT_NAME = "component";
constexpr std::string_view LOCATION_KEY = "location";
constexpr std::string_view NAME_KEY = "name";
constexpr std::string_view ROTATION_KEY = "rotation";
constexpr std::string_view SCALE_KEY = "scale";

} // end of anonymous namespace

//----------------------------------------------------------------------------------------------------------------------

Component::Spawners Component::_spawners {};

Component::Component ( uint32_t version, std::string &&name ) noexcept:
    _version ( version ),
    _name ( std::move ( name ) )
{
    // NOTHING
}

Component::Component ( SaveState::Container const &info ) noexcept
{
    _version = info.Read ( VERSION_KEY, DEFAULT_VERSION );
    _name = info.Read ( NAME_KEY, DEFAULT_NAME );
    _rotation = info.Read ( ROTATION_KEY, GXQuat::IDENTITY );
    _scale = info.Read ( SCALE_KEY, GXVec3::ZERO );
    _location = info.Read ( LOCATION_KEY, GXVec3::ZERO );
}

void Component::Register ( Actor &actor ) noexcept
{
    _actor = &actor;
}

void Component::Unregister () noexcept
{
    _actor = nullptr;
}

void Component::ActorTransformChanged () noexcept
{
    // NOTHING
}

void Component::Save ( SaveState::Container &root ) const noexcept
{
    root.Write ( VERSION_KEY, _version );
    root.Write ( NAME_KEY, _name );
    root.Write ( ROTATION_KEY, _rotation );
    root.Write ( SCALE_KEY, _scale );
    root.Write ( LOCATION_KEY, _location );
}

void Component::SetName ( std::string_view name ) noexcept
{
    _name = name;
}

void Component::InitSpawners () noexcept
{
    InitSpawner<CameraComponent> ();
    InitSpawner<PointLightComponent> ();
    InitSpawner<ReflectionComponent> ();
    InitSpawner<RigidBodyComponent> ();
    InitSpawner<ScriptComponent> ();
    InitSpawner<SkeletalMeshComponent> ();
    InitSpawner<SoundEmitterComponent> ();
    InitSpawner<StaticMeshComponent> ();
    InitSpawner<TransformComponent> ();
}

std::optional<ComponentRef> Component::Spawn ( SaveState::Container const &info ) noexcept
{
    if ( auto const spawn = _spawners.find ( info.Read ( TYPE_KEY, std::string_view {} ) ); spawn != _spawners.cend () )
    {
        [[likely]]
        return spawn->second ( info );
    }

    return std::nullopt;
}

} // namespace editor
