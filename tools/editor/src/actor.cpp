#include <precompiled_headers.hpp>
#include <actor.hpp>
#include <av_assert.hpp>


namespace editor {

namespace {

constexpr std::string_view VERSION_KEY = "version";
constexpr uint32_t VERSION = 1U;

constexpr std::string_view NAME_KEY = "name";
constexpr std::string_view ROTATION_KEY = "rotation";
constexpr std::string_view SCALE_KEY = "scale";
constexpr std::string_view LOCATION_KEY = "location";

constexpr std::string_view COMPONENTS_KEY = "components";

} // end of anonymous namespace

//----------------------------------------------------------------------------------------------------------------------

Actor::Actor ( MessageQueue &messageQueue, SaveState::Container const &info ) noexcept:
    _name ( info.Read ( NAME_KEY, DEFAULT_NAME ) )
{
    _rotation = info.Read ( ROTATION_KEY, GXQuat::IDENTITY );
    _scale = info.Read ( SCALE_KEY, GXVec3::ZERO );
    _location = info.Read ( LOCATION_KEY, GXVec3::ZERO );
    SaveState::Container const &components = info.ReadArray ( COMPONENTS_KEY );

    for ( size_t i = 0U, count = components.GetArraySize (); i < count; ++i )
    {
        if ( auto component = Component::Spawn ( messageQueue, components.ReadContainer () ); component ) [[likely]]
        {
            Append ( std::move ( *component ) );
        }
    }
}

Actor::~Actor () noexcept
{
    while ( !_components.empty () )
    {
        ComponentRef &ref = _components.back ();
        ref->Unregister ();
        _components.pop_back ();
    }
}

void Actor::SetName ( std::string_view name ) noexcept
{
    _name = name;
}

void Actor::Append ( ComponentRef &&component ) noexcept
{
    Component &c = *component;
    _components.push_back ( std::move ( component ) );
    c.Register ();
}

void Actor::Insert ( size_t before, ComponentRef &&component ) noexcept
{
    _components.insert ( _components.cbegin () + static_cast<Components::const_iterator::difference_type> ( before ),
        std::move ( component )
    );
}

ComponentRef Actor::Remove ( Component const &component ) noexcept
{
    auto end = _components.end ();

    auto findResult = std::find_if ( _components.begin (),
        _components.end (),

        [ target = &component ] ( ComponentRef const &item ) noexcept -> bool {
            return item.get () == target;
        }
    );

    AV_ASSERT ( findResult != end )
    ComponentRef result = std::move ( *findResult );
    result->Unregister ();
    _components.erase ( findResult );
    return result;
}

void Actor::Save ( SaveState::Container &root ) const noexcept
{
    root.Write ( VERSION_KEY, VERSION );
    root.Write ( ROTATION_KEY, _rotation );
    root.Write ( SCALE_KEY, _scale );
    root.Write ( LOCATION_KEY, _location );

    SaveState::Container &components = root.WriteArray ( COMPONENTS_KEY );

    for ( auto const &component : _components )
    {
        component->Save ( components.WriteContainer () );
    }
}

} // namespace editor
