#include <file.h>
#include <sound_storage.h>


namespace android_vulkan {

std::optional<SoundStorage::SoundFile> SoundStorage::GetFile ( std::string &&file ) noexcept
{
    std::unique_lock<std::mutex> const lock ( _mutex );

    if ( auto findResult = _storage.find ( file ); findResult != _storage.cend () )
        return findResult->second;

    File asset ( file );

    if ( !asset.LoadContent () )
        return std::nullopt;

    auto const [iterator, success] = _storage.emplace ( std::move ( file ),
        std::make_shared<std::vector<uint8_t> const> ( std::move ( asset.GetContent () ) )
    );

    return iterator->second;
}

void SoundStorage::Trim () noexcept
{
    std::unique_lock<std::mutex> const lock ( _mutex );

    for ( auto i = _storage.cbegin (); i != _storage.cend (); )
    {
        auto const& soundFile = i->second;

        if ( soundFile.use_count () > 1 )
        {
            ++i;
            continue;
        }

        i = _storage.erase ( i );
    }
}

} // namespace android_vulkan
