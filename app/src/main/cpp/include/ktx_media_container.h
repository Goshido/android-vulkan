#ifndef KTX_MEDIA_CONTAINER_H
#define KTX_MEDIA_CONTAINER_H


#include "file.h"
#include "ktx_header.h"
#include "mip_info.h"


namespace android_vulkan {

constexpr size_t const MAX_MIPS = 20U;

// Note the class object instance is not reusable by design.
// The KTXv1 format is described here: https://www.khronos.org/opengles/sdk/tools/KTX/file_format_spec
class KTXMediaContainer final
{
    private:
        // Function pointer type declaration.
        using ReadHander = uint32_t ( * ) ( uint32_t value );

    private:
        std::unique_ptr<File>       _file;
        VkFormat                    _format;
        uint8_t                     _mipCount;
        MipInfo                     _mips[ MAX_MIPS ];
        VkDeviceSize                _totalSize;

    public:
        KTXMediaContainer () noexcept;

        KTXMediaContainer ( KTXMediaContainer const & ) = delete;
        KTXMediaContainer& operator = ( KTXMediaContainer const & ) = delete;

        KTXMediaContainer ( KTXMediaContainer && ) = delete;
        KTXMediaContainer& operator = ( KTXMediaContainer && ) = delete;

        ~KTXMediaContainer () = default;

        [[nodiscard]] VkFormat GetFormat () const;
        [[nodiscard]] uint8_t GetMipCount () const;

        // Note the "mip" with index zero has the maximum resolution.
        [[nodiscard]]  MipInfo const& GetMip ( uint8_t mip ) const;

        [[nodiscard]] VkDeviceSize GetTotalSize () const;

        // The method family returns true if success, otherwise the method family returns false.
        [[nodiscard]] bool Init ( char const* fileName );
        [[nodiscard]] bool Init ( std::string const &fileName );

    private:
        void ExtractMips ( uint8_t const* rawData, size_t size, KTXHeader const &header, ReadHander reader );

        [[nodiscard]] static bool CheckField ( uint32_t field,
            char const* name,
            uint32_t expected,
            ReadHander reader,
            char const* fileName
        );

        [[nodiscard]] static bool CheckSignature ( KTXHeader const &header, char const* fileName );
        [[nodiscard]] static bool CheckSize ( size_t size, char const* fileName );

        [[nodiscard]] static uint8_t const* GetMipmapData ( uint8_t const* rawData,
            KTXHeader const &header,
            ReadHander reader
        );

        // Endianness help functions.
        [[nodiscard]] static uint32_t ReadConvert ( uint32_t value );
        [[nodiscard]] static uint32_t ReadNative ( uint32_t value );

        [[nodiscard]] static bool ResolveFormat ( uint32_t glInternalFormat,
            VkFormat &format,
            ReadHander reader,
            char const* fileName
        );

        [[nodiscard]] static bool ResolveReader ( ReadHander &reader, uint32_t endianness, char const* fileName );
};

} // namespace android_vulkan


#endif // KTX_MEDIA_CONTAINER_H
