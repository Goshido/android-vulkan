#include <precompiled_headers.hpp>
#include <skeletal_mesh_component.hpp>


namespace editor {

namespace {

constexpr std::string_view VERSION_KEY = "version";
constexpr uint32_t VERSION = 1U;

} // end of anonymous namespace

//----------------------------------------------------------------------------------------------------------------------

Component::Factory<SkeletalMeshComponent> const SkeletalMeshComponent::_factory ( "SkeletalMeshComponent" );

SkeletalMeshComponent::SkeletalMeshComponent () noexcept:
    Component ( "skeletal mesh" )
{
    // NOTHING
}

SkeletalMeshComponent::SkeletalMeshComponent ( SaveState::Container const &info ) noexcept:
    Component ( info )
{
    // FUCK
}

void SkeletalMeshComponent::Save ( SaveState::Container &root ) const noexcept
{
    Component::Save ( root );
    root.Write ( VERSION_KEY, VERSION );
    // FUCK
}

} // namespace editor
