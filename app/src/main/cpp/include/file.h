#ifndef ANDROID_VULKAN_FILE_H
#define ANDROID_VULKAN_FILE_H


#include <GXCommon/GXWarning.h>

GX_DISABLE_COMMON_WARNINGS

#include <string>
#include <vector>

GX_RESTORE_WARNING_STATE


namespace android_vulkan {

class File final
{
    private:
        std::string             _filePath;
        std::vector<uint8_t>    _content;

    public:
        File () = default;

        File ( File const &other ) = delete;
        File& operator = ( File const &other ) = delete;

        File ( File &&other ) = delete;
        File& operator = ( File &&other ) = delete;

        explicit File ( std::string &&filePath ) noexcept;
        explicit File ( std::string_view const &filePath ) noexcept;
        explicit File ( char const* filePath ) noexcept;

        ~File () = default;

        [[nodiscard]] std::vector<uint8_t>& GetContent () noexcept;
        [[maybe_unused, nodiscard]] std::vector<uint8_t> const& GetContent () const noexcept;

        [[maybe_unused, nodiscard]] std::string& GetPath () noexcept;
        [[maybe_unused, nodiscard]] std::string const& GetPath () const noexcept;

        [[nodiscard]] bool IsContentLoaded () const noexcept;
        [[nodiscard]] bool IsExist () const noexcept;
        [[nodiscard]] bool LoadContent () noexcept;
};

} // namespace android_vulkan


#endif // ANDROID_VULKAN_FILE_H
