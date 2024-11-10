#include <precompiled_headers.hpp>
#include <av_assert.hpp>
#include <ktx_media_container.hpp>
#include <logger.hpp>


namespace android_vulkan {

namespace {

constexpr uint8_t const SIGNATURE[ sizeof ( KTXHeader::_identifier ) ] =
{
    0xABU,
    'K', 'T', 'X', ' ', '1', '1',
    0xBBU,
    '\r', '\n',
    0x1AU,
    '\n'
};

constexpr uint32_t CONVERT_ENDIANNESS = 0x01020304U;
constexpr uint32_t NATIVE_ENDIANNESS = 0x04030201U;

constexpr uint32_t TARGET_GL_TYPE = 0U;
constexpr uint32_t TARGET_GL_TYPE_SIZE = 1U;
constexpr uint32_t TARGET_GL_FORMAT = 0U;
constexpr uint32_t TARGET_NUMBER_OF_ARRAY_ELEMENTS = 0U;
constexpr uint32_t TARGET_NUMBER_OF_FACES = 1U;
constexpr uint32_t TARGET_PIXEL_DEPTH = 0U;

// Taken from GLES3/gl32.h and GLES2/gl2ext.h. Less dependencies. Useful for Windows OS especially.
constexpr uint32_t AV_GL_COMPRESSED_RGBA_ASTC_6x6_KHR = 0x93B4U;
constexpr uint32_t AV_GL_COMPRESSED_SRGB8_ALPHA8_ASTC_6x6_KHR = 0x93D4U;

std::unordered_map<uint32_t, VkFormat> g_FormatMapper =
{
    { AV_GL_COMPRESSED_RGBA_ASTC_6x6_KHR, VK_FORMAT_ASTC_6x6_UNORM_BLOCK },
    { AV_GL_COMPRESSED_SRGB8_ALPHA8_ASTC_6x6_KHR, VK_FORMAT_ASTC_6x6_SRGB_BLOCK }
};

} // end of anonymous namespace

//----------------------------------------------------------------------------------------------------------------------

KTXMediaContainer::KTXMediaContainer () noexcept:
    _file {},
    _format ( VK_FORMAT_UNDEFINED ),
    _mipCount ( 0U ),
    _mips {},
    _totalSize ( 0U )
{
    // NOTHING
}

VkFormat KTXMediaContainer::GetFormat () const
{
    AV_ASSERT ( _format != VK_FORMAT_UNDEFINED )
    return _format;
}

uint8_t KTXMediaContainer::GetMipCount () const
{
    AV_ASSERT ( _format != VK_FORMAT_UNDEFINED )
    return _mipCount;
}

MipInfo const &KTXMediaContainer::GetMip ( uint8_t mip ) const
{
    AV_ASSERT ( _format != VK_FORMAT_UNDEFINED )
    return _mips[ mip ];
}

VkDeviceSize KTXMediaContainer::GetTotalSize () const
{
    return _totalSize;
}

bool KTXMediaContainer::Init ( char const* fileName )
{
    AV_ASSERT ( !_file )

    _file = std::make_unique<File> ( fileName );

    if ( !_file->LoadContent () )
        return false;

    std::vector<uint8_t> const &content = _file->GetContent ();
    size_t const size = content.size ();

    if ( !CheckSize ( size, fileName ) )
        return false;

    uint8_t const* rawData = content.data ();
    auto const &header = *reinterpret_cast<KTXHeader const*> ( rawData );

    if ( !CheckSignature ( header, fileName ) )
        return false;

    ReadHander reader = nullptr;

    if ( !ResolveReader ( reader, header._endianness, fileName ) )
        return false;

    if ( !CheckField ( header._glType, "_glType", TARGET_GL_TYPE, reader, fileName ) )
        return false;

    if ( !CheckField ( header._glTypeSize, "_glTypeSize", TARGET_GL_TYPE_SIZE, reader, fileName ) )
        return false;

    if ( !CheckField ( header._glFormat, "_glFormat", TARGET_GL_FORMAT, reader, fileName ) )
        return false;

    if ( !CheckField ( header._numberOfFaces, "_numberOfFaces", TARGET_NUMBER_OF_FACES, reader, fileName ) )
        return false;

    if ( !CheckField ( header._pixelDepth, "_pixelDepth", TARGET_PIXEL_DEPTH, reader, fileName ) )
        return false;

    bool const isFail = !CheckField ( header._numberOfArrayElements,
        "_numberOfArrayElements",
        TARGET_NUMBER_OF_ARRAY_ELEMENTS,
        reader,
        fileName
    );

    if ( isFail )
        return false;

    if ( !ResolveFormat ( header._glInternalFormat, _format, reader, fileName ) )
        return false;

    ExtractMips ( rawData, size, header, reader );
    return true;
}

bool KTXMediaContainer::Init ( std::string const &fileName )
{
    return Init ( fileName.c_str () );
}

void KTXMediaContainer::ExtractMips ( uint8_t const* rawData, size_t size, KTXHeader const &header, ReadHander reader )
{
    uint8_t const* view = GetMipmapData ( rawData, header, reader );
    auto const mips = static_cast<size_t const> ( reader ( header._numberOfMipmapLevels ) );

    AV_ASSERT ( mips <= MAX_MIPS )

    VkExtent2D resolution;
    resolution.width = reader ( header._pixelWidth );
    resolution.height = reader ( header._pixelHeight );

    _totalSize = 0U;

    for ( size_t i = 0U; i < mips; ++i )
    {
        uint32_t const mipSize = reader ( *reinterpret_cast<uint32_t const*> ( view ) );
        view += sizeof ( mipSize );

        MipInfo &mip = _mips[ i ];
        mip._size = static_cast<VkDeviceSize> ( mipSize );
        mip._data = view;
        mip._resolution = resolution;

        _totalSize += mip._size;

        resolution.width = std::max ( 1U, resolution.width / 2U );
        resolution.height = std::max ( 1U, resolution.height / 2U );

        // Obeying to the alignment constraint: Each mip block MUST be aligned by sizeof ( uint32_t ) bytes.

        size_t offset = reinterpret_cast<size_t> ( view ) + static_cast<size_t> ( mipSize );
        offset -= reinterpret_cast<size_t> ( rawData );

        size_t rest = size - offset;
        auto* p = reinterpret_cast<void*> ( offset );

        view = rawData + reinterpret_cast<size_t> ( std::align ( sizeof ( uint32_t ), sizeof ( uint32_t ), p, rest ) );
    }

    _mipCount = static_cast<uint8_t> ( mips );
}

bool KTXMediaContainer::CheckField ( uint32_t field,
    char const* name,
    uint32_t expected,
    ReadHander reader, char const* fileName
)
{
    uint32_t const native = reader ( field );

    if ( native == expected )
        return true;

    constexpr char const format[] =
R"__(KTXMediaContainer::CheckField - Incompatibility was detected:
    file name: %s
    KTX header field: %s
    value: 0x%08X
    expected: 0x%08X)__";

    LogError ( format, fileName, name, native, expected );
    return false;
}

bool KTXMediaContainer::CheckSignature ( KTXHeader const &header, char const* fileName )
{
    if ( memcmp ( header._identifier, SIGNATURE, sizeof ( header._identifier ) ) == 0 )
        return true;

    LogError ( "KTXMediaContainer::CheckSignature - The file %s isn't a KTX v1 file.", fileName );
    return false;
}

bool KTXMediaContainer::CheckSize ( size_t size, char const* fileName )
{
    if ( size >= sizeof ( KTXHeader ) )
        return true;

    LogError ( "KTXMediaContainer::CheckSize - The file %s is too small [%s byte(s)].", fileName, size );
    return false;
}

uint8_t const* KTXMediaContainer::GetMipmapData ( uint8_t const* rawData,
    KTXHeader const &header,
    ReadHander reader
)
{
    constexpr size_t const rewind =
        offsetof ( KTXHeader, _bytesOfKeyValueData ) + sizeof ( header._bytesOfKeyValueData );

    return rawData + rewind + static_cast<size_t> ( reader ( header._bytesOfKeyValueData ) );
}

uint32_t KTXMediaContainer::ReadConvert ( uint32_t value )
{
    uint32_t v = ( value & 0x000000FFU ) << 24U;
    v |= ( value & 0x0000FF00U ) << 8U;
    v |= ( value & 0x00FF0000U ) >> 8U;
    return v | ( ( value & 0xFF000000U ) >> 24U );
}

uint32_t KTXMediaContainer::ReadNative ( uint32_t value )
{
    return value;
}

bool KTXMediaContainer::ResolveFormat ( uint32_t glInternalFormat,
    VkFormat &format,
    ReadHander reader,
    char const* fileName
)
{
    uint32_t const f = reader ( glInternalFormat );
    auto const findResult = g_FormatMapper.find ( f );

    if ( findResult != g_FormatMapper.cend () )
    {
        format = findResult->second;
        return true;
    }

    LogError ( "KTXMediaContainer::ResolveFormat - Can't map glFormat 0x%08X for file %s.", f, fileName );
    return false;
}

bool KTXMediaContainer::ResolveReader ( ReadHander &reader, uint32_t endianness, char const* fileName )
{
    constexpr char const errorFormat[] =
        R"__(KTXMediaContainer::ResolveReader - Can't resolve endianness for file %s. "endianness" equals 0x%08X.)__";

    switch ( endianness )
    {
        case NATIVE_ENDIANNESS:
            reader = &KTXMediaContainer::ReadNative;
        return true;

        case CONVERT_ENDIANNESS:
            reader = &KTXMediaContainer::ReadConvert;
        return true;

        default:
            LogError ( errorFormat, fileName, endianness );
        return false;
    }
}

} // namespace android_vulkan
