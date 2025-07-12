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

// Note dedicated cpp is needed to avoid static initialization order fiasco (SIOF)
// https://en.cppreference.com/w/cpp/language/siof.html
Component::Spawners Component::_spawners {};

Component::Factory<CameraComponent> const CameraComponent::_factory ( "CameraComponent" );
Component::Factory<PointLightComponent> const PointLightComponent::_factory ( "PointLightComponent" );
Component::Factory<ReflectionComponent> const ReflectionComponent::_factory ( "ReflectionComponent" );
Component::Factory<RigidBodyComponent> const RigidBodyComponent::_factory ( "RigidBodyComponent" );
Component::Factory<ScriptComponent> const ScriptComponent::_factory ( "ScriptComponent" );
Component::Factory<SkeletalMeshComponent> const SkeletalMeshComponent::_factory ( "SkeletalMeshComponent" );
Component::Factory<SoundEmitterComponent> const SoundEmitterComponent::_factory ( "SoundEmitterComponent" );
Component::Factory<StaticMeshComponent> const StaticMeshComponent::_factory ( "StaticMeshComponent" );
Component::Factory<TransformComponent> const TransformComponent::_factory ( "TransformComponent" );

} // namespace editor
