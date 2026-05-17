#include <precompiled_headers.hpp>
#include <mesh_storage.hpp>
#include <message_queue.hpp>
#include <native_renderer.hpp>
#include <trace.hpp>


namespace editor {

MeshStorage* MeshStorage::_instance = nullptr;

MeshStorage::MeshStorage () noexcept
{
    _instance = this;
}

void MeshStorage::Load ( std::string_view asset, MeshLoadResult &&result ) noexcept
{
    MessageQueue &messageQueue = MessageQueue::Instance ();

    auto loadAsset = [ this,
        &messageQueue,
        asset = std::string ( asset ),
        result = std::move ( result )
    ] () mutable noexcept -> void* {
        AV_TRACE ( "Loading %s", asset.c_str () )

        MeshGeometryRef mesh = std::make_shared<android_vulkan::MeshGeometry> ();
        auto loadResult = mesh->LoadMesh ( NativeRenderer::Instance (), asset );

        if ( !loadResult ) [[unlikely]]
        {
            result ( std::nullopt );
            return nullptr;
        }

        // FUCK - incorrect, make proper owning chain
        _storage.insert ( std::make_pair ( mesh->GetName (), mesh ) );

        messageQueue.EnqueueBack (
            {
                ._type = eMessageType::UploadMesh,

                ._action = [
                    info = MeshUploadInfo ( std::move ( mesh ), std::move ( *loadResult ), std::move ( result ) )
                ] () mutable noexcept -> void* {
                    return &info;
                },

                ._serialNumber = 0U
            }
        );

        return nullptr;
    };

    messageQueue.EnqueueBack (
        {
            ._type = eMessageType::InvokeIO,
            ._action = std::move ( loadAsset ),
            ._serialNumber = 0U
        }
    );
}

void MeshStorage::Unload ( MeshGeometryRef &&mesh ) noexcept
{
    // FUCK make proper owning chain
    _storage.erase ( mesh->GetName () );

    MessageQueue::Instance ().EnqueueBack (
        {
            ._type = eMessageType::DestroyMesh,

            ._action = [ mesh = std::move ( mesh ) ] () mutable noexcept -> void* {
                return &mesh;
            },

            ._serialNumber = 0U
        }
    );
}

MeshStorage &MeshStorage::Instance () noexcept
{
    return *_instance;
}

} // namespace editor
