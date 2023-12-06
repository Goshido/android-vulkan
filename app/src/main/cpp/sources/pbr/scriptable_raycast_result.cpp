#include <pbr/scriptable_raycast_result.hpp>


namespace pbr {

bool ScriptableRaycastResult::Init ( lua_State &/*vm*/ ) noexcept
{
    // TODO
    return true;
}

void ScriptableRaycastResult::Destroy ( lua_State &/*vm*/ ) noexcept
{
    // TODO
}

bool ScriptableRaycastResult::PublishResult ( lua_State &/*vm*/,
    android_vulkan::RigidBodyRef &/*body*/,
    GXVec3 const &/*position*/,
    GXVec3 const &/*normal*/
) noexcept
{
    // TODO
    return true;
}

} // namespace pbr
