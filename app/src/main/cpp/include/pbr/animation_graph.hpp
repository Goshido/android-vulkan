#ifndef PBR_ANIMATION_GRAPH_HPP
#define PBR_ANIMATION_GRAPH_HPP


#include "joint_provider_node.hpp"
#include "node_link.hpp"
#include <android_vulkan_sdk/bone_joint.hpp>

GX_DISABLE_COMMON_WARNINGS

extern "C" {

#include <lua/lstate.h>

} // extern "C"

#include <string>
#include <unordered_map>

GX_RESTORE_WARNING_STATE


namespace pbr {

class AnimationGraph final : public NodeLink
{
    public:
        // Making sure that array will be tightly packed. It's important for upload to GPU side.
        using Joints = std::vector<android_vulkan::BoneJoint>;

    private:
        using Reference = std::unique_ptr<AnimationGraph>;

    private:
        JointProviderNode*                                              _inputNode = nullptr;
        Joints                                                          _inverseBindTransforms {};
        bool                                                            _isSleep = true;
        std::vector<std::string>                                        _names {};
        Joints                                                          _referenceTransforms {};
        std::vector<int32_t>                                            _parents {};
        Joints                                                          _poseLocal {};
        Joints                                                          _poseGlobal {};
        Joints                                                          _poseSkin {};

        static std::unordered_map<AnimationGraph const*, Reference>     _graphs;

    public:
        AnimationGraph () = delete;

        AnimationGraph ( AnimationGraph const& ) = delete;
        AnimationGraph& operator= ( AnimationGraph const& ) = delete;

        AnimationGraph ( AnimationGraph&& ) = delete;
        AnimationGraph& operator= ( AnimationGraph&& ) = delete;

        explicit AnimationGraph ( bool &success, std::string &&skeletonFile ) noexcept;

        ~AnimationGraph () = default;

        [[maybe_unused, nodiscard]] Joints const &GetPose () const noexcept;

        static void Init ( lua_State &vm ) noexcept;
        static void Destroy () noexcept;
        static void Update ( float deltaTime ) noexcept;

    private:
        void UpdateInternal ( float deltaTime ) noexcept;

        [[nodiscard]] static int OnAwake ( lua_State* state );
        [[nodiscard]] static int OnCreate ( lua_State* state );
        [[nodiscard]] static int OnDestroy ( lua_State* state );
        [[nodiscard]] static int OnSetInput ( lua_State* state );
        [[nodiscard]] static int OnSleep ( lua_State* state );
};

} // namespace pbr


#endif // PBR_ANIMATION_GRAPH_HPP
