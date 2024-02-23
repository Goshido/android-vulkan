#ifndef PBR_SCRIPTABLE_PENETRATION_HPP
#define PBR_SCRIPTABLE_PENETRATION_HPP


#include <physics.hpp>
//#include "scriptable_gxvec3.hpp"


namespace pbr {

class ScriptablePenetration final
{
    private:
        std::vector<GXVec3*>    _normals {};

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
            int errorHandlerIdx,
            int vec3Constructor,
            int penetrationIndex,
            int rigidBodyComponentIdx,
            lua_Number depth,
            GXVec3 const &normal
        ) noexcept;
};

} // namespace pbr


#endif // PBR_SCRIPTABLE_PENETRATION_HPP
