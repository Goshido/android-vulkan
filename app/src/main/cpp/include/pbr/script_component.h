#ifndef PBR_SCRIPT_COMPONENT_H
#define PBR_SCRIPT_COMPONENT_H


#include <GXCommon/GXWarning.h>

GX_DISABLE_COMMON_WARNINGS

#include <string>

GX_RESTORE_WARNING_STATE


namespace pbr {

class ScriptComponent final
{
    private:
        std::string const       _script;
        std::string const       _params {};

    public:
        ScriptComponent () = delete;

        ScriptComponent ( ScriptComponent const & ) = delete;
        ScriptComponent& operator = ( ScriptComponent const & ) = delete;

        ScriptComponent ( ScriptComponent && ) = delete;
        ScriptComponent& operator = ( ScriptComponent && ) = delete;

        explicit ScriptComponent ( std::string &&script ) noexcept;
        explicit ScriptComponent ( std::string &&script, std::string &&params ) noexcept;

        ~ScriptComponent () = default;

        [[nodiscard]] bool Register () noexcept;
        [[nodiscard]] bool Unregister () const noexcept;
};

} // namespace pbr


#endif // PBR_SCRIPT_COMPONENT_H
