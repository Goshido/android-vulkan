#ifndef PBR_SCRIPT_COMPONENT_H
#define PBR_SCRIPT_COMPONENT_H


#include "component.h"
#include "script_component_desc.h"


namespace pbr {

class ScriptComponent final : public Component
{
    private:
        std::string     _script {};
        std::string     _params {};

        static int      _registerScriptComponentIndex;

    public:
        ScriptComponent () = delete;

        ScriptComponent ( ScriptComponent const & ) = delete;
        ScriptComponent& operator = ( ScriptComponent const & ) = delete;

        ScriptComponent ( ScriptComponent && ) = delete;
        ScriptComponent& operator = ( ScriptComponent && ) = delete;

        explicit ScriptComponent ( ScriptComponentDesc const &desc, uint8_t const* data ) noexcept;
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
