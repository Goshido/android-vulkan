#include <precompiled_headers.hpp>
#include <static_mesh_component.hpp>


namespace editor {

namespace {

constexpr std::string_view VERSION_KEY = "version";
constexpr uint32_t VERSION = 1U;

} // end of anonymous namespace

//----------------------------------------------------------------------------------------------------------------------

StaticMeshComponent::StaticMeshComponent () noexcept:
    Component ( "static mesh" )
{
    // NOTHING
}

StaticMeshComponent::StaticMeshComponent ( SaveState::Container const &info ) noexcept:
    Component ( info )
{
    // FUCK
}

void StaticMeshComponent::Save ( SaveState::Container &root ) const noexcept
{
    Component::Save ( root );
    root.Write ( VERSION_KEY, VERSION );
    // FUCK
}

} // namespace editor
