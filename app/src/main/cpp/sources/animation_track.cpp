#include <animation_track.hpp>
#include <file.hpp>
#include <android_vulkan_sdk/animation.hpp>
#include <android_vulkan_sdk/bone_joint.hpp>


namespace android_vulkan {

float AnimationTrack::GetFPS () const noexcept
{
    return _fps;
}

size_t AnimationTrack::GetFrameCount () const noexcept
{
    return _frameCount;
}

Joint const &AnimationTrack::GetJoint ( std::string const &bone, size_t frame ) const noexcept
{
    size_t const bonesPerFrame = _mapper.size ();
    size_t const boneIdx = _mapper.find ( bone )->second;
    return _frameData[ bonesPerFrame * frame + boneIdx ];
}

bool AnimationTrack::HasBone ( std::string const &bone ) const noexcept
{
    return _mapper.contains ( bone );
}

bool AnimationTrack::Load ( std::string &&file ) noexcept
{
    File animationFile ( std::move ( file ) );

    if ( !animationFile.LoadContent () )
        return false;

    uint8_t const* content = animationFile.GetContent ().data ();
    auto const &header = *reinterpret_cast<android_vulkan::AnimationHeader const*> ( content );

    _fps = header._fps;
    _frameCount = static_cast<size_t> ( header._frameCount );

    uint32_t const frameCount = header._frameCount;
    uint32_t const boneCount = header._boneCount;
    _frameData.reserve ( static_cast<size_t> ( boneCount ) * static_cast<size_t> ( frameCount ) );

    auto const* frame = reinterpret_cast<android_vulkan::BoneJoint const*> ( content + header._animationDataOffset );

    for ( uint32_t frameIdx = 0U; frameIdx < frameCount; ++frameIdx )
    {
        for ( uint32_t boneIdx = 0U; boneIdx < boneCount; ++boneIdx )
        {
            Joint j {};
            std::memcpy ( &j._location, &frame->_location, sizeof ( android_vulkan::Vec3 ) );
            std::memcpy ( &j._orientation, &frame->_orientation, sizeof ( android_vulkan::Quat ) );
            _frameData.push_back ( j );
            ++frame;
        }
    }

    auto const* name = reinterpret_cast<android_vulkan::UTF8Offset const*> ( content + header._boneNameInfoOffset );

    for ( uint32_t i = 0U; i < boneCount; ++i )
    {
        _mapper.emplace ( reinterpret_cast<char const*> ( content + *name ), i );
        ++name;
    }

    return true;
}


} // namespace android_vulkan
