#ifndef PBR_ANIMATION_PLAYER_NODE_HPP
#define PBR_ANIMATION_PLAYER_NODE_HPP


#include "joint_provider_node.hpp"
#include <animation_track.hpp>

GX_DISABLE_COMMON_WARNINGS

extern "C" {

#include <lua/lstate.h>

} // extern "C"

GX_RESTORE_WARNING_STATE


namespace pbr {

class AnimationPlayerNode final : public JointProviderNode
{
    private:
        using Reference = std::unique_ptr<AnimationPlayerNode>;

    private:
        bool                                                                _hasTrack = false;
        android_vulkan::AnimationTrack                                      _track {};

        float                                                               _interpolation = 0.0F;
        size_t                                                              _nextFrame = 0U;
        size_t                                                              _prevFrame = 0U;
        size_t                                                              _lastFrame = 0U;

        float                                                               _time = 0.0F;
        float                                                               _trackDuration = 1.0F;
        float                                                               _timeToFrame = 0.0F;
        float                                                               _playbackSpeed = 1.0F;

        static std::unordered_map<AnimationPlayerNode const*, Reference>    _nodes;

    public:
        AnimationPlayerNode () = default;

        AnimationPlayerNode ( AnimationPlayerNode const& ) = delete;
        AnimationPlayerNode& operator= ( AnimationPlayerNode const& ) = delete;

        AnimationPlayerNode ( AnimationPlayerNode&& ) = delete;
        AnimationPlayerNode& operator= ( AnimationPlayerNode&& ) = delete;

        ~AnimationPlayerNode () override = default;

        static void Init ( lua_State &vm ) noexcept;
        static void Destroy () noexcept;

    private:
        [[nodiscard]] Result GetJoint ( std::string const &name ) noexcept override;
        void Update ( float deltaTime ) noexcept override;

        [[nodiscard]] bool LoadAnimation ( std::string &&animationTrack ) noexcept;

        [[nodiscard]] static int OnCreate ( lua_State* state );
        [[nodiscard]] static int OnDestroy ( lua_State* state );
        [[nodiscard]] static int OnLoadAnimation ( lua_State* state );
        [[nodiscard]] static int OnSetPlaybackSpeed ( lua_State* state );
};

} // namespace pbr


#endif // PBR_ANIMATION_PLAYER_NODE_HPP
