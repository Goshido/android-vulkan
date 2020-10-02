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

        // note "const std::string &" version is not implemented to distinguish rvalue|lvalue reference.
        explicit File ( std::string &filePath );
        explicit File ( std::string_view const &filePath );
        [[maybe_unused]] explicit File ( std::string &&filePath );

        File ( File &other ) = delete;
        File& operator = ( const File &other ) = delete;

        ~File () = default;

        [[nodiscard]] std::vector<uint8_t>& GetContent ();
        [[maybe_unused]] [[nodiscard]] std::vector<uint8_t> const& GetContent () const;

        [[nodiscard]] bool IsContentLoaded () const;
        [[nodiscard]] bool LoadContent ();
};

} // namespace android_vulkan


#endif // ANDROID_VULKAN_FILE_H
