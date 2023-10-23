#ifndef ANDROID_VULKAN_ANIMATION_TRACK_HPP
#define ANDROID_VULKAN_ANIMATION_TRACK_HPP


#include "joint.hpp"

GX_DISABLE_COMMON_WARNINGS

#include <string>
#include <unordered_map>
#include <vector>

GX_RESTORE_WARNING_STATE


namespace android_vulkan {

class AnimationTrack final
{
    private:
        size_t                                      _frameCount = 0U;
        std::vector<Joint>                          _frameData {};
        float                                       _fps = 60.0F;
        std::unordered_map<std::string, size_t>     _mapper {};

    public:
        AnimationTrack () = default;

        AnimationTrack ( AnimationTrack const& ) = delete;
        AnimationTrack& operator= ( AnimationTrack const& ) = delete;

        AnimationTrack ( AnimationTrack&& ) = delete;
        AnimationTrack& operator= ( AnimationTrack&& ) = delete;

        ~AnimationTrack () = default;

        [[nodiscard]] float GetFPS () const noexcept;
        [[nodiscard]] size_t GetFrameCount () const noexcept;
        [[nodiscard]] Joint const &GetJoint ( std::string const &bone, size_t frame ) const noexcept;
        [[nodiscard]] bool HasBone ( std::string const &bone ) const noexcept;
        [[nodiscard]] bool Load ( std::string &&file ) noexcept;
};

} // namespace android_vulkan


#endif // ANDROID_VULKAN_ANIMATION_TRACK_HPP
