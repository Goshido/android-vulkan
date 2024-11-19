#include <precompiled_headers.hpp>
#include <crash_dump.hpp>
#include <editor.hpp>


// FUCK
#include <android_vulkan_sdk/mesh2.hpp>
#include <file.hpp>
#include <logger.hpp>

GX_DISABLE_COMMON_WARNINGS

#include <mikktspace.h>

GX_RESTORE_WARNING_STATE


namespace {

constexpr size_t DATA_PER_VERTEX = sizeof ( android_vulkan::Mesh2Position ) + sizeof ( android_vulkan::Mesh2Vertex );
constexpr size_t IND16_VS_IND32 = 1U << 16U;

constexpr size_t const IND_CASES[] =
{
    sizeof ( android_vulkan::Mesh2Index16 ),
    sizeof ( android_vulkan::Mesh2Index32 )
};

constexpr size_t INDEX_DATA_OFFSET = sizeof ( android_vulkan::Mesh2Header );

#pragma pack ( push, 1 )

struct GXNativeMeshHeader final
{
    GXUInt          totalVertices;
    GXUBigInt       vboOffset;      // VBO element struct: position (GXVec3), uv (GXVec2), normal (GXVec3), tangent (GXVec3), bitangent (GXVec3).
};

struct Mesh2HeaderOLD final
{
    android_vulkan::AABB                            _bounds;

    uint64_t                        _indexCount;
    uint64_t                        _indexDataOffset;

    uint32_t                        _vertexCount;
    [[maybe_unused]] uint64_t       _vertexDataOffset;
};

struct Mesh2VertexOLD final
{
     [[maybe_unused]] android_vulkan::Vec3      _vertex;
     [[maybe_unused]] android_vulkan::Vec2      _uv;
     android_vulkan::Vec3                       _normal;
     [[maybe_unused]] android_vulkan::Vec3      _tangent;
     [[maybe_unused]] android_vulkan::Vec3      _bitangent;
};

using Mesh2IndexOLD [[maybe_unused]] = uint32_t;


struct DstVertex final
{
    GXVec2      _uv;
    uint32_t    _tbn;
};

struct Vertex final
{
    GXVec3      _position;
    GXVec2      _uv;
    GXVec3      _normal;
    GXVec3      _tangent;
    GXVec3      _bitangent;
};

#pragma pack ( pop )

class RawData final
{
    public:
        android_vulkan::Mesh2Index32*                       _indices = nullptr;
        GXVec3*                                             _positions = nullptr;
        GXVec3*                                             _normals = nullptr;
        android_vulkan::Mesh2Vertex*                        _vertices = nullptr;
        size_t const                                        _faceCount = 0U;

    private:
        std::unique_ptr<android_vulkan::Mesh2Index32[]>     _indexData {};
        std::unique_ptr<GXVec3[]>                           _vec3Data {};
        std::unique_ptr<android_vulkan::Mesh2Vertex[]>      _vertexData {};

    public:
        RawData () = delete;

        RawData ( RawData const & ) = delete;
        RawData &operator = ( RawData const & ) = delete;

        RawData ( RawData && ) = delete;
        RawData &operator = ( RawData && ) = delete;

        explicit RawData ( size_t faceCount, size_t indexCount) noexcept;

        ~RawData () = default;
};

RawData::RawData ( size_t faceCount, size_t indexCount ) noexcept:
    _faceCount ( faceCount ),
    _indexData ( new android_vulkan::Mesh2Index32[ indexCount ] ),
    _vec3Data ( new GXVec3[ indexCount * 2U ] ),
    _vertexData ( new android_vulkan::Mesh2Vertex[ indexCount ] )
{
    _indices = _indexData.get ();
    _positions = _vec3Data.get ();
    _normals = _positions + indexCount;
    _vertices = _vertexData.get ();
}

//----------------------------------------------------------------------------------------------------------------------

[[nodiscard]] int MikktGetNumFaces ( SMikkTSpaceContext const* pContext )
{
    return static_cast<int> ( static_cast<RawData*> ( pContext->m_pUserData )->_faceCount );
}

[[nodiscard]] int MikktGetNumVerticesOfFace ( SMikkTSpaceContext const* /*pContext*/, int /*iFace*/ )
{
    return 3;
}

void MikktGetPosition ( SMikkTSpaceContext const* pContext, float fvPosOut[], int iFace, int iVert )
{
    auto const &rawData = *static_cast<RawData const*> ( pContext->m_pUserData );

    android_vulkan::Mesh2Index32 const* faceIndices =
        rawData._indices + static_cast<size_t> ( iFace * 3 );

    std::memcpy ( fvPosOut,
        rawData._positions + static_cast<size_t> ( faceIndices[ static_cast<size_t> ( iVert ) ] ),
        sizeof ( GXVec3 )
    );
}

void MikktGetNormal ( SMikkTSpaceContext const* pContext, float fvNormOut[], int iFace, int iVert )
{
    auto const &rawData = *static_cast<RawData const*> ( pContext->m_pUserData );

    android_vulkan::Mesh2Index32 const* faceIndices =
        rawData._indices + static_cast<size_t> ( iFace * 3 );

    std::memcpy ( fvNormOut,
        rawData._normals + static_cast<size_t> ( faceIndices[ static_cast<size_t> ( iVert ) ] ),
        sizeof ( GXVec3 )
    );
}

void MikktGetTexCoord ( SMikkTSpaceContext const* pContext, float fvTexcOut[], int iFace, int iVert )
{
    auto const &rawData = *static_cast<RawData const*> ( pContext->m_pUserData );

    android_vulkan::Mesh2Index32 const* faceIndices =
        rawData._indices + static_cast<size_t> ( iFace * 3 );

    std::memcpy ( fvTexcOut,
        &rawData._vertices[ static_cast<size_t> ( faceIndices[ static_cast<size_t> ( iVert ) ] ) ]._uv,
        sizeof ( GXVec2 )
    );
}

void MikktSetTSpaceBasic ( SMikkTSpaceContext const* pContext,
    float const fvTangent[],
    float fSign,
    int iFace,
    int iVert
)
{
    auto &rawData = *static_cast<RawData*> ( pContext->m_pUserData );

    GXMat3 m {};
    auto &tangent = *reinterpret_cast<GXVec3*> ( &m._m[ 0U ][ 0U ] );
    auto &bitangent = *reinterpret_cast<GXVec3*> ( &m._m[ 1U ][ 0U ] );
    auto &normal = *reinterpret_cast<GXVec3*> ( &m._m[ 2U ][ 0U ] );

    std::memcpy ( &tangent, fvTangent, sizeof ( GXVec3 ) );

    android_vulkan::Mesh2Index32 const* faceIndices =
        rawData._indices + static_cast<size_t> ( iFace * 3 );

    auto const index = static_cast<size_t> ( faceIndices[ static_cast<size_t> ( iVert ) ] );
    normal = rawData._normals[ index ];

    bitangent.CrossProduct ( normal, tangent );

    GXQuat tbn {};
    tbn.FromFast ( m );
    rawData._vertices[ index ]._tbn = tbn.Compress ( fSign < 0.0F );
}

void RepackVertices ( GXVec3* dstPositions,
    DstVertex* dstVertices,
    GXAABB &bounds,
    std::span<Vertex const> srcData,
    bool uvInverseV,
    RawData &rawData
) noexcept
{
    size_t const count = srcData.size ();
    Vertex const* srcVertices = srcData.data ();
    auto const selector = static_cast<size_t> ( uvInverseV );

    for ( size_t i = 0U; i < count; ++i )
    {
        Vertex const &srcVertex = srcVertices[ i ];
        GXVec3 const &srcPosition = srcVertex._position;
        bounds.AddVertex ( srcPosition );
        dstPositions[ i ] = srcPosition;

        rawData._positions[ i ] = srcVertex._position;
        rawData._normals[ i ] = srcVertex._normal;

        android_vulkan::Vec2 &dstUV = rawData._vertices[ i ]._uv;
        GXVec2 const &srcUV = srcVertex._uv;

        float const v = srcUV._data[ 1U ];
        float const cases[] = { v, 1.0F - v };
        dstUV[ 0U ] = srcUV._data[ 0U ];
        dstUV[ 1U ] = cases[ selector ];
    }

    SMikkTSpaceInterface mikktInterface
    {
        .m_getNumFaces = &MikktGetNumFaces,
        .m_getNumVerticesOfFace = &MikktGetNumVerticesOfFace,
        .m_getPosition = &MikktGetPosition,
        .m_getNormal = &MikktGetNormal,
        .m_getTexCoord = &MikktGetTexCoord,
        .m_setTSpaceBasic = &MikktSetTSpaceBasic,
        .m_setTSpace = nullptr
    };

    SMikkTSpaceContext const mikktContext
    {
        .m_pInterface = &mikktInterface,
        .m_pUserData = &rawData
    };

    if ( genTangSpaceDefault ( &mikktContext ) == 0 )
    {
        printf ( "FUCK" );
    }

    for ( size_t i = 0U; i < count; ++i )
    {
        dstPositions[ i ] = rawData._positions[ i ];

        DstVertex &dst = dstVertices[ i ];
        android_vulkan::Mesh2Vertex const &src = rawData._vertices[ i ];

        std::memcpy ( &dst._uv, &src._uv, sizeof ( GXVec2 ) );
        dst._tbn = src._tbn;
    }
}

void RepackMesh ( std::string const &path ) noexcept
{
    android_vulkan::File srcFile ( path );

    if ( !srcFile.LoadContent () )
        return;

    std::vector<uint8_t> const &content = srcFile.GetContent ();
    uint8_t const* srcPtr = content.data ();
    auto const &srcHeader = *reinterpret_cast<GXNativeMeshHeader const*> ( srcPtr );
    auto const count = static_cast<size_t> ( srcHeader.totalVertices );

    size_t const indSize = count * IND_CASES[ static_cast<size_t> ( srcHeader.totalVertices >= IND16_VS_IND32 ) ];
    size_t const dstFileSize = sizeof ( android_vulkan::Mesh2Header ) + indSize + count * DATA_PER_VERTEX;

    std::unique_ptr<uint8_t[]> dstContent ( new uint8_t[ dstFileSize ] );
    uint8_t* dstPtr = dstContent.get ();
    auto &dstHeader = *reinterpret_cast<android_vulkan::Mesh2Header*> ( dstPtr );

    dstHeader._indexCount = static_cast<uint32_t> ( count );
    dstHeader._indexDataOffset = static_cast<uint64_t> ( INDEX_DATA_OFFSET );

    dstHeader._vertexCount = dstHeader._indexCount;
    dstHeader._positionDataOffset = static_cast<uint64_t> ( INDEX_DATA_OFFSET + indSize );
    dstHeader._vertexDataOffset = dstHeader._positionDataOffset + count * sizeof ( android_vulkan::Mesh2Position );

    GXAABB bounds {};
    RawData rawData ( count / 3U, count );
    std::iota ( rawData._indices, rawData._indices + count, 0U );

    RepackVertices ( reinterpret_cast<GXVec3*> ( dstPtr + dstHeader._positionDataOffset ),
        reinterpret_cast<DstVertex*> ( dstPtr + dstHeader._vertexDataOffset ),
        bounds,
        { reinterpret_cast<Vertex const*> ( srcPtr + static_cast<size_t> ( srcHeader.vboOffset ) ), count },
        true,
        rawData
    );

    if ( srcHeader.totalVertices >= IND16_VS_IND32 )
    {
        auto* start = reinterpret_cast<android_vulkan::Mesh2Index32*> ( dstPtr + dstHeader._indexDataOffset );
        std::iota ( start, start + count, static_cast<android_vulkan::Mesh2Index32> ( 0U ) );
    }
    else
    {
        auto* start = reinterpret_cast<android_vulkan::Mesh2Index16*> ( dstPtr + dstHeader._indexDataOffset );
        std::iota ( start, start + count, static_cast<android_vulkan::Mesh2Index16> ( 0U ) );
    }

    android_vulkan::AABB &b = dstHeader._bounds;
    std::memcpy ( b._max, bounds._max._data, sizeof ( android_vulkan::Vec3 ) );
    std::memcpy ( b._min, bounds._min._data, sizeof ( android_vulkan::Vec3 ) );

    std::ofstream dstFile ( path, std::ios::binary );
    dstFile.write ( reinterpret_cast<char*> ( dstPtr ), static_cast<std::streamsize> ( dstFileSize ) );
}

void RepackMesh2 ( std::string const &path )
{
    android_vulkan::File srcFile ( path );

    if ( !srcFile.LoadContent () )
        return;

    std::vector<uint8_t> const &content = srcFile.GetContent ();
    uint8_t const* srcPtr = content.data ();
    auto const &srcHeader = *reinterpret_cast<Mesh2HeaderOLD const*> ( srcPtr );

    auto const indCount = static_cast<size_t> ( srcHeader._indexCount );
    auto const vertexCount = static_cast<size_t> ( srcHeader._vertexCount );
    size_t const indSize = indCount * IND_CASES[ static_cast<size_t> ( vertexCount >= IND16_VS_IND32 ) ];
    size_t const dstFileSize = sizeof ( android_vulkan::Mesh2Header ) + indSize + vertexCount * DATA_PER_VERTEX;

    std::unique_ptr<uint8_t[]> dstContent ( new uint8_t[ dstFileSize ] );
    uint8_t* dstPtr = dstContent.get ();
    auto &dstHeader = *reinterpret_cast<android_vulkan::Mesh2Header*> ( dstPtr );

    dstHeader._indexCount = static_cast<uint32_t> ( indCount );
    dstHeader._indexDataOffset = static_cast<uint64_t> ( INDEX_DATA_OFFSET );

    dstHeader._vertexCount = static_cast<uint32_t> ( vertexCount );
    dstHeader._positionDataOffset = static_cast<uint64_t> ( INDEX_DATA_OFFSET + indSize );

    dstHeader._vertexDataOffset = dstHeader._positionDataOffset +
        vertexCount * sizeof ( android_vulkan::Mesh2Position );

    dstHeader._bounds = srcHeader._bounds;

    GXAABB bounds {};
    RawData rawData ( indCount / 3U, indCount );

    std::memcpy ( rawData._indices,
        srcPtr + srcHeader._indexDataOffset,
        indCount * sizeof ( android_vulkan::Mesh2Index32 )
    );

    RepackVertices ( reinterpret_cast<GXVec3*> ( dstPtr + dstHeader._positionDataOffset ),
        reinterpret_cast<DstVertex*> ( dstPtr + dstHeader._vertexDataOffset ),
        bounds,

        {
            reinterpret_cast<Vertex const*> ( srcPtr + static_cast<size_t> ( srcHeader._vertexDataOffset ) ),
            vertexCount
        },

        false,
        rawData
    );

    if ( vertexCount >= IND16_VS_IND32 )
    {
        std::memcpy ( dstPtr + dstHeader._indexDataOffset,
            srcPtr + srcHeader._indexDataOffset,
            indCount * sizeof ( android_vulkan::Mesh2Index32 )
        );
    }
    else
    {
        auto const* srcInd = reinterpret_cast<Mesh2IndexOLD const*> (
            srcPtr + srcHeader._indexDataOffset
        );

        auto* dstInd = reinterpret_cast<android_vulkan::Mesh2Index16*> ( dstPtr + dstHeader._indexDataOffset );

        for ( size_t i = 0U; i < indCount; ++i )
        {
            dstInd[ i ] = static_cast<android_vulkan::Mesh2Index16> ( srcInd[ i ] );
        }
    }

    std::ofstream dstFile ( path, std::ios::binary );
    dstFile.write ( reinterpret_cast<char*> ( dstPtr ), static_cast<std::streamsize> ( dstFileSize ) );
}

void Repack () noexcept
{
    std::deque<std::string> meshItems {};
    std::deque<std::string> mesh2Items {};

    constexpr std::string_view meshExt = ".mesh";
    constexpr std::string_view mesh2Ext = ".mesh2";

    for ( auto const &entry : std::filesystem::recursive_directory_iterator ( "D:/Development/android-vulkan/app/src/main/assets" ) )
    {
        if ( entry.is_directory () || entry.is_symlink () || !entry.is_regular_file () )
            continue;

        std::filesystem::path const path = entry.path ();
        auto const ext = path.extension ();

        if ( ext == meshExt )
        {
            meshItems.push_back ( path.string () );
            continue;
        }

        if ( ext == mesh2Ext ) [[unlikely]]
        {
            mesh2Items.push_back ( path.string () );
        }
    }

    if ( !meshItems.empty () ) [[likely]]
    {
        constexpr char const format[] = R"__(Mesh items: %zu
>>>)__";

        android_vulkan::LogDebug ( format, meshItems.size () );

        for ( std::string const &path : meshItems )
        {
            RepackMesh ( path );
            android_vulkan::LogDebug ( "    %s", path.c_str () );
        }

        android_vulkan::LogDebug ( "<<<" );
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
