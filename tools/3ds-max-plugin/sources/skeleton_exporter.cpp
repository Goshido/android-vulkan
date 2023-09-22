#include <skeleton_exporter.hpp>
#include <result_checker.hpp>

GX_DISABLE_COMMON_WARNINGS

#include <unordered_map>
#include <unordered_set>
#include <IGame/IGameModifier.h>

GX_RESTORE_WARNING_STATE


namespace avp {

void SkeletonExporter::Run ( HWND parent, MSTR const &/*path*/ ) noexcept
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
                ._idx = undefined,
                ._parentBone = parentBone,
                ._parentIdx = undefined,
                ._children {}
            }
        );
    }

    constexpr int32_t rootBoneIdx = -1;
    Bone* allBones = skeleton.data ();
    std::list<Bone*> rootBones {};

    for ( Bone &bone : skeleton )
    {
        IGameNode* parentBone = bone._parentBone;

        if ( !parentBone || unknownParentBones.contains ( parentBone ) )
        {
            bone._parentIdx = rootBoneIdx;
            rootBones.push_back ( &bone );
            continue;
        }

        int32_t const parentIdx = knownBones.find ( parentBone )->second;
        allBones[ parentIdx ]._children.push_back ( &bone );
    }

    int32_t idx = 0;

    for ( Bone* bone : rootBones )
        ProcessBone ( *bone, idx, rootBoneIdx );

    // TODO
    GXVec2 const stop {};
}

void SkeletonExporter::ProcessBone ( Bone &bone, int32_t &idx, int32_t parentIdx ) noexcept
{
    int32_t const boneIdx = idx++;

    bone._idx = boneIdx;
    bone._parentIdx = parentIdx;

    for ( Bone* childBone : bone._children )
    {
        ProcessBone ( *childBone, idx, boneIdx );
    }
}

} // namespace avp
