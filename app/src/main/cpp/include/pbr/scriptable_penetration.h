#ifndef PBR_SCRIPTABLE_PENETRATION_H
#define PBR_SCRIPTABLE_PENETRATION_H


#include <physics.h>
#include "rigid_body_component.h"
#include "scriptable_gxvec3.h"


namespace pbr {

class ScriptablePenetration final
{
    private:
        std::vector<std::reference_wrapper<ScriptableGXVec3::Item>>     _normals {};

    public:
        ScriptablePenetration () = default;

        ScriptablePenetration ( ScriptablePenetration const & ) = delete;
        ScriptablePenetration &operator = ( ScriptablePenetration const & ) = delete;

        ScriptablePenetration ( ScriptablePenetration && ) = delete;
        ScriptablePenetration &operator = ( ScriptablePenetration && ) = delete;

        ~ScriptablePenetration () = default;

        [[nodiscard]] bool Init ( lua_State &vm ) noexcept;
        void Destroy ( lua_State &vm ) noexcept;

        [[nodiscard]] bool PublishResult ( lua_State &vm,
            std::vector<android_vulkan::Penetration> const &penetrations
        ) noexcept;

    private:
        [[nodiscard]] bool Append ( lua_State &vm,
            int vec3Constructor,
            int penetrationIndex,
            lua_Number depth,
            GXVec3 const &normal,
            int rigidBodyComponentStack
        ) noexcept;

        [[nodiscard]] static bool FindVec3Constructor ( lua_State &vm ) noexcept;
};

} // namespace pbr


#endif // PBR_SCRIPTABLE_PENETRATION_H
