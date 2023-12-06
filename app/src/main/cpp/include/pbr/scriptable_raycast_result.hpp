#ifndef PBR_SCRIPTABLE_RAYCAST_RESULT_HPP
#define PBR_SCRIPTABLE_RAYCAST_RESULT_HPP


#include <rigid_body.hpp>

GX_DISABLE_COMMON_WARNINGS

extern "C" {

#include <lua/lstate.h>

} // extern "C"

GX_RESTORE_WARNING_STATE


namespace pbr {

class [[maybe_unused]] ScriptableRaycastResult final
{
    private:
        [[maybe_unused]] GXVec3*    _normal = nullptr;
        [[maybe_unused]] GXVec3*    _position = nullptr;

    public:
        ScriptableRaycastResult () = default;

        ScriptableRaycastResult ( ScriptableRaycastResult const & ) = delete;
        ScriptableRaycastResult &operator = ( ScriptableRaycastResult const & ) = delete;

        ScriptableRaycastResult ( ScriptableRaycastResult && ) = delete;
        ScriptableRaycastResult &operator = ( ScriptableRaycastResult && ) = delete;

        ~ScriptableRaycastResult () = default;

        [[nodiscard]] bool Init ( lua_State &vm ) noexcept;
        void Destroy ( lua_State &vm ) noexcept;

        [[nodiscard]] bool PublishResult ( lua_State &vm,
            android_vulkan::RigidBodyRef &body,
            GXVec3 const &position,
            GXVec3 const &normal
        ) noexcept;
};

} // namespace pbr


#endif // PBR_SCRIPTABLE_RAYCAST_RESULT_HPP
