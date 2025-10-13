#ifndef PBR_SCRIPTABLE_RAYCAST_RESULT_HPP
#define PBR_SCRIPTABLE_RAYCAST_RESULT_HPP


#include <ray_caster.hpp>

GX_DISABLE_COMMON_WARNINGS

extern "C" {

#include <lua/lstate.h>

} // extern "C"

GX_RESTORE_WARNING_STATE


namespace pbr {

class ScriptableRaycastResult final
{
    private:
        GXVec3*     _normal = nullptr;
        GXVec3*     _point = nullptr;

    public:
        ScriptableRaycastResult () = default;

        ScriptableRaycastResult ( ScriptableRaycastResult const & ) = delete;
        ScriptableRaycastResult &operator = ( ScriptableRaycastResult const & ) = delete;

        ScriptableRaycastResult ( ScriptableRaycastResult && ) = delete;
        ScriptableRaycastResult &operator = ( ScriptableRaycastResult && ) = delete;

        ~ScriptableRaycastResult () = default;

        [[nodiscard]] bool Init ( lua_State &vm ) noexcept;
        void Destroy ( lua_State &vm ) noexcept;

        [[nodiscard]] bool PublishResult ( lua_State &vm, android_vulkan::RaycastResult const &result ) noexcept;
};

} // namespace pbr


#endif // PBR_SCRIPTABLE_RAYCAST_RESULT_HPP
