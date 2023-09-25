#include <animation_exporter.hpp>
#include <result_checker.hpp>
#include <GXCommon/GXMath.hpp>

GX_DISABLE_COMMON_WARNINGS

#include <IGame/IGameModifier.h>

GX_RESTORE_WARNING_STATE


namespace avp {

void AnimationExporter::Run ( HWND parent, MSTR const &/*path*/, int /*startFrame*/, int /*lastFrame*/ ) noexcept
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

    int const boneCount = skin->GetTotalBoneCount ();

    if ( !CheckResult ( boneCount > 0, parent, "Bone count is zero", MB_ICONINFORMATION ) )
        return;

    BoneCollector boneCollector ( parent );

    for ( int i = 0; i < boneCount; ++i )
    {
        IGameNode* bone = skin->GetIGameBone ( i, true );
        IGameNode* parentBone = bone->GetNodeParent ();
        (void)parentBone;
        (void)bone;

        // TODO
    }

    // TODO
    MessageBoxA ( parent, "Done.", "android-vulkan", MB_ICONINFORMATION );
}

} // namespace avp
