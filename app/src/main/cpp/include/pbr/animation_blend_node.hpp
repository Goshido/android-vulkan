#ifndef PBR_ANIMATION_BLEND_NODE_HPP
#define PBR_ANIMATION_BLEND_NODE_HPP


#include "joint_provider_node.hpp"

GX_DISABLE_COMMON_WARNINGS

extern "C" {

#include <lua/lstate.h>

} // extern "C"

GX_RESTORE_WARNING_STATE


namespace pbr {

class AnimationBlendNode final : public JointProviderNode
{
    private:
        using Reference = std::unique_ptr<AnimationBlendNode>;

    private:
        float                                                               _blendFactor = 0.0F;
        JointProviderNode*                                                  _inputA = nullptr;
        JointProviderNode*                                                  _inputB = nullptr;

        static std::unordered_map<AnimationBlendNode const*, Reference>     _abNodes;

    public:
        AnimationBlendNode () = default;
    
        AnimationBlendNode ( AnimationBlendNode const& ) = delete;
        AnimationBlendNode& operator= ( AnimationBlendNode const& ) = delete;
    
        AnimationBlendNode ( AnimationBlendNode&& ) = delete;
        AnimationBlendNode& operator= ( AnimationBlendNode&& ) = delete;
    
        ~AnimationBlendNode () override = default;

        static void Init ( lua_State &vm ) noexcept;
        static void Destroy () noexcept;

    private:
        [[nodiscard]] Result GetJoint ( std::string const &name ) noexcept override;
        void Update ( lua_State &vm, float deltaTime ) noexcept override;

        [[nodiscard]] static int OnCreate ( lua_State* state );
        [[nodiscard]] static int OnDestroy ( lua_State* state );

        [[nodiscard]] static int OnSetBlendFactor ( lua_State* state );
        [[nodiscard]] static int OnSetInputA ( lua_State* state );
        [[nodiscard]] static int OnSetInputB ( lua_State* state );
};

} // namespace pbr


#endif // PBR_ANIMATION_BLEND_NODE_HPP
