#include <precompiled_headers.hpp>
#include <av_assert.hpp>
#include <mesh_storage.hpp>
#include <static_mesh_component.hpp>


namespace editor {

namespace {

constexpr uint32_t VERSION = 1U;
constexpr std::string_view DEFAULT_MESH = "pbr/system/unit-cube.mesh2";
constexpr std::string_view DEFAULT_NAME = "static mesh";

constexpr std::string_view MESH_KEY = "mesh";

} // end of anonymous namespace

//----------------------------------------------------------------------------------------------------------------------

StaticMeshComponent::StaticMeshComponent () noexcept:
    Component ( VERSION, std::string ( DEFAULT_NAME ) )
{
    LoadMesh ( DEFAULT_MESH );
}

StaticMeshComponent::StaticMeshComponent ( SaveState::Container const &info ) noexcept:
    Component ( info )
{
    AV_ASSERT ( _version == VERSION )
    LoadMesh ( info.Read ( MESH_KEY, DEFAULT_MESH ) );
}

StaticMeshComponent::StaticMeshComponent ( std::string_view mesh ) noexcept:
    Component ( VERSION, std::string ( DEFAULT_NAME ) )
{
    LoadMesh ( mesh );
}

void StaticMeshComponent::Register () noexcept
{
    // FUCK
}

void StaticMeshComponent::Unregister () noexcept
{
    // FUCK
}

void StaticMeshComponent::Save ( SaveState::Container &root ) const noexcept
{
    Component::Save ( root );
    root.Write ( TYPE_KEY, TYPE );
    root.Write ( MESH_KEY, _mesh->GetName () );
}

void StaticMeshComponent::LoadMesh ( std::string_view mesh ) noexcept
{
    auto result = [ this ] ( std::optional<MeshGeometryRef> &&mesh ) noexcept {
        _mesh = *mesh;
    };

    MeshStorage::Instance ().Load ( mesh, std::move ( result ) );
}

} // namespace editor
