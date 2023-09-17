#include <skin_exporter.hpp>
#include <result_checker.hpp>
#include <GXCommon/GXMath.hpp>

GX_DISABLE_COMMON_WARNINGS

#include <unordered_map>
#include <unordered_set>
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

#include <skin.hpp>


namespace avp {

namespace {

class SkinBuilder final
{
    private:
        std::vector<android_vulkan::SkinInfluence>      _influences {};

    public:
        SkinBuilder () noexcept;

        SkinBuilder ( SkinBuilder const & ) = delete;
        SkinBuilder &operator = ( SkinBuilder const & ) = delete;

        SkinBuilder ( SkinBuilder && ) = delete;
        SkinBuilder &operator = ( SkinBuilder && ) = delete;

        ~SkinBuilder () = default;

        void Add ( android_vulkan::SkinInfluence influence ) noexcept;
        void Clear () noexcept;
        [[nodiscard]] std::optional<android_vulkan::SkinVertex> Compute ( HWND parent ) noexcept;
};

SkinBuilder::SkinBuilder () noexcept
{
    _influences.reserve ( android_vulkan::BONES_PER_VERTEX );
}

void SkinBuilder::Add ( android_vulkan::SkinInfluence influence ) noexcept
{
    _influences.push_back ( influence );
}

void SkinBuilder::Clear () noexcept
{
    _influences.clear ();
}

std::optional<android_vulkan::SkinVertex> SkinBuilder::Compute ( HWND parent ) noexcept
{
    if ( _influences.size () > android_vulkan::BONES_PER_VERTEX )
    {
        std::sort ( _influences.begin (),
            _influences.end (),

            [] ( android_vulkan::SkinInfluence const &a, android_vulkan::SkinInfluence const &b ) noexcept -> bool {
                return a._boneWeight > b._boneWeight;
            }
        );

        _influences.resize ( android_vulkan::BONES_PER_VERTEX );
    }

    auto const end = _influences.cend ();
    auto it = _influences.cbegin ();
    float sum = it->_boneWeight;

    for ( ++it; it != end; ++it )
        sum += it->_boneWeight;

    constexpr float badWeightThreshold = 1.0e-4F;

    if ( !CheckResult ( sum > badWeightThreshold, parent, "Weights are too small.", MB_ICONINFORMATION ) )
        return std::nullopt;

    float const normFactor = 1.0F / sum;

    for ( auto &influence : _influences )
        influence._boneWeight *= normFactor;

    android_vulkan::SkinVertex result {};
    size_t const weightCount = _influences.size ();

    for ( size_t i = 0U; i < weightCount; ++i )
        result._influences[ i ] = _influences[ i ];

    uint32_t const whateverBone = _influences.front ()._boneIndex;

    for ( size_t i = weightCount; i < android_vulkan::BONES_PER_VERTEX; ++i )
    {
        result._influences[ i ] =
        {
            ._boneIndex = whateverBone,
            ._boneWeight = 0.0F
        };
    }

    return std::move ( result );
}

} // end of anonymous namespace

//----------------------------------------------------------------------------------------------------------------------

void SkinExporter::Run ( HWND parent, MSTR const &path ) noexcept
{
    Interface7 &core = *GetCOREInterface7 ();

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

    IGameObject &object = autoRelease1.GetGameObject ();
    IGameSkin* skin = object.GetIGameSkin ();

    if ( !CheckResult ( skin != nullptr, parent, "Please add skin modifier.", MB_ICONINFORMATION ) )
        return;

    // NOLINTNEXTLINE - downcast.
    IGameMesh &mesh = static_cast<IGameMesh &> ( object );
    Tab<int> mapper = mesh.GetActiveMapChannelNum ();

    if ( !CheckResult ( mapper.Count () > 0, parent, "Please add UV map to mesh.", MB_ICONINFORMATION ) )
        return;

    int const uvChannel = mapper[ 0 ];
    int const faceCount = mesh.GetNumberOfFaces ();
    auto const indexCount = static_cast<size_t> ( faceCount * FACE_CORNERS );

    GXAABB bounds {};
    SkinBuilder skinBuilder {};

    // Estimation from top.
    std::vector<android_vulkan::SkinVertex> skinVertices {};
    skinVertices.reserve ( indexCount );

    std::unordered_map<IGameNode*, int> boneMap {};
    std::unordered_set<Attributes, Attributes::Hasher> uniqueMapper {};

    for ( int faceIdx = 0; faceIdx < faceCount; ++faceIdx )
    {
        for ( int cornerIdx = 0; cornerIdx < FACE_CORNERS; ++cornerIdx )
        {
            int const positionIndex = mesh.GetFaceVertex ( faceIdx, cornerIdx );

            Attributes const attributes
            {
                ._normal = mesh.GetFaceVertexNormal ( faceIdx, cornerIdx ),
                ._position = positionIndex,
                ._tangentBitangent = mesh.GetFaceVertexTangentBinormal ( faceIdx, cornerIdx, uvChannel ),
                ._uv = mesh.GetFaceTextureVertex ( faceIdx, cornerIdx, uvChannel )
            };

            if ( auto const findResult = uniqueMapper.find ( attributes ); findResult != uniqueMapper.cend () )
                continue;

            int const boneCount = skin->GetNumberOfBones ( positionIndex );

            if ( !CheckResult ( boneCount > 0, parent, "Vertex is not influenced any bone.", MB_ICONINFORMATION ) )
                return;

            skinBuilder.Clear ();

            for ( int boneIdx = 0; boneIdx < boneCount; ++boneIdx )
            {
                IGameNode* bone = skin->GetIGameBone ( positionIndex, boneIdx );
                int targetBoneIndex;

                if ( auto const findResult = boneMap.find ( bone ); findResult != boneMap.cend () )
                {
                    targetBoneIndex = findResult->second;
                }
                else
                {
                    targetBoneIndex = skin->GetBoneIndex ( bone, true );
                    boneMap.emplace ( bone, targetBoneIndex );
                }

                skinBuilder.Add (
                    android_vulkan::SkinInfluence
                    {
                        ._boneIndex = static_cast<uint32_t> ( targetBoneIndex ),
                        ._boneWeight = skin->GetWeight ( positionIndex, boneIdx )
                    }
                );
            }

            auto skinVertex = skinBuilder.Compute ( parent );

            if ( !skinVertex )
                return;

            Point3 const p = mesh.GetVertex ( positionIndex, true );
            bounds.AddVertex ( p.x, p.y, p.z );

            uniqueMapper.emplace ( attributes );
            skinVertices.push_back ( std::move ( *skinVertex ) );
        }
    }

    auto file = OpenFile ( parent, path );

    if ( !file )
        return;

    std::ofstream &f = *file;
    (void)f;

    // TODO
}

} // namespace avp
