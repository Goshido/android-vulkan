#include <precompiled_headers.hpp>
#include <crash_dump.hpp>
#include <editor.hpp>


// FUCK
#include <android_vulkan_sdk/mesh2.hpp>
#include <file.hpp>
#include <logger.hpp>
#include <half_types.hpp>
#include <vertex_info.hpp>

GX_DISABLE_COMMON_WARNINGS

#include <span>

GX_RESTORE_WARNING_STATE


namespace {

constexpr size_t IND16_VS_IND32 = 1U << 16U;

constexpr size_t const IND_CASES[] =
{
    sizeof ( android_vulkan::Mesh2Index16 ),
    sizeof ( android_vulkan::Mesh2Index32 )
};

constexpr size_t INDEX_DATA_OFFSET = sizeof ( android_vulkan::Mesh2Header );

#pragma pack ( push, 1 )

struct Mesh2VertexOld final
{
    float       _uv[ 2U ];
    uint32_t    _tbn;
};

#pragma pack ( pop )

void RepackVertices ( std::span<android_vulkan::VertexInfo> dstVertices,
    std::span<Mesh2VertexOld const> srcVertices
) noexcept
{
    size_t const count = srcVertices.size ();

    for ( size_t i = 0U; i < count; ++i )
    {
        Mesh2VertexOld const &src = srcVertices[ i ];
        android_vulkan::VertexInfo &dst = dstVertices[ i ];

        dst._tbn = src._tbn;
        dst._uv = android_vulkan::Half2 ( src._uv[ 0U ], src._uv[ 1U ] );
    }
}

void RepackMesh2 ( std::string const &path )
{
    android_vulkan::File srcFile ( path );

    if ( !srcFile.LoadContent () )
        return;

    std::vector<uint8_t> const &content = srcFile.GetContent ();
    uint8_t const* srcPtr = content.data ();
    auto const &srcHeader = *reinterpret_cast<android_vulkan::Mesh2Header const*> ( srcPtr );

    auto const indCount = static_cast<size_t> ( srcHeader._indexCount );
    auto const vertexCount = static_cast<size_t> ( srcHeader._vertexCount );
    size_t const indSize = indCount * IND_CASES[ static_cast<size_t> ( vertexCount >= IND16_VS_IND32 ) ];

    size_t const dstFileSize = sizeof ( android_vulkan::Mesh2Header ) +
        indSize +
        vertexCount * ( sizeof ( android_vulkan::Mesh2Position ) + sizeof ( android_vulkan::VertexInfo ) );

    std::unique_ptr<uint8_t[]> dstContent ( new uint8_t[ dstFileSize ] );
    uint8_t* dstPtr = dstContent.get ();
    auto &dstHeader = *reinterpret_cast<android_vulkan::Mesh2Header*> ( dstPtr );

    dstHeader._indexCount = static_cast<uint32_t> ( indCount );
    dstHeader._indexDataOffset = static_cast<uint64_t> ( INDEX_DATA_OFFSET );

    dstHeader._vertexCount = static_cast<uint32_t> ( vertexCount );
    dstHeader._positionDataOffset = static_cast<uint64_t> ( INDEX_DATA_OFFSET + indSize );

    size_t const positionDataSize = vertexCount * sizeof ( android_vulkan::Mesh2Position );
    size_t const vertexDataOffset = dstHeader._positionDataOffset + positionDataSize;
    dstHeader._vertexDataOffset = static_cast<uint64_t> ( vertexDataOffset );

    dstHeader._bounds = srcHeader._bounds;

    std::memcpy ( dstPtr + dstHeader._indexDataOffset, srcPtr + srcHeader._indexDataOffset, indSize );
    std::memcpy ( dstPtr + dstHeader._positionDataOffset, srcPtr + srcHeader._positionDataOffset, positionDataSize );

    RepackVertices ( { reinterpret_cast<android_vulkan::VertexInfo*> ( dstPtr + vertexDataOffset ), vertexCount },
        { reinterpret_cast<Mesh2VertexOld const*> ( srcPtr + vertexDataOffset ), vertexCount }
    );

    std::ofstream dstFile ( path, std::ios::binary );
    dstFile.write ( reinterpret_cast<char*> ( dstPtr ), static_cast<std::streamsize> ( dstFileSize ) );
}

void Repack () noexcept
{
    std::deque<std::string> mesh2Items {};
    constexpr std::string_view mesh2Ext = ".mesh2";

    auto it = std::filesystem::recursive_directory_iterator (
        "D:/Development/android-vulkan/app/src/main/assets"
    );

    for ( auto const &entry : it )
    {
        if ( entry.is_directory () || entry.is_symlink () || !entry.is_regular_file () )
            continue;

        std::filesystem::path const path = entry.path ();
        auto const ext = path.extension ();

        if ( ext == mesh2Ext ) [[unlikely]]
        {
            mesh2Items.push_back ( path.string () );
        }
    }

    if ( mesh2Items.empty () ) [[unlikely]]
        return;

    constexpr char const format[] = R"__(Mesh2 items: %zu
>>>)__";

    android_vulkan::LogDebug ( format, mesh2Items.size () );

    for ( std::string const& path : mesh2Items )
    {
        RepackMesh2 ( path );
        android_vulkan::LogDebug ( "    %s", path.c_str () );
    }

    android_vulkan::LogDebug ( "<<<" );
}

} // end of anonymous namespace

[[nodiscard]] int main ( int argc, char** argv )
{
    Repack ();

    if ( !editor::CrashDump::Install () )
        return EXIT_FAILURE;

    constexpr int skipExePath = 1;

    std::unique_ptr<editor::Editor> editor = std::make_unique<editor::Editor> (
        editor::CommandLine ( argv + skipExePath, static_cast<size_t> ( argc - skipExePath ) )
    );

    return editor->Run () ? EXIT_SUCCESS : EXIT_FAILURE;
}
