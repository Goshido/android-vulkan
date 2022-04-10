#include <pbr/actor.h>
#include <pbr/component.h>
#include <guid_generator.h>


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

[[maybe_unused]] void Actor::AppendComponent ( ComponentRef &component ) noexcept
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

void Actor::RegisterRenderableComponents ( ComponentList &componentList ) noexcept
{
    for ( auto& pair : _componentStorage )
    {
        Components& components = pair.second;

        for ( auto& component : components )
        {
            if ( !component->IsRenderable () )
                continue;

            componentList.emplace_back ( std::ref ( component ) );
        }
    }
}

} // namespace pbr
