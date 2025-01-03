#ifndef PBR_UI_LAYER_HPP
#define PBR_UI_LAYER_HPP


#include "html5_element.hpp"
#include "scriptable_div_ui_element.hpp"
#include "ui_pass.hpp"

GX_DISABLE_COMMON_WARNINGS

#include <string>
#include <unordered_set>

GX_RESTORE_WARNING_STATE


namespace pbr {

class UILayer final
{
    public:
        struct LayoutStatus final
        {
            bool                                _hasChanges = true;
            size_t                              _neededUIVertices = 0U;
        };

    private:
        ScriptableDIVUIElement*                 _body = nullptr;
        CSSParser                               _css {};
        std::vector<float>                      _lineHeights { 1U, 0.0F };

        static std::unordered_set<UILayer*>     _uiLayers;

    public:
        UILayer () = delete;

        UILayer ( UILayer const & ) = delete;
        UILayer &operator = ( UILayer const & ) = delete;

        UILayer ( UILayer && ) = delete;
        UILayer &operator = ( UILayer && ) = delete;

        explicit UILayer ( bool &success, lua_State &vm ) noexcept;

        ~UILayer () = default;

        [[nodiscard]] LayoutStatus ApplyLayout ( android_vulkan::Renderer &renderer,
            FontStorage &fontStorage
        ) noexcept;

        void Submit ( UIElement::SubmitInfo &info ) noexcept;
        [[nodiscard]] bool UpdateCache ( FontStorage &fontStorage, VkExtent2D const &viewport ) noexcept;

        static void InitLuaFrontend ( lua_State &vm ) noexcept;
        static void Destroy () noexcept;

    private:
        [[nodiscard]] static bool AppendChild ( lua_State &vm,
            int errorHandlerIdx,
            int appendChildElementIdx,
            int uiLayerIdx,
            int registerNamedElementIdx,
            ScriptableDIVUIElement &parent,
            HTML5Element &htmlChild
        ) noexcept;

        [[nodiscard]] static bool RegisterNamedElement ( lua_State &vm,
            int errorHandlerIdx,
            int uiLayerIdx,
            int registerNamedElementIdx,
            std::u32string const &id
        ) noexcept;

        [[nodiscard]] static int OnCreate ( lua_State* state );
        [[nodiscard]] static int OnGarbageCollected ( lua_State* state );
        [[nodiscard]] static int OnHide ( lua_State* state );
        [[nodiscard]] static int OnIsVisible ( lua_State* state );
        [[nodiscard]] static int OnShow ( lua_State* state );
};

} // namespace pbr


#endif // PBR_UI_LAYER_HPP
