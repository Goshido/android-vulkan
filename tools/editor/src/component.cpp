#include <precompiled_headers.hpp>
#include <component.hpp>


namespace editor {

namespace {

constexpr std::string_view NAME_KEY = "name";
constexpr std::string_view DEFAULT_NAME = "component";

constexpr std::string_view TYPE_KEY = "type";
constexpr std::string_view PARENT_KEY = "parent";

} // end of anonymous namespace

//----------------------------------------------------------------------------------------------------------------------

Component::SpawnerRegistry Component::_spawnerRegistry = std::nullopt;

Component::Component ( std::string &&name ) noexcept:
    _name ( std::move ( name ) )
{
    // NOTHING
}

Component::Component ( SaveState::Container const &info ) noexcept
{
    _name = info.Read ( NAME_KEY, DEFAULT_NAME );
    _parent = info.Read ( PARENT_KEY, GXMat4::IDENTITY );
    _local.Identity ();
}

void Component::Save ( SaveState::Container &root ) const noexcept
{
    root.Write ( NAME_KEY, _name );
    root.Write ( PARENT_KEY, _parent );
}

std::optional<Component::Ref> Component::Spawn ( SaveState::Container const &info ) noexcept
{
    Spawners &spawners = *_spawnerRegistry;

    if ( auto const spawn = spawners.find ( info.Read ( TYPE_KEY, std::string_view {} ) ); spawn != spawners.cend () )
    {
        [[likely]]
        return spawn->second ( info );
    }

    return std::nullopt;
}

} // namespace editor
