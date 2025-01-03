#include <precompiled_headers.hpp>
#include <skeleton_exporter.hpp>
#include <result_checker.hpp>


namespace avp {

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

    file.write ( reinterpret_cast<char const*> ( &header ), sizeof ( header ) );

    WriteInfo writeInfo
    {
        ._referenceTransform = reinterpret_cast<android_vulkan::BoneJoint*> ( d ),
        ._inverseBindTransform = reinterpret_cast<android_vulkan::BoneJoint*> ( d + jointSize ),
        ._parent = reinterpret_cast<android_vulkan::BoneParent*> ( d + jointSize + jointSize ),
        ._nameOffset = reinterpret_cast<android_vulkan::UTF8Offset*> ( d + jointSize + jointSize + parentSize ),
        ._boneCollector { parent },
        ._boneIdx = 0,
        ._currentNameOffset = header._nameInfoOffset + nameInfoSize,
        ._parentWindow = parent,
        ._skin = skin
    };

    for ( Bone* bone : rootBones )
    {
        if ( !ProcessBone ( *bone, writeInfo, android_vulkan::ROOT_BONE ) )
        {
            return;
        }
    }

    file.write ( reinterpret_cast<char const*> ( d ), static_cast<std::streamsize> ( dataSize ) );

    for ( auto const &name : writeInfo._boneCollector._names )
        file.write ( name.c_str (), static_cast<std::streamsize> ( name.size () + 1U ) );

    MessageBoxA ( parent, "Done.", "android-vulkan", MB_ICONINFORMATION );
}

bool SkeletonExporter::ProcessBone ( Bone &bone, WriteInfo &writeInfo, int32_t parentIdx ) noexcept
{
    BoneCollector &boneCollector = writeInfo._boneCollector;

    if ( !boneCollector.AddBone ( *bone._bone ) )
        return false;

    *writeInfo._nameOffset = writeInfo._currentNameOffset;
    ++writeInfo._nameOffset;
    writeInfo._currentNameOffset += boneCollector.GetLastNameSize ();

    *writeInfo._parent = parentIdx;
    ++writeInfo._parent;

    IGameSkin &skin = *writeInfo._skin;
    HWND parentWindow = writeInfo._parentWindow;

    GMatrix worldTransformNative {};
    skin.GetInitBoneTM ( bone._bone, worldTransformNative );
    auto probe = ExtractTransform ( parentWindow, worldTransformNative );

    if ( !probe )
        return false;

    GXMat4 const &worldTransform = *probe.value ();

    GXMat4 inverseBindTransform {};
    inverseBindTransform.Inverse ( worldTransform );
    WriteBoneJoint ( *writeInfo._inverseBindTransform, inverseBindTransform );
    ++writeInfo._inverseBindTransform;

    android_vulkan::BoneJoint &ref = *writeInfo._referenceTransform;

    if ( !bone._parentBone )
    {
        WriteBoneJoint ( ref, worldTransform );
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
        skin.GetInitBoneTM ( bone._parentBone, parentWorldTransformNative );
        probe = ExtractTransform ( parentWindow, parentWorldTransformNative );

        if ( !probe )
            return false;

        GXMat4 const &parentWorldTransform = *probe.value ();

        GXMat4 inverseParentTransform {};
        inverseParentTransform.Inverse ( parentWorldTransform );

        GXMat4 referenceTransform {};
        referenceTransform.Multiply ( worldTransform, inverseParentTransform );
        WriteBoneJoint ( ref, referenceTransform );
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
