#ifndef KTX_MEDIA_CONTAINER_HPP
#define KTX_MEDIA_CONTAINER_HPP


#include "file.hpp"
#include "ktx_header.hpp"
#include "mip_info.hpp"

GX_DISABLE_COMMON_WARNINGS

#include <memory>

GX_RESTORE_WARNING_STATE


namespace android_vulkan {

constexpr size_t const MAX_MIPS = 20U;

// Note color space is defined in ktx container itself and can be fully trusted.
// The KTXv1 format is described here: https://www.khronos.org/opengles/sdk/tools/KTX/file_format_spec
class KTXMediaContainer final
{
    private:
        // Function pointer type declaration.
        using ReadHander = uint32_t ( * ) ( uint32_t value );

    private:
        std::unique_ptr<File>       _file = {};
        VkFormat                    _format = VK_FORMAT_UNDEFINED;
        uint8_t                     _mipCount = 0U;
        MipInfo                     _mips[ MAX_MIPS ] {};
        VkDeviceSize                _totalSize = 0U;

    public:
        explicit KTXMediaContainer () = default;

        KTXMediaContainer ( KTXMediaContainer const & ) = delete;
        KTXMediaContainer &operator = ( KTXMediaContainer const & ) = delete;

        KTXMediaContainer ( KTXMediaContainer && ) = delete;
        KTXMediaContainer &operator = ( KTXMediaContainer && ) = delete;

        ~KTXMediaContainer () = default;

        [[nodiscard]] VkFormat GetFormat () const noexcept;
        [[nodiscard]] uint8_t GetMipCount () const noexcept;

        // Note the "mip" with index zero has the maximum resolution.
        [[nodiscard]] MipInfo const &GetMip ( uint8_t mip ) const noexcept;

        [[nodiscard]] VkDeviceSize GetTotalSize () const noexcept;

        // The method family returns true if success, otherwise the method family returns false.
        [[nodiscard]] bool Init ( std::string_view fileName ) noexcept;

    private:
        void ExtractMips ( uint8_t const* rawData, size_t size, KTXHeader const &header, ReadHander reader ) noexcept;

        [[nodiscard]] bool CheckField ( uint32_t field,
            char const* name,
            uint32_t expected,
            ReadHander reader
        ) noexcept;

        [[nodiscard]] bool CheckSignature ( KTXHeader const &header ) noexcept;
        [[nodiscard]] bool CheckSize ( size_t size ) noexcept;

        [[nodiscard]] bool ResolveFormat ( uint32_t glInternalFormat, VkFormat &format, ReadHander reader ) noexcept;
        [[nodiscard]] bool ResolveReader ( ReadHander &reader, uint32_t endianness ) noexcept;

        [[nodiscard]] static uint8_t const* GetMipmapData ( uint8_t const* rawData,
            KTXHeader const &header,
            ReadHander reader
        ) noexcept;

        // Endianness help functions.
        [[nodiscard]] static uint32_t ReadConvert ( uint32_t value ) noexcept;
        [[nodiscard]] static uint32_t ReadNative ( uint32_t value ) noexcept;
};

} // namespace android_vulkan


#endif // KTX_MEDIA_CONTAINER_HPP
