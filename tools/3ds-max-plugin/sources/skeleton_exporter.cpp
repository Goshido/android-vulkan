#include <skeleton_exporter.hpp>
#include <result_checker.hpp>

GX_DISABLE_COMMON_WARNINGS

#include <unordered_map>
#include <IGame/IGameModifier.h>

GX_RESTORE_WARNING_STATE


namespace avp {

namespace {

constexpr float BONE_SCALING_THRESHOLD = 1.0e-3F;

} // end of anonymous namespace

//----------------------------------------------------------------------------------------------------------------------

void SkeletonExporter::Run ( HWND parent, MSTR const &path ) noexcept
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

    IGameSkin* skin = autoRelease1.GetGameObject ().GetIGameSkin ();

    if ( !CheckResult ( skin != nullptr, parent, "Please add skin modifier.", MB_ICONINFORMATION ) )
        return;

    int const boneCount = skin->GetTotalSkinBoneCount ();

    if ( !CheckResult ( boneCount > 0, parent, "Bone count is zero", MB_ICONINFORMATION ) )
        return;

    std::unordered_map<IGameNode*, int32_t> knownBones {};
    std::unordered_set<IGameNode*> unknownParentBones {};

    std::vector<Bone> skeleton {};
    skeleton.reserve ( static_cast<size_t> ( boneCount ) );
    constexpr int32_t undefined = -42;

    for ( int boneIdx = 0; boneIdx < boneCount; ++boneIdx )
    {
        IGameNode* bone = skin->GetIGameBone ( boneIdx, false );
        unknownParentBones.erase ( bone );
        knownBones.emplace ( bone, static_cast<int32_t> ( boneIdx ) );

        IGameNode* parentBone = bone->GetNodeParent ();

        if ( !knownBones.contains ( parentBone ) )
            unknownParentBones.insert ( parentBone );

        skeleton.emplace_back (
            Bone
            {
                ._bone = bone,
                ._parentBone = parentBone,
                ._parentIdx = undefined,
                ._children {}
            }
        );
    }

    Bone* allBones = skeleton.data ();
    std::list<Bone*> rootBones {};

    for ( Bone &bone : skeleton )
    {
        IGameNode* parentBone = bone._parentBone;

        if ( !parentBone || unknownParentBones.contains ( parentBone ) )
        {
            bone._parentIdx = android_vulkan::ROOT_BONE;
            rootBones.push_back ( &bone );
            continue;
        }

        int32_t const parentIdx = knownBones.find ( parentBone )->second;
        allBones[ parentIdx ]._children.push_back ( &bone );
    }

    auto f = OpenFile ( parent, path );

    if ( !f )
        return;

    std::ofstream &file = *f;

    auto const jointSize = static_cast<size_t> ( boneCount * sizeof ( android_vulkan::BoneJoint ) );
    auto const parentSize = static_cast<size_t> ( boneCount * sizeof ( android_vulkan::BoneParent ) );
    auto const nameInfoSize = static_cast<size_t> ( boneCount * sizeof ( android_vulkan::UTF8Offset ) );

    auto const dataSize = jointSize + jointSize + parentSize + nameInfoSize;
    std::vector<uint8_t> data ( dataSize );
    uint8_t* d = data.data ();

    android_vulkan::SkeletonHeader const header
    {
        ._boneCount = static_cast<uint32_t> ( boneCount ),
        ._referenceTransformOffset = sizeof ( android_vulkan::SkeletonHeader ),
        ._inverseBindTransformOffset = sizeof ( android_vulkan::SkeletonHeader ) + jointSize,
        ._parentOffset = sizeof ( android_vulkan::SkeletonHeader ) + jointSize + jointSize,
        ._nameInfoOffset = sizeof ( android_vulkan::SkeletonHeader ) + jointSize + jointSize + parentSize,
    };

    file.write ( reinterpret_cast<char const*> ( &header ), sizeof ( android_vulkan::SkeletonHeader ) );

    WriteInfo writeInfo
    {
        ._referenceTransform = reinterpret_cast<android_vulkan::BoneJoint*> ( d ),
        ._inverseBindTransform = reinterpret_cast<android_vulkan::BoneJoint*> ( d + jointSize ),
        ._parent = reinterpret_cast<android_vulkan::BoneParent*> ( d + jointSize + jointSize ),
        ._nameOffset = reinterpret_cast<android_vulkan::UTF8Offset*> ( d + jointSize + jointSize + parentSize ),
        ._boneIdx = 0,
        ._currentNameOffset = header._nameInfoOffset + nameInfoSize,
        ._names {},
        ._parentWindow = parent,
        ._skin = skin,
        ._uniqueNames {}
    };

    for ( Bone* bone : rootBones )
    {
        if ( !ProcessBone ( *bone, writeInfo, android_vulkan::ROOT_BONE ) )
        {
            return;
        }
    }

    file.write ( reinterpret_cast<char const*> ( d ), static_cast<std::streamsize> ( dataSize ) );

    for ( auto const &name : writeInfo._names )
        file.write ( name.c_str (), static_cast<std::streamsize> ( name.size () + 1U ) );

    MessageBoxA ( parent, "Done.", "android-vulkan", MB_ICONINFORMATION );
}

bool SkeletonExporter::ProcessBone ( Bone &bone, WriteInfo &writeInfo, int32_t parentIdx ) noexcept
{
    MSTR const name = bone._bone->GetName ();

    if ( !CheckResult ( !name.isNull (), writeInfo._parentWindow, "Bone name must not be empty.", MB_ICONINFORMATION ) )
        return false;

    std::string utf8 ( name.ToUTF8 () );
    bool const result = writeInfo._uniqueNames.contains ( utf8 );

    if ( !CheckResult ( !result, writeInfo._parentWindow, "Bone name must be unique.", MB_ICONINFORMATION) )
        return false;

    *writeInfo._nameOffset = writeInfo._currentNameOffset;
    ++writeInfo._nameOffset;

    // Strings must be null terminated.
    constexpr size_t NULL_TERMINATOR_CHARACTER = 1U;
    auto const nameSize = static_cast<android_vulkan::UTF8Offset> ( utf8.size () + NULL_TERMINATOR_CHARACTER );
    writeInfo._currentNameOffset += nameSize;

    writeInfo._names.push_back ( utf8 );
    writeInfo._uniqueNames.insert ( std::move ( utf8 ) );

    *writeInfo._parent = parentIdx;
    ++writeInfo._parent;

    auto const extractTransform = [] ( HWND parentWindow,
        GMatrix &scratch,
        IGameNode &bone,
        IGameSkin &skin 
    ) noexcept -> std::optional<GXMat4 const *> {
        skin.GetInitBoneTM ( &bone, scratch );
        Point3 const scaling = scratch.Scaling ();

        bool const c0 = std::abs ( scaling.x - 1.0F ) < BONE_SCALING_THRESHOLD;
        bool const c1 = std::abs ( scaling.y - 1.0F ) < BONE_SCALING_THRESHOLD;
        bool const c2 = std::abs ( scaling.z - 1.0F ) < BONE_SCALING_THRESHOLD;

        if ( !CheckResult ( c0 & c1 & c2, parentWindow, "Bone scaling has been detected.", MB_ICONINFORMATION ) )
            return std::nullopt;

        return reinterpret_cast<GXMat4 const*> ( &scratch );
    };

    auto const writeBoneJoint = [] ( android_vulkan::BoneJoint &joint, GXMat4 const &transform ) noexcept {
        auto &orientation = *reinterpret_cast<GXQuat*> ( &joint._orientation );
        auto &location = *reinterpret_cast<GXVec3*> ( &joint._location );
        orientation.From ( transform );
        orientation.Normalize ();
        transform.GetW ( location );
    };

    IGameSkin &skin = *writeInfo._skin;
    HWND parentWindow = writeInfo._parentWindow;

    GMatrix worldTransformNative {};
    auto probe = extractTransform ( parentWindow, worldTransformNative, *bone._bone, skin );

    if ( !probe )
        return false;

    GXMat4 const &worldTransform = *probe.value ();

    GXMat4 inverseBindTransform {};
    inverseBindTransform.Inverse ( worldTransform );
    writeBoneJoint ( *writeInfo._inverseBindTransform, inverseBindTransform );
    ++writeInfo._inverseBindTransform;

    android_vulkan::BoneJoint &ref = *writeInfo._referenceTransform;

    if ( !bone._parentBone )
    {
        writeBoneJoint ( ref, worldTransform );
    }
    else
    {
        //Note: X * P = G
        //where:
        //      X - bone reference transform
        //      P - parent bone world transform,
        //      G - bone world transform
        //We know P and G, so:
        //      X * P * P^(-1) = G * P^(-1)
        //=>    X = G * P^(-1)
        GMatrix parentWorldTransformNative {};
        probe = extractTransform ( parentWindow, parentWorldTransformNative, *bone._parentBone, skin );

        if ( !probe )
            return false;

        GXMat4 const &parentWorldTransform = *probe.value ();

        GXMat4 inverseParentTransform {};
        inverseParentTransform.Inverse ( parentWorldTransform );

        GXMat4 referenceTransform {};
        referenceTransform.Multiply ( worldTransform, inverseParentTransform );
        writeBoneJoint ( ref, referenceTransform );
    }

    ++writeInfo._referenceTransform;
    parentIdx = writeInfo._boneIdx++;

    for ( Bone* childBone : bone._children )
    {
        if ( !ProcessBone ( *childBone, writeInfo, parentIdx ) )
        {
            return false;
        }
    }

    return true;
}

} // namespace avp
