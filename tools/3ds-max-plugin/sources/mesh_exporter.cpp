#include <precompiled_headers.hpp>
#include <mesh_exporter.hpp>
#include <result_checker.hpp>
#include <android_vulkan_sdk/mesh2.hpp>
#include <GXCommon/GXMath.hpp>


namespace avp {

namespace {

constexpr float REPACK_THRESHOLD = 1.0e-4F;
constexpr size_t INDEX16_LIMIT = 1U << 16U;

} // end of anonymous namespace

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

    std::vector<android_vulkan::Mesh2Index32> indices {};
    auto const indexCount = static_cast<size_t> ( faceCount * FACE_CORNERS );
    indices.reserve ( indexCount );
    android_vulkan::Mesh2Index32 idx = 0U;

    // Estimation from top.
    std::vector<GXVec3> positions ( indexCount );
    std::vector<android_vulkan::Mesh2Vertex> vertices ( indexCount );
    GXVec3* pos = positions.data ();
    android_vulkan::Mesh2Vertex* v = vertices.data ();

    constexpr auto uvToVec2 = [] ( Point3 p3, android_vulkan::Vec2 &v2 ) noexcept {
        v2[ 0U ] = p3.x;

        // [2023/09/16] For some reason 3ds Max 2023 SDK completely ignores 'V' direction in UserCoord.
        // Patching V coordinate by ourself...
        v2[ 1U ] = 1.0F - p3.y;
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
                indices.push_back ( findResult->second );
                continue;
            }

            GXVec3 &position = pos[ idx ];
            android_vulkan::Mesh2Vertex &vertex = v[ idx ];

            Point3 const p = mesh.GetVertex ( attributes._position, false );
            p3ToGXVec3 ( p, position );
            bounds.AddVertex ( p.x, p.y, p.z );

            uvToVec2 ( mesh.GetMapVertex ( uvChannel, attributes._uv ), vertex._uv );

            GXMat3 m {};
            auto &tangent = *reinterpret_cast<GXVec3*> ( &m._m[ 0U ][ 0U ] );
            auto &bitangent = *reinterpret_cast<GXVec3*> ( &m._m[ 1U ][ 0U ] );
            auto &normal = *reinterpret_cast<GXVec3*> ( &m._m[ 2U ][ 0U ] );

            p3ToGXVec3 ( mesh.GetTangent ( attributes._tangentBitangent, uvChannel ), tangent );
            tangent.Normalize ();

            p3ToGXVec3 ( mesh.GetNormal ( attributes._normal, false ), normal );
            normal.Normalize ();

            GXVec3 testBitangent {};
            p3ToGXVec3 ( mesh.GetBinormal ( attributes._tangentBitangent, uvChannel ), testBitangent );
            testBitangent.Normalize ();

            if ( float const ortho = std::abs ( tangent.DotProduct ( normal ) ); ortho > REPACK_THRESHOLD ) [[likely]]
            {
                bitangent.CrossProduct ( normal, tangent );
            }
            else
            {
                GXVec3 n = normal;
                n.Normalize ();
                m.From ( n );
            }

            GXQuat tbn {};
            tbn.FromFast ( m );
            vertex._tbn = tbn.Compress ( bitangent.DotProduct ( testBitangent ) < 0.0F );

            indices.push_back ( idx );
            uniqueMapper.emplace ( attributes, idx++ );
        }
    }

    auto f = OpenFile ( parent, path );

    if ( !f )
        return;

    std::ofstream &file = *f;
    float const* bMin = bounds._min._data;
    float const* bMax = bounds._max._data;

    size_t indexSize = indexCount * sizeof ( android_vulkan::Mesh2Index32 );
    void* indexData = indices.data ();
    std::unique_ptr<android_vulkan::Mesh2Index16[]> index16Storage {};

    if ( indexCount < INDEX16_LIMIT )
    {
        index16Storage.reset ( new android_vulkan::Mesh2Index16[ indexCount ] );
        android_vulkan::Mesh2Index16* dst = index16Storage.get ();
        android_vulkan::Mesh2Index32 const* src = indices.data ();

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
