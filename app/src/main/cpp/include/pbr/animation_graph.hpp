#ifndef PBR_ANIMATION_GRAPH_HPP
#define PBR_ANIMATION_GRAPH_HPP


#include <android_vulkan_sdk/bone_joint.hpp>

GX_DISABLE_COMMON_WARNINGS

extern "C" {

#include <lua/lstate.h>

} // extern "C"

#include <string>
#include <unordered_map>

GX_RESTORE_WARNING_STATE


namespace pbr {

class AnimationGraph final
{
    private:
        using Reference = std::unique_ptr<AnimationGraph>;
        using Joints = std::vector<android_vulkan::BoneJoint>;

    private:
        Joints                                                          _inverseBindTransforms {};
        [[maybe_unused]] bool                                           _isSleep = true;
        std::vector<std::string>                                        _names {};
        Joints                                                          _referenceTransforms {};
        std::vector<int32_t>                                            _parents {};
        Joints                                                          _pose {};

        static std::unordered_map<AnimationGraph const*, Reference>     _graphs;

    public:
        AnimationGraph () = delete;

        AnimationGraph ( AnimationGraph const& ) = delete;
        AnimationGraph& operator= ( AnimationGraph const& ) = delete;

        AnimationGraph ( AnimationGraph&& ) = delete;
        AnimationGraph& operator= ( AnimationGraph&& ) = delete;

        explicit AnimationGraph ( bool &success, std::string &&skeletonFile ) noexcept;

        ~AnimationGraph () = default;

        static void Init ( lua_State &vm ) noexcept;
        static void Destroy () noexcept;

    private:
        [[nodiscard]] static int OnAwake ( lua_State* state );
        [[nodiscard]] static int OnCreate ( lua_State* state );
        [[nodiscard]] static int OnDestroy ( lua_State* state );
        [[nodiscard]] static int OnSetInput ( lua_State* state );
        [[nodiscard]] static int OnSleep ( lua_State* state );
};

} // namespace pbr


#endif // PBR_ANIMATION_GRAPH_HPP
