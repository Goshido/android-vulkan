#ifndef PBR_SCRIPT_COMPONENT_H
#define PBR_SCRIPT_COMPONENT_H


#include "component.h"


namespace pbr {

class ScriptComponent final : public Component
{
    private:
        std::string const       _script;
        std::string const       _params {};

        static int              _registerScriptComponentIndex;

    public:
        ScriptComponent () = delete;

        ScriptComponent ( ScriptComponent const & ) = delete;
        ScriptComponent& operator = ( ScriptComponent const & ) = delete;

        ScriptComponent ( ScriptComponent && ) = delete;
        ScriptComponent& operator = ( ScriptComponent && ) = delete;

        explicit ScriptComponent ( std::string &&script, std::string &&name ) noexcept;
        explicit ScriptComponent ( std::string &&script, std::string &&params, std::string &&name ) noexcept;

        ~ScriptComponent () override = default;

        [[nodiscard]] bool Register ( lua_State &vm ) noexcept;

        [[nodiscard]] static bool Init ( lua_State &vm ) noexcept;

    private:
        [[nodiscard]] static int OnCreate ( lua_State* state );
};

} // namespace pbr


#endif // PBR_SCRIPT_COMPONENT_H
