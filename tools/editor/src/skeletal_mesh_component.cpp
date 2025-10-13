#include <precompiled_headers.hpp>
#include <av_assert.hpp>
#include <skeletal_mesh_component.hpp>


namespace editor {

namespace {

constexpr uint32_t VERSION = 1U;

} // end of anonymous namespace

//----------------------------------------------------------------------------------------------------------------------

SkeletalMeshComponent::SkeletalMeshComponent () noexcept:
    Component ( VERSION, "skeletal mesh" )
{
    // NOTHING
}

SkeletalMeshComponent::SkeletalMeshComponent ( SaveState::Container const &info ) noexcept:
    Component ( info )
{
    AV_ASSERT ( _version == VERSION )
    // FUCK
}

void SkeletalMeshComponent::Save ( SaveState::Container &root ) const noexcept
{
    Component::Save ( root );
    root.Write ( TYPE_KEY, TYPE );
    // FUCK
}

} // namespace editor
