#include <mesh_exporter.hpp>

GX_DISABLE_COMMON_WARNINGS

#include <maxapi.h>
#include <IGame/IGame.h>
#include <IGame/IGameModifier.h>

GX_RESTORE_WARNING_STATE


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
        CheckResult ( _scene->InitialiseIGame (), parent, "Can't init IGameScene.", MB_ICONWARNING );

    if ( !result )
        return false;

    IGameConversionManager &conventions = *GetConversionManager ();

    conventions.SetUserCoordSystem (
        UserCoord
        {
            .rotation = 0,
            .xAxis = 1,
            .yAxis = 2,
            .zAxis = 5,
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

} // end of anonymous namespace

//----------------------------------------------------------------------------------------------------------------------

MeshExporter::MeshExporter ( HWND parent, MSTR const &/*path*/, bool exportInCurrentPose ) noexcept:
    _parent ( parent )
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
    PumpLowLevelData ( mesh, uvChannel );
}

void MeshExporter::PumpLowLevelData ( IGameMesh &mesh, int uvChannel ) noexcept
{
    int count = mesh.GetNumberOfVerts ();
    _positions.resize ( static_cast<size_t> ( count ) );
    Point3* v3 = _positions.data ();

    for ( int i = 0; i < count; ++i )
        mesh.GetVertex ( i, v3[ i ], true );

    count = mesh.GetNumberOfNormals ();
    _normals.resize ( static_cast<size_t> ( count ) );
    v3 = _normals.data ();

    for ( int i = 0; i < count; ++i )
        mesh.GetNormal ( i, v3[ i ], true );

    count = mesh.GetNumberOfTangents ( uvChannel );
    _tangents.resize ( static_cast<size_t> ( count ) );
    v3 = _tangents.data ();

    for ( int i = 0; i < count; ++i )
        mesh.GetTangent ( i, v3[ i ], true );

    count = mesh.GetNumberOfBinormals ( uvChannel );
    _bitangents.resize ( static_cast<size_t> ( count ) );
    v3 = _bitangents.data ();

    for ( int i = 0; i < count; ++i )
        mesh.GetBinormal ( i, v3[ i ], true );

    count = mesh.GetNumberOfMapVerts ( uvChannel );
    _uvs.resize ( static_cast<size_t> ( count ) );
    Point2* v2 = _uvs.data ();

    for ( int i = 0; i < count; ++i )
    {
        Point3 const uvw = mesh.GetMapVertex ( uvChannel, i );
        Point2 &target = v2[ i ];
        target.x = uvw.x;
        target.y = uvw.y;
    }
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

} // namespace avp
