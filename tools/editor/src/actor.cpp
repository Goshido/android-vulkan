#include <precompiled_headers.hpp>
#include <actor.hpp>
#include <av_assert.hpp>
#include <property_tool.hpp>


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

Actor::ComponentInfo::ComponentInfo ( ComponentInfo const &/*other*/ ) noexcept
{
    AV_ASSERT ( false )
}

Actor::ComponentInfo &Actor::ComponentInfo::operator = ( ComponentInfo const &/*other*/ ) noexcept
{
    AV_ASSERT ( false )
    return *this;
}

Actor::ComponentInfo::ComponentInfo ( Actor::ComponentInfo &&other ) noexcept:
    _component ( std::move ( other._component ) ),
    _index ( std::exchange ( other._index, 0UZ ) )
{
    // NOTHING
}

Actor::ComponentInfo &Actor::ComponentInfo::operator = ( Actor::ComponentInfo &&other ) noexcept
{
    if ( this == &other ) [[unlikely]]
        return *this;

    _component = std::move ( other._component );
    _index = std::exchange ( other._index, 0UZ );

    return *this;
}

Actor::ComponentInfo::ComponentInfo ( ComponentRef &&component, size_t index ) noexcept:
    _component ( std::move ( component ) ),
    _index ( index )
{
    // NOTHING
}

//----------------------------------------------------------------------------------------------------------------------

Actor::Actor ( SaveState::Container const &info ) noexcept:
    _name ( std::string ( info.Read ( NAME_KEY, DEFAULT_NAME ) ) )
{
    _rotation = info.Read ( ROTATION_KEY, GXQuat::IDENTITY );
    _scale = info.Read ( SCALE_KEY, GXVec3::ZERO );
    _location = info.Read ( LOCATION_KEY, GXVec3::ZERO );
    SaveState::Container const &components = info.ReadArray ( COMPONENTS_KEY );

    for ( size_t i = 0U, count = components.GetArraySize (); i < count; ++i )
    {
        if ( auto component = Component::Spawn ( components.ReadContainer () ); component ) [[likely]]
        {
            ComponentRef &ref = *component;
            Component &c = *ref;
            _components.push_back ( std::move ( ref ) );
            c.Register ( *this );
            c.ActorTransformChanged ();
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

void Actor::SetName ( std::string &&name ) noexcept
{
    _name = std::move ( name );
}

void Actor::Attach ( ComponentInfo &&component ) noexcept
{
    Component &c = *component._component;

    _components.insert (
        _components.cbegin () + static_cast<Components::const_iterator::difference_type> ( component._index ),
        std::move ( component._component )
    );

    c.Register ( *this );
    c.ActorTransformChanged ();
}

Actor::ComponentInfo Actor::Detach ( Component const &component ) noexcept
{
    size_t const limit = _components.size ();
    auto item = _components.begin ();

    for ( size_t i = 0UZ; i < limit; ++i )
    {
        ComponentRef &ref = *item;

        if ( ref.get () != &component )
        {
            ++item;
            continue;
        }

        ref->Unregister ();
        ComponentInfo info ( std::move ( ref ), i );
        _components.erase ( item );
        return info;
    }

    AV_ASSERT ( false )
    return ComponentInfo {};
}

void Actor::Select () noexcept
{
    _select = true;

    for ( auto &component : _components )
    {
        component->Select ();
    }
}

void Actor::Deselect () noexcept
{
    _select = false;

    for ( auto &component : _components )
    {
        component->Deselect ();
    }
}

bool Actor::IsSelected () const noexcept
{
    return _select;
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

void Actor::ShowProperties ( PropertyTool &propertyTool ) noexcept
{
    propertyTool.Append ( "Name", _name );

    if ( _components.empty () ) [[unlikely]]
        return;

    for ( auto &component : _components )
    {
        propertyTool.AppendSeparator ();
        component->ShowProperties ( propertyTool );
    }
}

Actor::Components &Actor::GetComponents () noexcept
{
    return _components;
}

GXQuat const &Actor::GetRotation () const noexcept
{
    return _rotation;
}

void Actor::SetRotation ( GXQuat const &rotation ) noexcept
{
    _rotation = rotation;
    NotifyTransformChanged ();
}

void Actor::SetRotation ( GXMat3 const &rotation ) noexcept
{
    _rotation = GXQuat ( rotation );
    NotifyTransformChanged ();
}

void Actor::SetRotation ( GXMat4 const &rotation ) noexcept
{
    _rotation = GXQuat ( rotation );
    NotifyTransformChanged ();
}

GXVec3 const &Actor::GetLocation () const noexcept
{
    return _location;
}

void Actor::SetLocation ( GXVec3 const &location ) noexcept
{
    _location = location;
    NotifyTransformChanged ();
}

GXVec3 const &Actor::GetScale () const noexcept
{
    return _scale;
}

void Actor::SetScale ( GXVec3 const &scale ) noexcept
{
    _scale = scale;
    NotifyTransformChanged ();
}

void Actor::SetLocal ( GXQuat const &rotation, GXVec3 const &location ) noexcept
{
    _rotation = rotation;
    _location = location;
    NotifyTransformChanged ();
}

void Actor::SetLocal ( GXQuat const &rotation, GXVec3 const &location, GXVec3 const &scale ) noexcept
{
    _rotation = rotation;
    _location = location;
    _scale = scale;
    NotifyTransformChanged ();
}

void Actor::SetLocal ( GXVec3 const &location, GXVec3 const &scale ) noexcept
{
    _location = location;
    _scale = scale;
    NotifyTransformChanged ();
}

void Actor::NotifyTransformChanged () noexcept
{
    for ( auto &component : _components )
    {
        component->ActorTransformChanged ();
    }
}

} // namespace editor
