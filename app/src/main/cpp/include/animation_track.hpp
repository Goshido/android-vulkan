#ifndef ANDROID_VULKAN_ANIMATION_TRACK_HPP
#define ANDROID_VULKAN_ANIMATION_TRACK_HPP


#include "joint.hpp"

GX_DISABLE_COMMON_WARNINGS

#include <string>
#include <unordered_map>
#include <vector>

GX_RESTORE_WARNING_STATE


namespace android_vulkan {

class [[maybe_unused]] AnimationTrack final
{
    private:
        std::vector<Joint>                          _frames {};
        float                                       _fps = 60.0F;
        std::unordered_map<std::string, size_t>     _mapper {};

    public:
        AnimationTrack () = default;

        AnimationTrack ( AnimationTrack const& ) = delete;
        AnimationTrack& operator= ( AnimationTrack const& ) = delete;

        AnimationTrack ( AnimationTrack&& ) = delete;
        AnimationTrack& operator= ( AnimationTrack&& ) = delete;

        ~AnimationTrack () = default;

        [[maybe_unused, nodiscard]] float GetFPS () const noexcept;
        [[maybe_unused, nodiscard]] size_t GetFrames () const noexcept;
        [[maybe_unused, nodiscard]] Joint const &GetJoint ( std::string const &bone, size_t frame ) const noexcept;
        [[maybe_unused, nodiscard]] bool HasBone ( std::string const &bone ) const noexcept;
        [[maybe_unused, nodiscard]] bool Load ( std::string &&file ) noexcept;
};

} // namespace android_vulkan


#endif // ANDROID_VULKAN_ANIMATION_TRACK_HPP
