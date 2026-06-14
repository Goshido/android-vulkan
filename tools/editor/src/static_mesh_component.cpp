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

StaticMeshComponent::StaticMeshComponent ( std::string_view mesh, std::string_view emission ) noexcept:
    Component ( VERSION, std::string ( DEFAULT_NAME ) )
{
    LoadResources ( mesh, emission );
}

StaticMeshComponent::~StaticMeshComponent () noexcept
{
    if ( _mesh ) [[likely]]
        MeshStorage::Instance ().Unload ( std::move ( _mesh ) );

    Texture2DStorage &storage = Texture2DStorage::Instance ();

    if ( _material._albedo )
        storage.Unload ( std::move ( _material._albedo ) );

    if ( _material._emission )
        storage.Unload ( std::move ( _material._emission ) );

    if ( _material._mask )
        storage.Unload ( std::move ( _material._mask ) );

    if ( _material._normal )
        storage.Unload ( std::move ( _material._normal ) );

    if ( _material._param )
    {
        storage.Unload ( std::move ( _material._param ) );
    }
}

void StaticMeshComponent::Register ( Actor &actor ) noexcept
{
    Component::Register ( actor );
    JoinRendering ();
}

void StaticMeshComponent::Unregister () noexcept
{
    QuitRendering ();
    Component::Unregister ();
}

void StaticMeshComponent::ActorTransformChanged () noexcept
{
    UpdateTransform ();
}

void StaticMeshComponent::Save ( SaveState::Container &root ) const noexcept
{
    Component::Save ( root );
    root.Write ( TYPE_KEY, TYPE );
    root.Write ( MESH_KEY, _mesh->GetName () );
}

void StaticMeshComponent::LoadResources ( std::string_view mesh, std::string_view emission ) noexcept
{
    MeshStorage::Instance ().Load ( mesh,
        [ this ] ( std::optional<MeshGeometryRef> &&mesh ) noexcept {
            _mesh = std::move ( *mesh );
            JoinRendering ();
        }
    );

    Texture2DStorage::Instance ().Load ( emission,
        [ this ] ( std::optional<Texture2DRef> &&texture ) noexcept {
            _material._emission = std::move ( *texture );
            JoinRendering ();
        }
    );
}

void StaticMeshComponent::JoinRendering () noexcept
{
    if ( !_actor || !_mesh ) [[unlikely]]
        return;

    if ( !_node.IsConnected () ) [[unlikely]]
        _node = Workspace::Instance ().RegisterOpaqueMesh ( _mesh );

    GXColorUNORM const c ( 255U, 255U, 255U, 255 );
    _node.SetColor ( c, c, c, c, 1.0F );
    _node.SetMaterial ( _material );
    _node.SetID ( _actor );

    UpdateTransform ();
}

void StaticMeshComponent::QuitRendering () noexcept
{
    if ( _node.IsConnected () ) [[likely]]
    {
        Workspace::Instance ().Unregister ( _node );
        _node = {};
    }
}

void StaticMeshComponent::UpdateTransform () noexcept
{
    if ( !_node.IsConnected () ) [[unlikely]]
        return;

    GXMat4 parent {};
    _actor->GetTransform ( parent );

    GXMat4 local {};
    local.FromFast ( _rotation, _location );
    auto &x = *reinterpret_cast<GXVec3*> ( local._data[ 0U ] );
    GXVec3 const &s = _scale;

    auto &y = *reinterpret_cast<GXVec3*> ( local._data[ 1U ] );
    x.Multiply ( x, s._data[ 0U ] );

    auto &z = *reinterpret_cast<GXVec3*> ( local._data[ 2U ] );
    y.Multiply ( y, s._data[ 1U ] );
    z.Multiply ( z, s._data[ 2U ] );

    GXMat4 transform {};
    transform.Multiply ( local, parent );

    _node.SetLocal ( transform );
    _node.SetBounds ( _mesh->GetBounds () );
}

} // namespace editor
