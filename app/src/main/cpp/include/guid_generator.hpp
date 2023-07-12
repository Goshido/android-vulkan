#ifndef ANDROID_VULKAN_GUID_GENERATOR_HPP
#define ANDROID_VULKAN_GUID_GENERATOR_HPP


#include <GXCommon/GXWarning.h>

GX_DISABLE_COMMON_WARNINGS

#include <string_view>

GX_RESTORE_WARNING_STATE


namespace android_vulkan {

// Android NDK v24.0.8215888 does not contain <uuid.h> header file unfortunately.
// The implementation is based on AOSP source code of the UUID library.
// https://android.googlesource.com/platform/external/e2fsprogs/+/master/lib/uuid
// Tree: 5ff5e934cfae30b5cf1d1a4b5ef259d05b5fc25b
// Date: 2022/04/09 09:45 +3.00 UTC
class GUID final
{
    private:

#pragma pack ( push, 1 )

        struct UUID final
        {
            uint8_t     _data[ 16U ];
        };

        struct TimeUUID final
        {
            uint32_t    _timeLow;
            uint16_t    _timeMid;
            uint16_t    _timeHiAndVersion;
            uint16_t    _clockSeq;
            uint8_t     _node[ 6U ];
        };

#pragma pack ( pop )

    public:
        GUID () = delete;

        GUID ( GUID const & ) = delete;
        GUID &operator = ( GUID const & ) = delete;

        GUID ( GUID && ) = delete;
        GUID &operator = ( GUID && ) = delete;

        ~GUID () = delete;

        // Method returns result in format:
        //      prefix@xxxxxxxx-xxxx-xxxx-xxxx-xxxxxxxxxxxx
        [[nodiscard]] static std::string GenerateAsString ( std::string_view prefix ) noexcept;

    private:
        [[nodiscard]] static UUID GenerateUUID () noexcept;
        [[nodiscard]] static UUID Pack ( TimeUUID const &timeUUID ) noexcept;
        [[nodiscard]] static TimeUUID Unpack ( UUID const &uuid ) noexcept;
};

} // namespace android_vulkan


#endif // ANDROID_VULKAN_GUID_GENERATOR_HPP
