#include <pbr/actor.h>
#include <pbr/script_component.h>
#include <pbr/rigid_body_component.h>
#include <guid_generator.h>
#include <logger.h>


namespace pbr {

Actor::Actor () noexcept:
    _name ( android_vulkan::GUID::GenerateAsString ( "Actor" ) )
{
    // NOTHING
}

Actor::Actor ( std::string &&name ) noexcept:
    _name ( std::move ( name ) )
{
    // NOTHING
}

void Actor::AppendComponent ( ComponentRef &component ) noexcept
{
    _componentStorage[ component->GetName () ].push_back ( component );
}

[[maybe_unused]] Actor::FindResult Actor::FindComponents ( std::string const &componentName ) noexcept
{
    auto findResult = _componentStorage.find ( componentName );

    return findResult == _componentStorage.end () ?
        std::nullopt :
        std::optional<std::reference_wrapper<Components>> { findResult->second };
}

std::string const& Actor::GetName () const noexcept
{
    return _name;
}

void Actor::RegisterComponents ( ComponentList &freeTransferResource,
    ComponentList &renderable,
    android_vulkan::Physics &physics,
    ScriptEngine &scriptEngine
) noexcept
{
    for ( auto& pair : _componentStorage )
    {
        Components& components = pair.second;

        for ( auto& component : components )
        {
            ClassID const classID = component->GetClassID ();

            if ( classID == ClassID::Reflection | classID == ClassID::StaticMesh )
            {
                freeTransferResource.emplace_back ( std::ref ( component ) );
                continue;
            }

            if ( classID == ClassID::RigidBody )
            {
                // NOLINTNEXTLINE - downcast.
                auto& rigiBodyComponent = static_cast<RigidBodyComponent&> ( *component );

                if ( !rigiBodyComponent.Register ( physics ) )
                {
                    android_vulkan::LogWarning ( "pbr::Actor::RegisterComponents - Can't register rigid body "
                        "component %s.",
                        rigiBodyComponent.GetName ().c_str ()
                    );
                }

                continue;
            }

            if ( classID == ClassID::PointLight )
            {
                renderable.emplace_back ( std::ref ( component ) );
                continue;
            }

            if ( classID == ClassID::Script )
            {
                // NOLINTNEXTLINE - downcast.
                auto& scriptComponent = static_cast<ScriptComponent&> ( *component );

                if ( scriptComponent.Register ( scriptEngine ) )
                    continue;

                android_vulkan::LogWarning ( "pbr::Actor::RegisterComponents - Can't register script component %s.",
                    scriptComponent.GetName ().c_str ()
                );
            }
        }
    }
}

} // namespace pbr
