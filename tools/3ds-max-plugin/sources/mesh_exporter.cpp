#include <mesh_exporter.hpp>
#include <result_checker.hpp>
#include <GXCommon/GXMath.hpp>

GX_DISABLE_COMMON_WARNINGS

#include <maxapi.h>
#include <unordered_map>
#include <vector>
#include <IGame/IGameModifier.h>

GX_RESTORE_WARNING_STATE

// This stuff is defined in Windows SDK headers as macro. Same time such members exist
// in android_vulkan namespace as constants. Solution - undef TRUE and FALSE.
#if defined ( TRUE )
#undef TRUE
#endif

#if defined ( FALSE )
#undef FALSE
#endif

#include <mesh2.hpp>


namespace avp {

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
    std::unordered_map<Attributes, android_vulkan::Mesh2Index, Attributes::Hasher> uniqueMapper {};

    std::vector<android_vulkan::Mesh2Index> indices {};
    auto const indexCount = static_cast<size_t> ( faceCount * FACE_CORNERS );
    indices.reserve ( indexCount );
    android_vulkan::Mesh2Index idx = 0U;

    // Estimation from top.
    std::vector<android_vulkan::Mesh2Vertex> vertices ( indexCount );
    android_vulkan::Mesh2Vertex* v = vertices.data ();

    auto const uvToVec2 = [] ( Point3 p3, android_vulkan::Vec2 &v2 ) noexcept {
        v2[ 0U ] = p3.x;

        // [2023/09/16] For some reason 3ds Max 2023 SDK completely ignores 'V' direction in UserCoord.
        // Patching V coordinate by ourself...
        v2[ 1U ] = 1.0F - p3.y;
    };

    auto const p3ToVec3 = [] ( Point3 p3, android_vulkan::Vec3 &v3 ) noexcept {
        v3[ 0U ] = p3.x;
        v3[ 1U ] = p3.y;
        v3[ 2U ] = p3.z;
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

            android_vulkan::Mesh2Vertex &vertex = v[ idx ];

            Point3 const p = mesh.GetVertex ( attributes._position, true );
            p3ToVec3 ( p, vertex._vertex );
            bounds.AddVertex ( p.x, p.y, p.z );

            p3ToVec3 ( mesh.GetTangent ( attributes._tangentBitangent, true ), vertex._tangent );
            p3ToVec3 ( mesh.GetBinormal ( attributes._tangentBitangent, true ), vertex._bitangent );
            p3ToVec3 ( mesh.GetNormal ( attributes._normal, true ), vertex._normal );
            uvToVec2 ( mesh.GetMapVertex ( uvChannel, attributes._uv ), vertex._uv );

            indices.push_back ( idx );
            uniqueMapper.emplace ( attributes, idx++ );
        }
    }

    auto file = OpenFile ( parent, path );

    if ( !file )
        return;

    std::ofstream &f = *file;
    float const* bMin = bounds._min._data;
    float const* bMax = bounds._max._data;
    size_t const indexSize = indexCount * sizeof ( android_vulkan::Mesh2Index );

    android_vulkan::Mesh2Header const header
    {
        ._bounds = 
        {
            ._min = { bMin[ 0U ], bMin[ 1U ], bMin[ 2U ] },
            ._max = { bMax[ 0U ], bMax[ 1U ], bMax[ 2U ] }
        },

        ._indexCount = static_cast<uint64_t> ( indexCount ),
        ._indexDataOffset = static_cast<uint64_t> ( sizeof ( android_vulkan::Mesh2Header ) ),
        ._vertexCount = static_cast<uint64_t> ( idx ),
        ._vertexDataOffset = static_cast<uint64_t> ( sizeof ( android_vulkan::Mesh2Header ) + indexSize )
    };

    f.write ( reinterpret_cast<char const*> ( &header ), sizeof ( header ) );
    f.write ( reinterpret_cast<char const*> ( indices.data () ), static_cast<std::streamsize> ( indexSize ) );

    f.write ( reinterpret_cast<char const*> ( v ),
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
