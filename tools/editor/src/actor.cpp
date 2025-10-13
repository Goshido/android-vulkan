#include <precompiled_headers.hpp>
#include <actor.hpp>


namespace editor {

namespace {

constexpr std::string_view VERSION_KEY = "version";
constexpr uint32_t VERSION = 1U;

constexpr std::string_view NAME_KEY = "name";
constexpr std::string_view LOCAL_KEY = "local";

constexpr std::string_view COMPONENTS_KEY = "components";

} // end of anonymous namespace

//----------------------------------------------------------------------------------------------------------------------

Actor::Actor ( SaveState::Container const &info ) noexcept:
    _name ( info.Read ( NAME_KEY, DEFAULT_NAME ) )
{
    _local = info.Read ( LOCAL_KEY, GXMat4::IDENTITY );
    SaveState::Container const &components = info.ReadArray ( COMPONENTS_KEY );

    for ( size_t i = 0U, count = components.GetArraySize (); i < count; ++i )
    {
        if ( auto component = Component::Spawn ( components.ReadContainer () ); component ) [[likely]]
        {
            Append ( std::move ( *component ) );
        }
    }
}

void Actor::Append ( std::unique_ptr<Component> &&component ) noexcept
{
    _components.push_back ( std::move ( component ) );
}

void Actor::Insert ( size_t before, std::unique_ptr<Component> &&component ) noexcept
{
    _components.insert ( _components.cbegin () + static_cast<Components::const_iterator::difference_type> ( before ),
        std::move ( component )
    );
}

void Actor::Save ( SaveState::Container &root ) const noexcept
{
    root.Write ( VERSION_KEY, VERSION );
    root.Write ( LOCAL_KEY, _local );

    SaveState::Container &components = root.WriteArray ( COMPONENTS_KEY );

    for ( auto const &component : _components )
    {
        component->Save ( components.WriteContainer () );
    }
}

} // namespace editor
