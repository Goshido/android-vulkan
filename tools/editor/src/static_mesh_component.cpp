#include <precompiled_headers.hpp>
#include <av_assert.hpp>
#include <mesh_storage.hpp>
#include <static_mesh_component.hpp>
#include <texture2D_storage.hpp>
#include <workspace.hpp>


namespace editor {

namespace {

constexpr uint32_t VERSION = 1U;
constexpr std::string_view DEFAULT_MESH = "pbr/system/unit-cube.mesh2";
constexpr std::string_view DEFAULT_ALBEDO = "pbr/system/white.tga";
constexpr std::string_view DEFAULT_NAME = "static mesh";

constexpr std::string_view MESH_KEY = "mesh";

} // end of anonymous namespace

//----------------------------------------------------------------------------------------------------------------------

StaticMeshComponent::StaticMeshComponent () noexcept:
    Component ( VERSION, std::string ( DEFAULT_NAME ) )
{
    LoadResources ( DEFAULT_MESH, DEFAULT_ALBEDO );
}

StaticMeshComponent::StaticMeshComponent ( SaveState::Container const &info ) noexcept:
    Component ( info )
{
    AV_ASSERT ( _version == VERSION )
    LoadResources ( info.Read ( MESH_KEY, DEFAULT_MESH ), DEFAULT_ALBEDO );
}

StaticMeshComponent::StaticMeshComponent ( std::string_view mesh, std::string_view albedo ) noexcept:
    Component ( VERSION, std::string ( DEFAULT_NAME ) )
{
    LoadResources ( mesh, albedo );
}

StaticMeshComponent::~StaticMeshComponent () noexcept
{
    if ( _mesh ) [[likely]]
        MeshStorage::Instance ().Unload ( std::move ( _mesh ) );

    if ( _material._albedo )
        Texture2DStorage::Instance ().Unload ( std::move ( _material._albedo ) );

    if ( _material._emission )
        Texture2DStorage::Instance ().Unload ( std::move ( _material._emission ) );

    if ( _material._mask )
        Texture2DStorage::Instance ().Unload ( std::move ( _material._mask ) );

    if ( _material._normal )
        Texture2DStorage::Instance ().Unload ( std::move ( _material._normal ) );

    if ( _material._param )
    {
        Texture2DStorage::Instance ().Unload ( std::move ( _material._param ) );
    }
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

void StaticMeshComponent::LoadResources ( std::string_view mesh, std::string_view albedo ) noexcept
{
    MeshStorage::Instance ().Load ( mesh,
        [ this ] ( std::optional<MeshGeometryRef> &&mesh ) noexcept {
            _mesh = std::move ( *mesh );
        }
    );

    Texture2DStorage::Instance ().Load ( albedo,
        [ this ] ( std::optional<Texture2DRef> &&texture ) noexcept {
            _material._emission = std::move ( *texture );

            // FUCK - make it right
            _node = Workspace::Instance ().RegisterOpaqueMesh ( _mesh );
            _node.SetLocal ( GXQuat::IDENTITY, GXVec3 ( 0.0F, -1.0F, 3.0F ), GXVec3::ONE );
            _node.SetBounds ( _mesh->GetBounds () );

            GXColorUNORM const c ( 255U, 255U, 255U, 255 );
            _node.SetColor ( c, c, c, c, 1.0F );
            _node.SetMaterial ( _material );
        }
    );
}

} // namespace editor
