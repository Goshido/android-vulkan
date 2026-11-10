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

    public:
        explicit PropertyTool () = default;

        PropertyTool ( PropertyTool const & ) = delete;
        PropertyTool &operator = ( PropertyTool const & ) = delete;

        PropertyTool ( PropertyTool && ) = delete;
        PropertyTool &operator = ( PropertyTool && ) = delete;

        ~PropertyTool () = default;

        void Init ( SaveState::Container const &save ) noexcept;
        void Destroy ( SaveState::Container &save ) noexcept;

        void OnContentCommitted ( Selection::Actors const &actors ) noexcept;
        void OnContentUpdated ( Selection::Actors const &actors ) noexcept;
        void OnSelectionChanged ( Selection::Actors const &actors ) noexcept;

        void AddString ( std::string_view name, StringProperty &value ) noexcept;

        void AddSeparator () noexcept;

        // FUCK - add collapse property which owns by component
        void BeginComponent ( StringProperty &name, std::string_view glyph ) noexcept;
        void EndComponent () noexcept;

    private:
        void CreateUI () noexcept;
};

} // namespace editor


#endif // EDITOR_PROPERTY_TOOL_HPP
