#include <mesh_exporter.hpp>
#include <GXCommon/GXMath.hpp>

GX_DISABLE_COMMON_WARNINGS

#include <filesystem>
#include <fstream>
#include <maxapi.h>
#include <unordered_map>
#include <vector>
#include <IGame/IGame.h>
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

namespace {

[[nodiscard]] bool CheckResult ( bool result, HWND parent, char const *message, UINT icon ) noexcept
{
    if ( result )
        return true;

    MessageBoxA ( parent, message, "android-vulkan", icon );
    return false;
}

//----------------------------------------------------------------------------------------------------------------------

class AutoReleaseIGameScene final
{
    private:
        IGameScene*     _scene = nullptr;

    public:
        AutoReleaseIGameScene () = default;

        AutoReleaseIGameScene ( AutoReleaseIGameScene const & ) = delete;
        AutoReleaseIGameScene &operator = ( AutoReleaseIGameScene const & ) = delete;

        AutoReleaseIGameScene ( AutoReleaseIGameScene && ) = delete;
        AutoReleaseIGameScene &operator = ( AutoReleaseIGameScene && ) = delete;

        ~AutoReleaseIGameScene () noexcept;

        [[nodiscard]] IGameScene &GetScene () noexcept;
        [[nodiscard]] bool Init ( HWND parent ) noexcept;
};

AutoReleaseIGameScene::~AutoReleaseIGameScene () noexcept
{
    if ( _scene )
    {
        _scene->ReleaseIGame ();
    }
}

IGameScene &AutoReleaseIGameScene::GetScene () noexcept
{
    return *_scene;
}

bool AutoReleaseIGameScene::Init ( HWND parent ) noexcept
{
    _scene = GetIGameInterface ();

    bool const result = CheckResult ( _scene != nullptr, parent, "Can't get IGameScene.", MB_ICONWARNING ) &&
        CheckResult ( _scene->InitialiseIGame ( true ), parent, "Can't init IGameScene.", MB_ICONWARNING );

    if ( !result )
        return false;

    IGameConversionManager &conventions = *GetConversionManager ();

    conventions.SetUserCoordSystem (
        UserCoord
        {
            .rotation = 0,
            .xAxis = 1,
            .yAxis = 2,
            .zAxis = 4,
            .uAxis = 1,
            .vAxis = 1
        }
    );

    conventions.SetCoordSystem ( IGameConversionManager::CoordSystem::IGAME_USER );
    return true;
}

//----------------------------------------------------------------------------------------------------------------------

class AutoReleaseIGameNode final
{
    private:
        IGameNode*      _node = nullptr;
        IGameObject*    _object = nullptr;

    public:
        AutoReleaseIGameNode () = default;

        AutoReleaseIGameNode ( AutoReleaseIGameNode const & ) = delete;
        AutoReleaseIGameNode &operator = ( AutoReleaseIGameNode const & ) = delete;

        AutoReleaseIGameNode ( AutoReleaseIGameNode && ) = delete;
        AutoReleaseIGameNode &operator = ( AutoReleaseIGameNode && ) = delete;

        ~AutoReleaseIGameNode () noexcept;

        [[nodiscard]] IGameObject &GetGameObject () noexcept;
        [[nodiscard]] bool Init ( HWND parent, IGameScene &scene, INode &node ) noexcept;
};

AutoReleaseIGameNode::~AutoReleaseIGameNode () noexcept
{
    if ( _object )
    {
        _node->ReleaseIGameObject ();
    }
}

IGameObject &AutoReleaseIGameNode::GetGameObject () noexcept
{
    return *_object;
}

bool AutoReleaseIGameNode::Init ( HWND parent, IGameScene &scene, INode &node ) noexcept
{
    _node = scene.GetIGameNode ( &node );

    if ( !CheckResult ( _node != nullptr, parent, "Can't init IGameNode.", MB_ICONWARNING ) )
        return false;

    _object = _node->GetIGameObject ();

    return CheckResult ( _object->InitializeData (), parent, "Can't init IGameObject.", MB_ICONWARNING ) &&

        CheckResult ( _object->GetIGameType () == IGameMesh::IGAME_MESH,
            parent, 
            "Please select mesh to export.",
            MB_ICONINFORMATION
        );
}

//----------------------------------------------------------------------------------------------------------------------

struct Attributes final
{
    int     _normal;
    int     _position;
    int     _tangentBitangent;
    int     _uv;

    [[nodiscard]] bool operator == ( Attributes const &other ) const noexcept;
};

bool Attributes::operator == ( Attributes const &other ) const noexcept
{
    bool const c0 = _normal == other._normal;
    bool const c1 = _position == other._position;
    bool const c2 = _tangentBitangent == other._tangentBitangent;
    bool const c3 = _uv == other._uv;
    return c0 & c1 & c2 & c3;
}

//----------------------------------------------------------------------------------------------------------------------

class Hasher final
{
    private:
        std::hash<uint64_t> const       _hashServer {};

    public:
        Hasher () = default;

        Hasher ( Hasher const & ) = default;
        Hasher &operator = ( Hasher const & ) = delete;

        Hasher ( Hasher && ) = delete;
        Hasher &operator = ( Hasher && ) = delete;

        ~Hasher () = default;

        [[nodiscard]] size_t operator () ( Attributes const &item ) const noexcept;
};

size_t Hasher::operator () ( Attributes const &item ) const noexcept
{
    // Hash function is based on Boost implementation:
    // https://www.boost.org/doc/libs/1_55_0/doc/html/hash/reference.html#boost.hash_combine

    size_t hash = 0U;

    auto hashCombine = [ & ] ( uint64_t v ) noexcept
    {
        constexpr size_t magic = 0x9E3779B9U;
        hash ^= _hashServer ( v ) + magic + ( hash << 6U ) + ( hash >> 2U );
    };

    hashCombine ( static_cast<uint64_t> ( item._normal ) | ( static_cast<uint64_t> ( item._position ) << 32U ) );
    hashCombine ( static_cast<uint64_t> ( item._tangentBitangent ) | ( static_cast<uint64_t> ( item._uv ) << 32U ) );

    return hash;
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
    constexpr int faceCorners = 3;

    std::unordered_map<Attributes, android_vulkan::Mesh2Index, Hasher> uniqueMapper {};

    std::vector<android_vulkan::Mesh2Index> indices {};
    auto const indexCount = static_cast<size_t> ( faceCount * faceCorners );
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
        for ( int cornerIdx = 0; cornerIdx < faceCorners; ++cornerIdx )
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
    float const* bMax = bounds._min._data;
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
    IGameSkin *skin = object.GetIGameSkin ();

    if ( exportInCurrentPose | !skin )
    {
        // NOLINTNEXTLINE - downcast.
        return static_cast<IGameMesh &> ( object );
    }

    return *skin->GetInitialPose ();
}

std::optional<std::ofstream> MeshExporter::OpenFile ( HWND parent, MSTR const &path ) noexcept
{
    std::filesystem::path const filePath ( path.data () );

    if ( filePath.has_parent_path () )
    {
        std::filesystem::path const parentDirectory = filePath.parent_path ();

        if ( !std::filesystem::exists ( parentDirectory ) )
        {
            bool const result = CheckResult ( std::filesystem::create_directories ( parentDirectory ),
                parent,
                "Can't create file directory",
                MB_ICONWARNING
            );

            if ( !result )
            {
                return std::nullopt;
            }
        }
    }

    std::ofstream f ( filePath, std::ios::binary );

    if ( !CheckResult ( f.is_open (), parent, "Can't open file", MB_ICONWARNING ) )
        return std::nullopt;

    return std::move ( f );
}

} // namespace avp
