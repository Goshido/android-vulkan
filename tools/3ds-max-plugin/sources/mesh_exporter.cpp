#include <precompiled_headers.hpp>
#include <half_types.hpp>
#include <mesh_exporter.hpp>
#include <result_checker.hpp>
#include <android_vulkan_sdk/mesh2.hpp>
#include <GXCommon/GXMath.hpp>

GX_DISABLE_COMMON_WARNINGS

#include <mikkt/mikktspace.h>

GX_RESTORE_WARNING_STATE


namespace avp {

namespace {

constexpr size_t INDEX16_LIMIT = 1U << 16U;
constexpr int MIKKT_SUCCESS = 1;

//----------------------------------------------------------------------------------------------------------------------

class RawData final
{
    public:
        android_vulkan::Mesh2Index32*                       _indices = nullptr;
        GXVec3*                                             _positions = nullptr;
        GXVec3*                                             _normals = nullptr;
        GXVec2*                                             _uvs = nullptr;
        android_vulkan::Mesh2Vertex*                        _vertices = nullptr;
        size_t const                                        _faceCount = 0U;

    private:
        std::unique_ptr<android_vulkan::Mesh2Index32[]>     _indexData {};
        size_t                                              _indexInsert = 0U;

        std::unique_ptr<float[]>                            _floatData {};
        std::unique_ptr<android_vulkan::Mesh2Vertex[]>      _vertexData {};

    public:
        RawData () = delete;

        RawData ( RawData const & ) = delete;
        RawData &operator = ( RawData const & ) = delete;

        RawData ( RawData && ) = delete;
        RawData &operator = ( RawData && ) = delete;

        explicit RawData ( size_t faceCount, size_t indexCount) noexcept;

        ~RawData () = default;

        void PushIndex ( android_vulkan::Mesh2Index32 index ) noexcept;
};

// Estimating size from top when every vertex in mesh is unique.
RawData::RawData ( size_t faceCount, size_t indexCount ) noexcept:
    _faceCount ( faceCount ),
    _indexData ( new android_vulkan::Mesh2Index32[ indexCount ] ),
    _floatData ( new float[ indexCount * ( 3U + 3U + 2U )]),
    _vertexData ( new android_vulkan::Mesh2Vertex[ indexCount ] )
{
    _indices = _indexData.get ();
    _positions = reinterpret_cast<GXVec3*> ( _floatData.get () );
    _normals = _positions + indexCount;
    _uvs = reinterpret_cast<GXVec2*> ( _normals + indexCount );
    _vertices = _vertexData.get ();
}

void RawData::PushIndex ( android_vulkan::Mesh2Index32 index ) noexcept
{
    _indices[ _indexInsert++ ] = index;
}

//----------------------------------------------------------------------------------------------------------------------

[[nodiscard]] int MikktGetNumFaces ( SMikkTSpaceContext const* pContext )
{
    return static_cast<int> ( static_cast<RawData*> ( pContext->m_pUserData )->_faceCount );
}

[[nodiscard]] int MikktGetNumVerticesOfFace ( SMikkTSpaceContext const* /*pContext*/, int /*iFace*/ )
{
    return Exporter::FACE_CORNERS;
}

void MikktGetPosition ( SMikkTSpaceContext const* pContext, float fvPosOut[], int iFace, int iVert )
{
    auto const &rawData = *static_cast<RawData const*> ( pContext->m_pUserData );

    android_vulkan::Mesh2Index32 const* faceIndices =
        rawData._indices + static_cast<size_t> ( iFace * Exporter::FACE_CORNERS );

    std::memcpy ( fvPosOut,
        rawData._positions + static_cast<size_t> ( faceIndices[ static_cast<size_t> ( iVert ) ] ),
        sizeof ( GXVec3 )
    );
}

void MikktGetNormal ( SMikkTSpaceContext const* pContext, float fvNormOut[], int iFace, int iVert )
{
    auto const &rawData = *static_cast<RawData const*> ( pContext->m_pUserData );

    android_vulkan::Mesh2Index32 const* faceIndices =
        rawData._indices + static_cast<size_t> ( iFace * Exporter::FACE_CORNERS );

    std::memcpy ( fvNormOut,
        rawData._normals + static_cast<size_t> ( faceIndices[ static_cast<size_t> ( iVert ) ] ),
        sizeof ( GXVec3 )
    );
}

void MikktGetTexCoord ( SMikkTSpaceContext const* pContext, float fvTexcOut[], int iFace, int iVert )
{
    auto const &rawData = *static_cast<RawData const*> ( pContext->m_pUserData );

    android_vulkan::Mesh2Index32 const* faceIndices =
        rawData._indices + static_cast<size_t> ( iFace * Exporter::FACE_CORNERS );

    std::memcpy ( fvTexcOut,
        rawData._uvs + static_cast<size_t> ( faceIndices[ static_cast<size_t> ( iVert ) ] ),
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
        rawData._indices + static_cast<size_t> ( iFace * Exporter::FACE_CORNERS );

    auto const index = static_cast<size_t> ( faceIndices[ static_cast<size_t> ( iVert ) ] );
    normal = rawData._normals[ index ];

    bitangent.CrossProduct ( normal, tangent );

    GXQuat tbn {};
    tbn.FromFast ( m );
    rawData._vertices[ index ]._tbn = tbn.Compress32 ( fSign < 0.0F );
}

} // end of anonymous namespace

//----------------------------------------------------------------------------------------------------------------------

void MeshExporter::Run ( HWND parent, MSTR const &path, bool exportInCurrentPose ) noexcept
{
    Interface17 &core = *GetCOREInterface17 ();

    if ( !CheckResult ( core.GetSelNodeCount () == 1, parent, "Please select single mesh", MB_ICONINFORMATION ) )
        return;

    INode &node = *core.GetSelNode ( 0 );
    AutoReleaseIGameScene autoRelease0 {};

    if ( !autoRelease0.Init ( parent ) )
        return;

    IGameScene &scene = autoRelease0.GetScene ();
    scene.SetStaticFrame ( 0 );
    AutoReleaseIGameNode autoRelease1 {};

    if ( !autoRelease1.Init ( parent, scene, node ) )
        return;

    IGameMesh &mesh = GetMesh ( autoRelease1.GetGameObject (), exportInCurrentPose );
    Tab<int> mapper = mesh.GetActiveMapChannelNum ();

    if ( !CheckResult ( mapper.Count () > 0, parent, "Please add UV map to mesh.", MB_ICONINFORMATION ) )
        return;

    int const uvChannel = mapper[ 0 ];
    int const faceCount = mesh.GetNumberOfFaces ();
    std::unordered_map<Attributes, android_vulkan::Mesh2Index32, Attributes::Hasher> uniqueMapper {};

    auto const indexCount = static_cast<size_t> ( faceCount * FACE_CORNERS );
    android_vulkan::Mesh2Index32 idx = 0U;

    RawData rawData ( static_cast<size_t> ( faceCount ), indexCount );
    GXVec3* pos = rawData._positions;
    GXVec3* n = rawData._normals;
    GXVec2* uvs = rawData._uvs;
    android_vulkan::Mesh2Vertex* v = rawData._vertices;

    constexpr auto uvToVec2 = [] ( Point3 p3, GXVec2 &uv, android_vulkan::HVec2 &h2 ) noexcept {
        // [2023/09/16] For some reason 3ds Max 2023 SDK completely ignores 'V' direction in UserCoord.
        // Patching V coordinate by ourself...
        float const invV = 1.0F - p3.y;

        uv._data[ 0U ] = p3.x;
        uv._data[ 1U ] = invV;

        android_vulkan::Half2 const hUV ( p3.x, invV );
        h2[ 0U ] = hUV._data[ 0U ];
        h2[ 1U ] = hUV._data[ 1U ];
    };

    constexpr auto p3ToGXVec3 = [] ( Point3 p3, GXVec3 &v3 ) noexcept {
        v3._data[ 0U ] = p3.x;
        v3._data[ 1U ] = p3.y;
        v3._data[ 2U ] = p3.z;
    };

    GXAABB bounds {};

    for ( int faceIdx = 0; faceIdx < faceCount; ++faceIdx )
    {
        for ( int cornerIdx = 0; cornerIdx < FACE_CORNERS; ++cornerIdx )
        {
            Attributes const attributes
            {
                ._normal = mesh.GetFaceVertexNormal ( faceIdx, cornerIdx ),
                ._position = mesh.GetFaceVertex ( faceIdx, cornerIdx ),
                ._tangentBitangent = mesh.GetFaceVertexTangentBinormal ( faceIdx, cornerIdx, uvChannel ),
                ._uv = mesh.GetFaceTextureVertex ( faceIdx, cornerIdx, uvChannel )
            };

            if ( auto const findResult = uniqueMapper.find ( attributes ); findResult != uniqueMapper.cend () )
            {
                rawData.PushIndex ( findResult->second );
                continue;
            }

            GXVec3 &position = pos[ idx ];
            Point3 const p = mesh.GetVertex ( attributes._position, false );
            p3ToGXVec3 ( p, position );
            bounds.AddVertex ( p.x, p.y, p.z );

            uvToVec2 ( mesh.GetMapVertex ( uvChannel, attributes._uv ), uvs[ idx ], v[ idx ]._uv );
            p3ToGXVec3 ( mesh.GetNormal ( attributes._normal, false ), n[ idx ] );

            rawData.PushIndex ( idx );
            uniqueMapper.emplace ( attributes, idx++ );
        }
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

    bool const result = CheckResult ( genTangSpaceDefault ( &mikktContext ) == MIKKT_SUCCESS,
        parent,
        "Can't compute Mikkt tangents.",
        MB_ICONINFORMATION
    );

    if ( !result )
        return;

    auto f = OpenFile ( parent, path );

    std::ofstream &file = *f;
    float const* bMin = bounds._min._data;
    float const* bMax = bounds._max._data;

    size_t indexSize = indexCount * sizeof ( android_vulkan::Mesh2Index32 );
    void* indexData = rawData._indices;
    std::unique_ptr<android_vulkan::Mesh2Index16[]> index16Storage {};

    if ( indexCount < INDEX16_LIMIT )
    {
        index16Storage.reset ( new android_vulkan::Mesh2Index16[ indexCount ] );
        android_vulkan::Mesh2Index16* dst = index16Storage.get ();
        android_vulkan::Mesh2Index32 const* src = rawData._indices;

        for ( size_t i = 0U; i < indexCount; ++i )
            dst[ i ] = static_cast<android_vulkan::Mesh2Index16> ( src[ i ] );

        indexSize = indexCount * sizeof ( android_vulkan::Mesh2Index16 );
        indexData = dst;
    }

    size_t const indexOffset = sizeof ( android_vulkan::Mesh2Header );
    size_t const positionOffset = indexOffset + indexSize;
    size_t const positionSize = idx * sizeof ( android_vulkan::Mesh2Position );
    size_t const vertexOffset = positionOffset + positionSize;

    android_vulkan::Mesh2Header const header
    {
        ._bounds =
        {
            ._min = { bMin[ 0U ], bMin[ 1U ], bMin[ 2U ] },
            ._max = { bMax[ 0U ], bMax[ 1U ], bMax[ 2U ] }
        },

        ._indexCount = static_cast<uint32_t> ( indexCount ),
        ._indexDataOffset = static_cast<uint64_t> ( indexOffset ),
        ._vertexCount = static_cast<uint32_t> ( idx ),
        ._positionDataOffset = static_cast<uint64_t> ( positionOffset ),
        ._vertexDataOffset = static_cast<uint64_t> ( vertexOffset )
    };

    file.write ( reinterpret_cast<char const*> ( &header ), static_cast<std::streamsize> ( sizeof ( header ) ) );
    file.write ( static_cast<char const*> ( indexData ), static_cast<std::streamsize> ( indexSize ) );
    file.write ( reinterpret_cast<char const*> ( pos ), static_cast<std::streamsize> ( positionSize ) );

    file.write ( reinterpret_cast<char const*> ( v ),
        static_cast<std::streamsize> ( idx * sizeof ( android_vulkan::Mesh2Vertex ) )
    );

    MessageBoxA ( parent, "Done.", "android-vulkan", MB_ICONINFORMATION );
}

IGameMesh &MeshExporter::GetMesh ( IGameObject &object, bool exportInCurrentPose ) noexcept
{
    IGameSkin* skin = object.GetIGameSkin ();

    if ( exportInCurrentPose | !skin )
    {
        // NOLINTNEXTLINE - downcast.
        return static_cast<IGameMesh &> ( object );
    }

    return *skin->GetInitialPose ();
}

} // namespace avp
