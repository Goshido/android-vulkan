#ifndef EDITOR_PROPERTY_TOOL_HPP
#define EDITOR_PROPERTY_TOOL_HPP


#include "hotkey.hpp"
#include "save_state.hpp"
#include "selection.hpp"
#include "ui_property_dialog.hpp"


namespace editor {

class PropertyTool final
{
    private:
        Hotkey                  _toggle {};
        Rect                    _rect {};
        UIPropertyDialog*       _ui = nullptr;

        static PropertyTool*    _instance;

    public:
        explicit PropertyTool () noexcept;

        PropertyTool ( PropertyTool const & ) = delete;
        PropertyTool &operator = ( PropertyTool const & ) = delete;

        PropertyTool ( PropertyTool && ) = delete;
        PropertyTool &operator = ( PropertyTool && ) = delete;

        ~PropertyTool () = default;

        void Init ( SaveState::Container const &save ) noexcept;
        void Destroy ( SaveState::Container &save ) noexcept;

        void Rebuild ( Selection::Actors const &actors ) noexcept;

        // Method return true if there is anything to delete. Otherwise method return false.
        [[nodiscard]] bool DeleteComponents () noexcept;

        void OnContentUpdated ( Selection::Actors const &actors ) noexcept;

        void Append ( std::string_view name, StringProperty &property ) noexcept;

        void Append ( StringProperty &componentName,
            bool &componentCollapse,
            bool &componentEdit,
            std::string_view componentGlyph
        ) noexcept;

        void AppendSeparator () noexcept;

        [[nodiscard]] static PropertyTool &Instance () noexcept;

    private:
        void CreateUI ( Selection::Actors const &actors ) noexcept;
};

} // namespace editor


#endif // EDITOR_PROPERTY_TOOL_HPP
