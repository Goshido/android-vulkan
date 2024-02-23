#include <av_assert.hpp>
#include <logger.hpp>
#include <pbr/animation_blend_node.hpp>

GX_DISABLE_COMMON_WARNINGS

extern "C" {

#include <lua/lauxlib.h>

} // extern "C"

GX_RESTORE_WARNING_STATE


namespace pbr {

std::unordered_map<AnimationBlendNode const*, AnimationBlendNode::Reference> AnimationBlendNode::_abNodes {};

void AnimationBlendNode::Init ( lua_State &vm ) noexcept
{
    constexpr luaL_Reg const extensions[] =
    {
        {
            .name = "av_AnimationBlendNodeCreate",
            .func = &AnimationBlendNode::OnCreate
        },
        {
            .name = "av_AnimationBlendNodeDestroy",
            .func = &AnimationBlendNode::OnDestroy
        },
        {
            .name = "av_AnimationBlendNodeSetBlendFactor",
            .func = &AnimationBlendNode::OnSetBlendFactor
        },
        {
            .name = "av_AnimationBlendNodeSetInputA",
            .func = &AnimationBlendNode::OnSetInputA
        },
        {
            .name = "av_AnimationBlendNodeSetInputB",
            .func = &AnimationBlendNode::OnSetInputB
        }
    };

    for ( auto const &extension : extensions )
    {
        lua_register ( &vm, extension.name, extension.func );
    }
}

void AnimationBlendNode::Destroy () noexcept
{
    _abNodes.clear ();
}

JointProviderNode::Result AnimationBlendNode::GetJoint ( std::string const &name ) noexcept
{
    if ( !_inputA & !_inputB ) [[unlikely]]
        return std::nullopt;

    if ( _blendFactor == 0.0F )
    {
        if ( !_inputA ) [[unlikely]]
            return std::nullopt;

        return _inputA->GetJoint ( name );
    }

    if ( _blendFactor == 1.0F )
    {
        if ( !_inputB ) [[unlikely]]
            return std::nullopt;

        return _inputB->GetJoint ( name );
    }

    Result const resA = _inputA->GetJoint ( name );
    bool const isValidA = static_cast<bool> ( resA );

    Result const resB = _inputB->GetJoint ( name );
    bool const isValidB = static_cast<bool> ( resB );

    if ( !isValidA | !isValidB ) [[unlikely]]
        return std::nullopt;

    android_vulkan::Joint result {};
    result._location.LinearInterpolation ( resA->_location, resB->_location, _blendFactor );
    result._orientation.SphericalLinearInterpolation ( resA->_orientation, resB->_orientation, _blendFactor );

    return result;
}

void AnimationBlendNode::Update ( lua_State &vm, float deltaTime ) noexcept
{
    if ( ( _inputA != nullptr ) & ( _blendFactor != 1.0F ) )
        _inputA->Update ( vm, deltaTime );

    if ( ( _inputB != nullptr ) & ( _blendFactor != 0.0F ) )
    {
        _inputB->Update ( vm, deltaTime );
    }
}

int AnimationBlendNode::OnCreate ( lua_State* state )
{
    if ( !lua_checkstack ( state, 1 ) ) [[unlikely]]
    {
        android_vulkan::LogWarning ( "pbr::AnimationBlendNode::OnCreate - Stack is too small." );
        return 0;
    }

    auto ab = std::make_unique<AnimationBlendNode> ();
    AnimationBlendNode* handle = ab.get ();
    _abNodes.emplace ( handle, std::move ( ab ) );

    lua_pushlightuserdata ( state, handle );
    return 1;
}

int AnimationBlendNode::OnDestroy ( lua_State* state )
{
    auto* handle = static_cast<AnimationBlendNode*> ( lua_touserdata ( state, 1 ) );
    handle->UnregisterSelf ();

    [[maybe_unused]] auto const result = _abNodes.erase ( handle );
    AV_ASSERT ( result > 0U )

    return 0;
}

int AnimationBlendNode::OnSetBlendFactor ( lua_State* state )
{
    auto &self = *static_cast<AnimationBlendNode*> ( lua_touserdata ( state, 1 ) );
    self._blendFactor = static_cast<float> ( lua_tonumber ( state, 2 ) );
    return 0;
}

int AnimationBlendNode::OnSetInputA ( lua_State* state )
{
    auto* handle = static_cast<AnimationBlendNode*> ( lua_touserdata ( state, 1 ) );
    JointProviderNode* &selfInputA = handle->_inputA;

    if ( selfInputA ) [[likely]]
        selfInputA->UnregisterNode ( handle );

    auto* inputA = static_cast<JointProviderNode*> ( lua_touserdata ( state, 2 ) );
    inputA->RegisterNode ( handle );
    handle->RegisterNode ( inputA );
    selfInputA = inputA;

    return 0;
}

int AnimationBlendNode::OnSetInputB ( lua_State* state )
{
    auto* handle = static_cast<AnimationBlendNode*> ( lua_touserdata ( state, 1 ) );
    JointProviderNode* &selfInputB = handle->_inputB;

    if ( selfInputB ) [[likely]]
        selfInputB->UnregisterNode ( handle );

    auto* inputB = static_cast<JointProviderNode*> ( lua_touserdata ( state, 2 ) );
    inputB->RegisterNode ( handle );
    handle->RegisterNode ( inputB );
    selfInputB = inputB;

    return 0;
}


} // namespace pbr
