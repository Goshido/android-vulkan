#include <av_assert.hpp>
#include <logger.hpp>
#include <pbr/animation_player_node.hpp>
#include <pbr/script_engine.hpp>

GX_DISABLE_COMMON_WARNINGS

extern "C" {

#include <lua/lauxlib.h>

} // extern "C"

GX_RESTORE_WARNING_STATE


namespace pbr {

constexpr std::string_view FIELD_CONTEXT = "_context";
constexpr std::string_view FIELD_CALLBACK = "_callback";
constexpr char const GLOBAL_TABLE[] = "av_scriptablePlayerNodeEvents";

namespace {

} // end of anonymous namespace

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
            .name = "av_AnimationPlayerNodeSetEvent",
            .func = &AnimationPlayerNode::OnSetEvent
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

    lua_newtable ( &vm );
    lua_setglobal ( &vm, GLOBAL_TABLE );
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

void AnimationPlayerNode::Update ( lua_State &vm, float deltaTime ) noexcept
{
    if ( !_hasTrack ) [[unlikely]]
        return;

    float const oldTime = _time;
    _time += deltaTime * _playbackSpeed;

    while ( _time >= _trackDuration ) [[unlikely]]
        _time -= _trackDuration;

    while ( _time < 0.0F ) [[unlikely]]
        _time += _trackDuration;

    float p;
    _interpolation = std::modf ( _time * _timeToFrame, &p );
    auto const intP = static_cast<size_t> ( p );

    _prevFrame = intP;

    size_t const frameCases[] = { intP + 1U, 0U };
    _nextFrame = frameCases[ static_cast<size_t> ( _lastFrame == intP ) ];

    if ( _events.empty () ) [[likely]]
        return;

    float const timeCases[] = { oldTime, _time };
    bool const isForward = _playbackSpeed > 0.0F;
    float const t0 = timeCases[ static_cast<size_t> ( !isForward ) ];
    float const t1 = timeCases[ static_cast<size_t> ( isForward ) ];

    auto const processEvents = [ & ] ( float from, float to ) noexcept
    {
        for ( auto const [frameIdx, time] : _events )
        {
            if ( time < from | time > to ) [[likely]]
                continue;

            if ( !lua_checkstack ( &vm, 4 ) ) [[unlikely]]
            {
                android_vulkan::LogWarning ( "pbr::AnimationPlayerNode::Update - Stack is too small." );
                return;
            }

            lua_getglobal ( &vm, GLOBAL_TABLE );

            lua_pushlightuserdata ( &vm, this );
            lua_rawget ( &vm, -2 );

            auto const idx = static_cast<lua_Integer> ( frameIdx );

            lua_replace ( &vm, -2 );
            lua_pushinteger ( &vm, idx );
            lua_rawget ( &vm, -2 );

            lua_replace ( &vm, -2 );
            lua_pushlstring ( &vm, FIELD_CALLBACK.data (), FIELD_CALLBACK.size () );
            lua_rawget ( &vm, -2 );

            lua_pushlstring ( &vm, FIELD_CONTEXT.data (), FIELD_CONTEXT.size () );
            lua_rawget ( &vm, -3 );

            lua_pushinteger ( &vm, idx );

            auto const errorElementInStack = static_cast<lua_Integer> (
                lua_pcall ( &vm, 2, 0, ScriptEngine::GetErrorHandlerIndex () ) != LUA_OK
            );

            lua_pop ( &vm, 1 + errorElementInStack );
        }
    };

    if ( t0 < t1 ) [[likely]]
    {
        // No loop transition: Single time range.
        processEvents ( t0, t1 );
        return;
    }

    // Loop transition: Two time ranges.
    processEvents ( 0.0F, t1 );
    processEvents ( t0, _trackDuration );
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

int AnimationPlayerNode::SetEvent ( lua_State* state ) noexcept
{
    if ( !lua_checkstack ( state, 4 ) ) [[unlikely]]
    {
        android_vulkan::LogWarning ( "pbr::AnimationPlayerNode::SetEvent - Stack is too small." );
        return 0;
    }

    if ( !_hasTrack ) [[unlikely]]
    {
        android_vulkan::LogWarning ( "pbr::AnimationPlayerNode::SetEvent - Animation is not set yet." );
        lua_pushboolean ( state, false );
        return 1;
    }

    auto const frameIdx = static_cast<size_t> ( lua_tointeger ( state, 3 ) );
    size_t const frameCount = _track.GetFrameCount ();

    if ( frameIdx >= _track.GetFrameCount () ) [[unlikely]]
    {
        android_vulkan::LogWarning ( "pbr::AnimationPlayerNode::SetEvent - Can't set event with frame index %zu. "
            "Animation has only %zu frames.", frameIdx, frameCount );

        lua_pushboolean ( state, false );
        return 1;
    }

    lua_getglobal ( state, GLOBAL_TABLE );
    lua_pushlightuserdata ( state, this );

    if ( lua_rawget ( state, -2 ) == LUA_TNIL ) [[likely]]
    {
        lua_pop ( state, 1 );
        lua_newtable ( state );
        lua_pushlightuserdata ( state, this );
        lua_pushvalue ( state, -2 );
        lua_rawset ( state, -4 );
    }

    lua_replace ( state, -2 );
    auto const idx = static_cast<lua_Integer> ( frameIdx );
    lua_pushinteger ( state, idx );

    if ( lua_rawget ( state, -2 ) == LUA_TNIL ) [[likely]]
    {
        lua_pop ( state, 1 );
        lua_newtable ( state );
        lua_pushinteger ( state, idx );
        lua_pushvalue ( state, -2 );
        lua_rawset ( state, -4 );
    }

    lua_replace ( state, -2 );
    lua_pushlstring ( state, FIELD_CONTEXT.data (), FIELD_CONTEXT.size () );
    lua_pushvalue ( state, 2 );
    lua_rawset ( state, -3 );

    lua_pushlstring ( state, FIELD_CALLBACK.data (), FIELD_CALLBACK.size () );
    lua_pushvalue ( state, 4 );
    lua_rawset ( state, -3 );

    _events.insert (
        std::find_if ( _events.cbegin (),
            _events.cend (),

            [ frameIdx ] ( Event const &e ) constexpr -> bool {
                return e._frameIdx > frameIdx;
            }
        ),

        Event
        {
            ._frameIdx = frameIdx,
            ._time = static_cast<float> ( frameIdx ) / _track.GetFPS ()
        }
    );

    lua_pop ( state, 1 );
    lua_pushboolean ( state, true );
    return 1;
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

    lua_getglobal ( state, GLOBAL_TABLE );
    lua_pushlightuserdata ( state, handle );
    lua_pushnil ( state );
    lua_rawset ( state, -3 );
    handle->_events.clear ();

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

    auto &self = *static_cast<AnimationPlayerNode*> ( lua_touserdata ( state, 1 ) );
    lua_pushboolean ( state, self.LoadAnimation ( animation ) );
    return 1;
}

int AnimationPlayerNode::OnSetEvent ( lua_State* state )
{
    auto &self = *static_cast<AnimationPlayerNode*> ( lua_touserdata ( state, 1 ) );
    return self.SetEvent ( state );
}

int AnimationPlayerNode::OnSetPlaybackSpeed ( lua_State* state )
{
    auto &self = *static_cast<AnimationPlayerNode*> ( lua_touserdata ( state, 1 ) );
    self._playbackSpeed = static_cast<float> ( lua_tonumber ( state, 2 ) );
    return 0;
}

} // namespace pbr
