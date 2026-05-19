#include <precompiled_headers.hpp>
#include <file.hpp>
#include <mesh_storage.hpp>
#include <message_queue.hpp>
#include <native_renderer.hpp>
#include <scope_quard.hpp>
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

        std::string path = android_vulkan::File::ResolvePath ( std::move ( asset ) );

        if ( auto findResult = _storage.find ( path ); findResult != _storage.end () )
        {
            Item &item = findResult->second;
            ++item._references;
            result ( std::optional<MeshGeometryRef> { item._mesh } );
            return nullptr;
        }

        if ( auto findResult = _tasks.find ( path ); findResult != _tasks.cend () )
        {
            findResult->second.push_back ( std::move ( result ) );
            return nullptr;
        }

        MeshGeometryRef mesh = std::make_shared<android_vulkan::MeshGeometry> ();
        auto loadResult = mesh->LoadMesh ( NativeRenderer::Instance (), path );

        if ( !loadResult ) [[unlikely]]
        {
            result ( std::nullopt );
            return nullptr;
        }

        auto uploadResult = [ this, &messageQueue, path ] ( std::optional<MeshGeometryRef> &&mesh ) noexcept {
            AV_TRACE ( "Upload done %s", path.c_str () )

            auto finishUpload = [ this, path = std::move ( path ), mesh = std::move ( mesh ) ] () noexcept -> void* {
                AV_TRACE ( "Upload done %s", path.c_str () )
                auto const findResult = _tasks.find ( path );

                android_vulkan::ScopeGuard const freeTask (
                    [ this, findResult ] () noexcept {
                        _tasks.erase ( findResult );
                    }
                );

                std::deque<MeshLoadResult> const &results = findResult->second;

                for ( auto &result : results )
                    result ( std::optional<MeshGeometryRef> ( mesh ) );

                if ( !mesh )
                    return nullptr;

                _storage.insert (
                    std::pair ( std::move ( path ),
                        Item
                        {
                            ._mesh = std::move ( *mesh ),
                            ._references = results.size ()
                        }
                    )
                );

                return nullptr;
            };

            messageQueue.EnqueueBack (
                {
                    ._type = eMessageType::InvokeIO,
                    ._action = std::move ( finishUpload ),
                    ._serialNumber = 0U
                }
            );
        };

        messageQueue.EnqueueBack (
            {
                ._type = eMessageType::UploadMesh,

                ._action = [
                    info = MeshUploadInfo ( std::move ( mesh ), std::move ( *loadResult ), std::move ( uploadResult ) )
                ] () mutable noexcept -> void* {
                    return &info;
                },

                ._serialNumber = 0U
            }
        );

        _tasks.insert ( std::pair ( std::move ( path ), std::deque<MeshLoadResult> ( { std::move ( result ) } ) ) );
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
    MessageQueue &messageQueue = MessageQueue::Instance ();

    auto unload = [ this, &messageQueue, mesh = std::move ( mesh ) ] noexcept -> void* {
        AV_TRACE ( "Unload %s", mesh->GetName ().c_str () )
        auto findResult = _storage.find ( mesh->GetName () );

        if ( --findResult->second._references > 0U )
            return nullptr;

        messageQueue.EnqueueBack (
            {
                ._type = eMessageType::DestroyMesh,

                ._action = [ mesh = std::move ( mesh ) ] () mutable noexcept -> void* {
                    return &mesh;
                },

                ._serialNumber = 0U
            }
        );

        _storage.erase ( findResult );
        return nullptr;
    };

    messageQueue.EnqueueBack (
        {
            ._type = eMessageType::InvokeIO,
            ._action = std::move ( unload ),
            ._serialNumber = 0U
        }
    );
}

MeshStorage &MeshStorage::Instance () noexcept
{
    return *_instance;
}

} // namespace editor
