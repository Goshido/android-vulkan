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
        explicit File ( std::string &&filePath );

        File ( File &other ) = delete;
        File& operator = ( const File &other ) = delete;

        ~File () = default;

        const std::vector<uint8_t>& GetContent () const;
        bool IsContentLoaded () const;
        bool LoadContent ();
};

} // namespace android_vulkan


#endif // ANDROID_VULKAN_FILE_H
