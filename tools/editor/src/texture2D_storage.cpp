#include <precompiled_headers.hpp>
#include <message_queue.hpp>
#include <native_renderer.hpp>
#include <scope_quard.hpp>
#include <texture2D_storage.hpp>
#include <trace.hpp>


namespace editor {

Texture2DStorage* Texture2DStorage::_instance = nullptr;

Texture2DStorage::Texture2DStorage () noexcept
{
    _instance = this;
}

void Texture2DStorage::Load ( std::string_view asset, Texture2DLoadResult &&result ) noexcept
{
    // The main goal: minimize the amount of work in rendering thread as much as possible. To achieve that every IO
    // task will be done in separate thread. Any Vulkan resource allocations will be done is separate thread.
    // The rendering thread will only record transfer and mipmap generation operations into command buffer.

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
            result ( std::optional<Texture2DRef> { item._texture } );
            return nullptr;
        }

        if ( auto findResult = _tasks.find ( path ); findResult != _tasks.cend () )
        {
            findResult->second.push_back ( std::move ( result ) );
            return nullptr;
        }

        Texture2DRef texture = std::make_shared<android_vulkan::Texture2D> ();

        bool const status = texture->UploadToStagingBuffer ( NativeRenderer::Instance (),
            path,
            android_vulkan::eColorSpace::Unorm,
            true
        );

        if ( !status ) [[unlikely]]
        {
            result ( std::nullopt );
            return nullptr;
        }

        auto uploadResult = [ this, &messageQueue, path ] ( std::optional<Texture2DRef> &&texture ) mutable noexcept {
            AV_TRACE ( "Upload done %s", path.c_str () )

            auto finishUpload = [ this,
                path = std::move ( path ),
                texture = std::move ( texture )
            ] () mutable noexcept -> void* {
                AV_TRACE ( "Upload done %s", path.c_str () )
                auto const findResult = _tasks.find ( path );

                android_vulkan::ScopeGuard const freeTask (
                    [ this, findResult ] () noexcept {
                        _tasks.erase ( findResult );
                    }
                );

                std::deque<Texture2DLoadResult> const &results = findResult->second;

                for ( auto &result : results )
                    result ( std::optional<Texture2DRef> ( texture ) );

                if ( !texture ) [[unlikely]]
                    return nullptr;

                _storage.insert (
                    std::pair ( std::move ( path ),
                        Item
                        {
                            ._texture = std::move ( *texture ),
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
                ._type = eMessageType::UploadTexture2D,

                ._action = [
                    info = Texture2DUploadInfo ( std::move ( texture ), std::move ( uploadResult ) )
                ] () mutable noexcept -> void* {
                    return &info;
                },

                ._serialNumber = 0U
            }
        );

        _tasks.insert (
            std::pair ( std::move ( path ),
                std::deque<Texture2DLoadResult> ( { std::move ( result ) } )
            )
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

void Texture2DStorage::Unload ( Texture2DRef &&texture ) noexcept
{
    MessageQueue &messageQueue = MessageQueue::Instance ();

    auto unload = [ this, &messageQueue, texture = std::move ( texture ) ] noexcept -> void* {
        AV_TRACE ( "Unload %s", texture->GetName ().c_str () )
        auto findResult = _storage.find ( texture->GetName () );

        if ( --findResult->second._references > 0U )
            return nullptr;

        messageQueue.EnqueueBack (
            {
                ._type = eMessageType::DestroyTexture2D,

                ._action = [ texture = std::move ( texture ) ] () mutable noexcept -> void* {
                    return &texture;
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

Texture2DStorage &Texture2DStorage::Instance () noexcept
{
    return *_instance;
}

} // namespace editor
