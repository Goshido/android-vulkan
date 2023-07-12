#ifndef PBR_SCRIPT_COMPONENT_HPP
#define PBR_SCRIPT_COMPONENT_HPP


#include "actor.h"
#include "component.h"
#include "script_component_desc.h"


namespace pbr {

class ScriptComponent final : public Component
{
    private:
        Actor*                                                          _actor = nullptr;
        std::string                                                     _script {};
        std::string                                                     _params {};

        static int                                                      _registerScriptComponentIndex;
        static std::unordered_map<Component const*, ComponentRef>       _scripts;

    public:
        ScriptComponent () = delete;

        ScriptComponent ( ScriptComponent const & ) = delete;
        ScriptComponent &operator = ( ScriptComponent const & ) = delete;

        ScriptComponent ( ScriptComponent && ) = delete;
        ScriptComponent &operator = ( ScriptComponent && ) = delete;

        explicit ScriptComponent ( ScriptComponentDesc const &desc, uint8_t const* data ) noexcept;
        explicit ScriptComponent ( std::string &&name ) noexcept;
        explicit ScriptComponent ( std::string &&script, std::string &&name ) noexcept;
        explicit ScriptComponent ( std::string &&script, std::string &&params, std::string &&name ) noexcept;

        ~ScriptComponent () override = default;

        [[nodiscard]] bool RegisterFromNative ( lua_State &vm, Actor &actor ) noexcept;
        void RegisterFromScript ( Actor &actor ) noexcept;

        [[nodiscard]] static bool Init ( lua_State &vm ) noexcept;
        static void Destroy () noexcept;

    private:
        [[nodiscard]] ComponentRef &GetReference () noexcept override;

        [[nodiscard]] static int OnCreate ( lua_State* state );
        [[nodiscard]] static int OnDestroy ( lua_State* state );
        [[nodiscard]] static int OnGarbageCollected ( lua_State* state );
};

} // namespace pbr


#endif // PBR_SCRIPT_COMPONENT_HPP
