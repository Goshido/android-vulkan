#ifndef PBR_SCRIPTABLE_UI_ELEMENT_HPP
#define PBR_SCRIPTABLE_UI_ELEMENT_HPP


#include "ui_element.hpp"

GX_DISABLE_COMMON_WARNINGS

#include <unordered_map>

extern "C" {

#include <lua/lstate.h>

} // extern "C"

// This macro was defined by Lua headers and breaks 'std'.
#undef ispow2

GX_RESTORE_WARNING_STATE


namespace pbr {

class ScriptableUIElement
{
    private:
        using Storage = std::unordered_map<ScriptableUIElement const*, std::unique_ptr<ScriptableUIElement>>;

    private:
        static Storage      _uiElements;

    public:
        ScriptableUIElement ( ScriptableUIElement const & ) = delete;
        ScriptableUIElement &operator = ( ScriptableUIElement const & ) = delete;

        ScriptableUIElement ( ScriptableUIElement && ) = delete;
        ScriptableUIElement &operator = ( ScriptableUIElement && ) = delete;

        virtual ~ScriptableUIElement () = default;

        [[nodiscard]] virtual UIElement &GetElement () noexcept = 0;
        [[nodiscard]] virtual UIElement const &GetElement () const noexcept = 0;

        static void AppendElement ( ScriptableUIElement &element ) noexcept;
        static void InitCommon ( lua_State &vm ) noexcept;
        static void Destroy () noexcept;

    protected:
        explicit ScriptableUIElement () = default;

    private:
        [[nodiscard]] static int OnGarbageCollected ( lua_State* state );
        [[nodiscard]] static int OnHide ( lua_State* state );
        [[nodiscard]] static int OnIsVisible ( lua_State* state );
        [[nodiscard]] static int OnShow ( lua_State* state );
};

} // namespace pbr


#endif // PBR_SCRIPTABLE_UI_ELEMENT_HPP
