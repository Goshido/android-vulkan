#include <av_assert.hpp>
#include <logger.hpp>
#include <pbr/animation_player_node.hpp>

GX_DISABLE_COMMON_WARNINGS

extern "C" {

#include <lua/lauxlib.h>

} // extern "C"

GX_RESTORE_WARNING_STATE


namespace pbr {

std::unordered_map<AnimationPlayerNode const*, AnimationPlayerNode::Reference> AnimationPlayerNode::_nodes {};

void AnimationPlayerNode::Init ( lua_State &vm ) noexcept
{
    constexpr luaL_Reg const extensions[] =
    {
        {
            .name = "av_AnimationPlayerNodeCreate",
            .func = &AnimationPlayerNode::OnCreate
        },
        {
            .name = "av_AnimationPlayerNodeDestroy",
            .func = &AnimationPlayerNode::OnDestroy
        },
        {
            .name = "av_AnimationPlayerNodeLoadAnimation",
            .func = &AnimationPlayerNode::OnLoadAnimation
        },
        {
            .name = "av_AnimationPlayerNodeSetPlaybackSpeed",
            .func = &AnimationPlayerNode::OnSetPlaybackSpeed
        }
    };

    for ( auto const &extension : extensions )
    {
        lua_register ( &vm, extension.name, extension.func );
    }
}

void AnimationPlayerNode::Destroy () noexcept
{
    _nodes.clear ();
}

JointProviderNode::Result AnimationPlayerNode::GetJoint ( std::string const &name ) noexcept
{
    if ( !_hasTrack || !_track.HasBone ( name ) )
        return std::nullopt;

    android_vulkan::Joint const &p = _track.GetJoint ( name, _prevFrame );
    android_vulkan::Joint const &n = _track.GetJoint ( name, _nextFrame );

    android_vulkan::Joint result {};
    result._location.LinearInterpolation ( p._location, n._location, _interpolation );
    result._orientation.SphericalLinearInterpolation ( p._orientation, n._orientation, _interpolation );

    return result;
}

void AnimationPlayerNode::Update ( float deltaTime ) noexcept
{
    if ( !_hasTrack ) [[unlikely]]
        return;

    _time += deltaTime * _playbackSpeed;

    while ( _time >= _trackDuration ) [[unlikely]]
        _time -= _trackDuration;

    while ( _time < 0.0F ) [[unlikely]]
        _time += _trackDuration;

    float p;
    _interpolation = std::modf ( _time * _timeToFrame, &p );
    auto const intP = static_cast<size_t> ( p );

    _prevFrame = intP;

    size_t const cases[] = { intP + 1U, 0U };
    _nextFrame = cases[ static_cast<size_t> ( _lastFrame == intP ) ];
}

bool AnimationPlayerNode::LoadAnimation ( std::string &&animationTrack ) noexcept
{
    if ( _hasTrack = _track.Load ( std::move ( animationTrack ) ); !_hasTrack ) [[unlikely]]
        return false;

    _time = 0.0F;
    size_t const frameCount = _track.GetFrameCount ();
    _lastFrame = frameCount - 1U;

    auto const f = static_cast<float> ( frameCount );
    _trackDuration = f / _track.GetFPS ();
    _timeToFrame = f / _trackDuration;
    return true;
}

int AnimationPlayerNode::OnCreate ( lua_State* state )
{
    if ( !lua_checkstack ( state, 1 ) ) [[unlikely]]
    {
        android_vulkan::LogWarning ( "pbr::AnimationPlayerNode::OnCreate - Stack is too small." );
        return 0;
    }

    auto ap = std::make_unique<AnimationPlayerNode> ();
    AnimationPlayerNode* handle = ap.get ();
    _nodes.emplace ( handle, std::move ( ap ) );

    lua_pushlightuserdata ( state, handle );
    return 1;
}

int AnimationPlayerNode::OnDestroy ( lua_State* state )
{
    auto* handle = static_cast<AnimationPlayerNode*> ( lua_touserdata ( state, 1 ) );
    handle->UnregisterSelf ();

    [[maybe_unused]] auto const result = _nodes.erase ( handle );
    AV_ASSERT ( result > 0U )
    return 0;
}

int AnimationPlayerNode::OnLoadAnimation ( lua_State* state )
{
    if ( !lua_checkstack ( state, 1 ) ) [[unlikely]]
    {
        android_vulkan::LogWarning ( "pbr::AnimationPlayerNode::OnLoadAnimation - Stack is too small." );
        return 0;
    }

    char const* animation = lua_tostring ( state, 2 );

    if ( !animation ) [[unlikely]]
    {
        android_vulkan::LogWarning ( "pbr::AnimationPlayerNode::OnLoadAnimation - Animation file is not specified." );
        lua_pushboolean ( state, false );
        return 1;
    }

    auto &self = *static_cast<AnimationPlayerNode *> ( lua_touserdata ( state, 1 ) );
    lua_pushboolean ( state, self.LoadAnimation ( animation ) );
    return 1;
}

int AnimationPlayerNode::OnSetPlaybackSpeed ( lua_State* state )
{
    auto &self = *static_cast<AnimationPlayerNode *> ( lua_touserdata ( state, 1 ) );
    self._playbackSpeed = static_cast<float> ( lua_tonumber ( state, 2 ) );
    return 0;
}

} // namespace pbr
