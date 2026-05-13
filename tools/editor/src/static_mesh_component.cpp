#include <precompiled_headers.hpp>
#include <av_assert.hpp>
#include <file.hpp>
#include <static_mesh_component.hpp>
#include <trace.hpp>


namespace editor {

namespace {

constexpr uint32_t VERSION = 1U;
constexpr std::string_view DEFAULT_MESH = "pbr/system/unit-cube.mesh2";

constexpr std::string_view MESH_KEY = "mesh";

} // end of anonymous namespace

//----------------------------------------------------------------------------------------------------------------------

StaticMeshComponent::StaticMeshComponent ( MessageQueue &messageQueue ) noexcept:
    Component ( messageQueue, VERSION, "static mesh" )
{
    LoadMesh ( DEFAULT_MESH );
}

StaticMeshComponent::StaticMeshComponent ( MessageQueue &messageQueue, SaveState::Container const &info ) noexcept:
    Component ( messageQueue, info )
{
    AV_ASSERT ( _version == VERSION )
    LoadMesh ( info.Read ( MESH_KEY, DEFAULT_MESH ) );
}

StaticMeshComponent::StaticMeshComponent ( MessageQueue &messageQueue, std::string_view mesh ) noexcept:
    Component ( messageQueue, VERSION, "static mesh" )
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
    auto loadAsset = [ mesh = std::string ( mesh ) ] () noexcept -> void* {
        AV_TRACE ( "Loading %s", mesh.c_str () )
        android_vulkan::File file ( mesh );

        if ( !file.LoadContent () ) [[unlikely]]
            return nullptr;

        // TODO
        return nullptr;
    };

    _messageQueue.EnqueueBack (
        {
            ._type = eMessageType::InvokeIO,
            ._action = std::move ( loadAsset ),
            ._serialNumber = 0U
        }
    );
}

} // namespace editor
