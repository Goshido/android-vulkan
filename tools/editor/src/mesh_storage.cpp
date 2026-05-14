#include <precompiled_headers.hpp>
#include <file.hpp>
#include <mesh_storage.hpp>
#include <trace.hpp>


namespace editor {

MeshStorage* MeshStorage::_instance = nullptr;

MeshStorage::MeshStorage ( android_vulkan::Renderer &renderer ) noexcept:
    _renderer ( renderer )
{
    // NOTHING
}

void MeshStorage::Load ( std::string_view asset, LoadResult &&result ) noexcept
{
    auto loadAsset = [ asset = std::string ( asset ), result = std::move ( result ) ] () noexcept -> void* {
        AV_TRACE ( "Loading %s", asset.c_str () )
        android_vulkan::File file ( asset );

        if ( !file.LoadContent () ) [[unlikely]]
        {
            result ( std::nullopt );
            return nullptr;
        }

        // TODO
        return nullptr;
    };

    MessageQueue::Instance ().EnqueueBack (
        {
            ._type = eMessageType::InvokeIO,
            ._action = std::move ( loadAsset ),
            ._serialNumber = 0U
        }
    );
}

void MeshStorage::CollectGarbage () noexcept
{
    // FUCK
}

MeshStorage &MeshStorage::Instance () noexcept
{
    return *_instance;
}

} // namespace editor
