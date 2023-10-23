#include <animation_exporter.hpp>
#include <result_checker.hpp>
#include <android_vulkan_sdk/animation.hpp>

GX_DISABLE_COMMON_WARNINGS

#include <IGame/IGameModifier.h>

GX_RESTORE_WARNING_STATE


namespace avp {

void AnimationExporter::Run ( HWND parent, MSTR const &path, int startFrame, int lastFrame ) noexcept
{
    bool const c0 = startFrame >= 0;
    bool const c1 = lastFrame > 0;

    if ( !CheckResult ( c0 & c1, parent, "Frame indices should be non negative.", MB_ICONINFORMATION ) )
        return;

    if ( !CheckResult ( startFrame < lastFrame, parent, "Start frame is bigger than last frame.", MB_ICONINFORMATION ) )
        return;

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

    auto const frameCount = static_cast<uint32_t> ( lastFrame - startFrame + 1 );
    auto const nameInfoSize = static_cast<size_t> ( boneCount * sizeof ( android_vulkan::UTF8Offset ) );

    auto const animationDataSize =
        static_cast<size_t> ( frameCount * boneCount * sizeof ( android_vulkan::BoneJoint ) );

    android_vulkan::AnimationHeader const header
    {
        ._fps = static_cast<float> ( GetFrameRate () ),
        ._boneCount = static_cast<uint32_t> ( boneCount ),
        ._frameCount = frameCount,
        ._animationDataOffset = static_cast<uint64_t> ( sizeof ( android_vulkan::AnimationHeader ) ),
        ._boneNameInfoOffset = static_cast<uint64_t> ( sizeof ( android_vulkan::AnimationHeader ) + animationDataSize )
    };

    size_t const dataSize = animationDataSize + nameInfoSize;
    std::vector<uint8_t> data ( dataSize );
    uint8_t* d = data.data ();
    auto* nameInfoOffset = reinterpret_cast<android_vulkan::UTF8Offset*> ( d + animationDataSize );
    android_vulkan::UTF8Offset currentNameOffset = header._boneNameInfoOffset + nameInfoSize;

    BoneCollector boneCollector ( parent );

    for ( int boneIdx = 0; boneIdx < boneCount; ++boneIdx )
    {
        IGameNode* bone = skin->GetIGameBone ( boneIdx, false );

        if ( !boneCollector.AddBone ( *bone ) )
            return;

        *nameInfoOffset = currentNameOffset;
        ++nameInfoOffset;
        currentNameOffset += boneCollector.GetLastNameSize ();
    }

    auto* joint = reinterpret_cast<android_vulkan::BoneJoint*> ( d );
    int const ticksPerFrame = GetTicksPerFrame ();

    for ( int frameIdx = startFrame; frameIdx <= lastFrame; ++frameIdx )
    {
        TimeValue const time = static_cast<TimeValue> ( frameIdx * ticksPerFrame );

        for ( int boneIdx = 0; boneIdx < boneCount; ++boneIdx )
        {
            IGameNode* bone = skin->GetIGameBone ( boneIdx, false );
            GMatrix const worldTransformNative = bone->GetObjectTM ( time );
            auto probe = ExtractTransform ( parent, worldTransformNative );

            if ( !probe )
                return;

            GXMat4 const &worldTransform = *probe.value ();
            IGameNode* parentBone = bone->GetNodeParent ();

            if ( !parentBone )
            {
                WriteBoneJoint ( *joint, worldTransform );
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
                GMatrix const parentWorldTransformNative = parentBone->GetObjectTM ( time );
                probe = ExtractTransform ( parent, parentWorldTransformNative );

                if ( !probe )
                    return;

                GXMat4 const &parentWorldTransform = *probe.value ();

                GXMat4 inverseParentTransform {};
                inverseParentTransform.Inverse ( parentWorldTransform );

                GXMat4 poseTransform {};
                poseTransform.Multiply ( worldTransform, inverseParentTransform );
                WriteBoneJoint ( *joint, poseTransform );
            }

            ++joint;
        }
    }

    auto f = OpenFile ( parent, path );

    if ( !f )
        return;

    std::ofstream &file = *f;
    file.write ( reinterpret_cast<char const*> ( &header ), sizeof ( header ) );
    file.write ( reinterpret_cast<char const*> ( d ), static_cast<std::streamsize> ( dataSize ) );

    for ( auto const &name : boneCollector._names )
        file.write ( name.c_str (), static_cast<std::streamsize> ( name.size () + 1U ) );

    MessageBoxA ( parent, "Done.", "android-vulkan", MB_ICONINFORMATION );
}

} // namespace avp
