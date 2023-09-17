#include <exporter.hpp>
#include <result_checker.hpp>

GX_DISABLE_COMMON_WARNINGS

#include <filesystem>
#include <IGame/IGameModifier.h>

GX_RESTORE_WARNING_STATE


namespace avp {

size_t Exporter::Attributes::Hasher::operator () ( Attributes const &item ) const noexcept
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

//----------------------------------------------------------------------------------------------------------------------

bool Exporter::Attributes::operator == ( Exporter::Attributes const &other ) const noexcept
{
    bool const c0 = _normal == other._normal;
    bool const c1 = _position == other._position;
    bool const c2 = _tangentBitangent == other._tangentBitangent;
    bool const c3 = _uv == other._uv;
    return c0 & c1 & c2 & c3;
}

//----------------------------------------------------------------------------------------------------------------------

Exporter::AutoReleaseIGameScene::~AutoReleaseIGameScene () noexcept
{
    if ( _scene )
    {
        _scene->ReleaseIGame ();
    }
}

IGameScene &Exporter::AutoReleaseIGameScene::GetScene () noexcept
{
    return *_scene;
}

bool Exporter::AutoReleaseIGameScene::Init ( HWND parent ) noexcept
{
    _scene = GetIGameInterface ();

    bool const result = CheckResult ( _scene != nullptr, parent, "Can't get IGameScene.", MB_ICONWARNING ) &&
        CheckResult ( _scene->InitialiseIGame ( false ), parent, "Can't init IGameScene.", MB_ICONWARNING );

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

Exporter::AutoReleaseIGameNode::~AutoReleaseIGameNode () noexcept
{
    if ( _object )
    {
        _node->ReleaseIGameObject ();
    }
}

IGameObject &Exporter::AutoReleaseIGameNode::GetGameObject () noexcept
{
    return *_object;
}

bool Exporter::AutoReleaseIGameNode::Init ( HWND parent, IGameScene &scene, INode &node ) noexcept
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

std::optional<std::ofstream> Exporter::OpenFile ( HWND parent, MSTR const &path ) noexcept
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
