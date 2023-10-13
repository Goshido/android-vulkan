#include <animation_track.hpp>
#include <file.hpp>
#include <android_vulkan_sdk/animation.hpp>
#include <android_vulkan_sdk/bone_joint.hpp>


namespace android_vulkan {

[[maybe_unused]] float AnimationTrack::GetFPS () const noexcept
{
    return _fps;
}

[[maybe_unused]] size_t AnimationTrack::GetFrames () const noexcept
{
    return _frames.size ();
}

[[maybe_unused]] Joint const &AnimationTrack::GetJoint ( std::string const &/*bone*/, size_t /*frame*/ ) const noexcept
{
    // TODO
    static Joint const dummy{};
    return dummy;
}

[[maybe_unused]] bool AnimationTrack::HasBone ( std::string const &bone ) const noexcept
{
    return _mapper.contains ( bone );
}

[[maybe_unused]] bool AnimationTrack::Load ( std::string &&file ) noexcept
{
    File animationFile ( std::move ( file ) );

    if ( !animationFile.LoadContent () )
        return false;

    uint8_t const* content = animationFile.GetContent ().data ();
    auto const &header = *reinterpret_cast<android_vulkan::AnimationHeader const*> ( content );

    _fps = header._fps;
    uint32_t const frameCount = header._frameCount;
    uint32_t const boneCount = header._boneCount;
    _frames.reserve ( static_cast<size_t> ( boneCount ) * static_cast<size_t> ( frameCount ) );

    auto const* frame = reinterpret_cast<android_vulkan::BoneJoint const*> ( content + header._animationDataOffset );
    (void)frame;
    // TODO

    auto const* name = reinterpret_cast<android_vulkan::UTF8Offset const*> ( content + header._boneNameInfoOffset );
    (void)name;
    // TODO

    return true;
}


} // namespace android_vulkan
