#include <bone_data_exporter.hpp>
#include <result_checker.hpp>

GX_DISABLE_COMMON_WARNINGS

#include <IGame/IGameModifier.h>

GX_RESTORE_WARNING_STATE


namespace avp {

namespace {

constexpr float BONE_SCALING_THRESHOLD = 1.0e-3F;

} // end of anonymous namespace

//----------------------------------------------------------------------------------------------------------------------

BoneDataExporter::BoneCollector::BoneCollector ( HWND parent ) noexcept:
    _parent ( parent )
{
    // NOTHING
}

bool BoneDataExporter::BoneCollector::AddBone ( IGameNode &bone ) noexcept
{
    MSTR const name = bone.GetName ();

    if ( !CheckResult ( !name.isNull (), _parent, "Bone name must not be empty.", MB_ICONINFORMATION ) )
        return false;

    std::string utf8 ( name.ToUTF8 () );

    if ( !CheckResult ( !_uniqueNames.contains ( utf8 ), _parent, "Bone name must be unique.", MB_ICONINFORMATION) )
        return false;

    _names.push_back ( utf8 );
    _uniqueNames.insert ( std::move ( utf8 ) );
    return true;
}

android_vulkan::UTF8Offset BoneDataExporter::BoneCollector::GetLastNameSize () const noexcept
{
    // Strings must be null terminated.
    constexpr android_vulkan::UTF8Offset NULL_TERMINATOR_CHARACTER = 1U;
    return static_cast<android_vulkan::UTF8Offset> ( _names.back ().size () + NULL_TERMINATOR_CHARACTER );
}

//----------------------------------------------------------------------------------------------------------------------

std::optional<GXMat4 const*> BoneDataExporter::ExtractTransform ( HWND parentWindow, GMatrix const &scratch ) noexcept
{
    Point3 const scaling = scratch.Scaling ();

    bool const c0 = std::abs ( scaling.x - 1.0F ) < BONE_SCALING_THRESHOLD;
    bool const c1 = std::abs ( scaling.y - 1.0F ) < BONE_SCALING_THRESHOLD;
    bool const c2 = std::abs ( scaling.z - 1.0F ) < BONE_SCALING_THRESHOLD;

    if ( !CheckResult ( c0 & c1 & c2, parentWindow, "Bone scaling has been detected.", MB_ICONINFORMATION ) )
        return std::nullopt;

    return reinterpret_cast<GXMat4 const*> ( &scratch );
};

void BoneDataExporter::WriteBoneJoint ( android_vulkan::BoneJoint &joint, GXMat4 const &transform ) noexcept
{
    auto &orientation = *reinterpret_cast<GXQuat*> ( &joint._orientation );
    auto &location = *reinterpret_cast<GXVec3*> ( &joint._location );
    orientation.From ( transform );
    orientation.Normalize ();
    transform.GetW ( location );
};

} // namespace avp
