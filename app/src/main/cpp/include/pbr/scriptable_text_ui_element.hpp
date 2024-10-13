#ifndef PBR_SCRIPTABLE_TEXT_UI_ELEMENT_HPP
#define PBR_SCRIPTABLE_TEXT_UI_ELEMENT_HPP


#include "scriptable_ui_element.hpp"
#include "text_ui_element.hpp"


namespace pbr {

class ScriptableTextUIElement final : public ScriptableUIElement
{
    private:
        TextUIElement       _text;

    public:
        ScriptableTextUIElement () = delete;

        ScriptableTextUIElement ( ScriptableTextUIElement const & ) = delete;
        ScriptableTextUIElement &operator = ( ScriptableTextUIElement const & ) = delete;

        ScriptableTextUIElement ( ScriptableTextUIElement && ) = delete;
        ScriptableTextUIElement &operator = ( ScriptableTextUIElement && ) = delete;

        explicit ScriptableTextUIElement ( bool &success,
            UIElement const* parent,
            lua_State &vm,
            int errorHandlerIdx,
            std::u32string &&text
        ) noexcept;

        ~ScriptableTextUIElement () override = default;

        [[nodiscard]] UIElement &GetElement () noexcept override;
        [[nodiscard]] UIElement const &GetElement () const noexcept override;

        static void Init ( lua_State &vm ) noexcept;

    private:
        [[nodiscard]] static int OnSetColorHSV ( lua_State* state );
        [[nodiscard]] static int OnSetColorRGB ( lua_State* state );
        [[nodiscard]] static int OnSetText ( lua_State* state );
};

} // namespace pbr


#endif // PBR_SCRIPTABLE_TEXT_UI_ELEMENT_HPP
