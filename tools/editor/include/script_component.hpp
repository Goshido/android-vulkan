#ifndef EDITOR_SCRIPT_COMPONENT_HPP
#define EDITOR_SCRIPT_COMPONENT_HPP


#include "component.hpp"


namespace editor {

class ScriptComponent final : public Component
{
    public:
        constexpr static std::string_view       TYPE = "Script";

    public:
        explicit ScriptComponent () noexcept;

        ScriptComponent ( ScriptComponent const & ) = delete;
        ScriptComponent &operator = ( ScriptComponent const & ) = delete;

        ScriptComponent ( ScriptComponent && ) = delete;
        ScriptComponent &operator = ( ScriptComponent && ) = delete;

        explicit ScriptComponent ( SaveState::Container const &info ) noexcept;

        ~ScriptComponent () = default;

    private:
        void Save ( SaveState::Container &root ) const noexcept override;
};

} // namespace editor


#endif // EDITOR_SCRIPT_COMPONENT_HPP
