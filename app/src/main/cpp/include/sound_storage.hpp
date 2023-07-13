#ifndef ANDROID_VULKAN_SOUND_STORAGE_HPP
#define ANDROID_VULKAN_SOUND_STORAGE_HPP


#include <GXCommon/GXWarning.hpp>

GX_DISABLE_COMMON_WARNINGS

#include <optional>
#include <mutex>
#include <unordered_map>
#include <vector>

GX_RESTORE_WARNING_STATE


namespace android_vulkan {

class SoundStorage final
{
    public:
        using SoundFile = std::shared_ptr<std::vector<uint8_t> const>;

    private:
        std::mutex                                      _mutex {};
        std::unordered_map<std::string, SoundFile>      _storage {};

    public:
        SoundStorage () = default;

        SoundStorage ( SoundStorage const & ) = delete;
        SoundStorage &operator = ( SoundStorage const & ) = delete;

        SoundStorage ( SoundStorage && ) = delete;
        SoundStorage &operator = ( SoundStorage && ) = delete;

        ~SoundStorage () = default;

        [[nodiscard]] std::optional<SoundFile> GetFile ( std::string &&file ) noexcept;

        // Method removes elements from storage which are not owned at this moment.
        void Trim () noexcept;
};

} // namespace android_vulkan


#endif // ANDROID_VULKAN_SOUND_STORAGE_HPP
