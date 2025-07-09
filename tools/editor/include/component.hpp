#ifndef EDITOR_COMPONENT_HPP
#define EDITOR_COMPONENT_HPP


#include <GXCommon/GXMath.hpp>
#include "save_state.hpp"

GX_DISABLE_COMMON_WARNINGS

#include <memory>
#include <optional>

GX_RESTORE_WARNING_STATE


namespace editor {

class Component
{
    public:
        using Ref = std::unique_ptr<Component>;

    private:
        GXMat4          _local {};
        GXMat4          _parent {};

        std::string     _name {};

    public:
        explicit Component () noexcept;

        Component ( Component const & ) = delete;
        Component &operator = ( Component const & ) = delete;

        Component ( Component && ) = delete;
        Component &operator = ( Component && ) = delete;

        explicit Component ( SaveState::Container const &info ) noexcept;

        virtual ~Component () = default;

        virtual void Save ( SaveState::Container &root ) const noexcept;

        [[nodiscard]] static std::optional<Ref> Spawn ( SaveState::Container const &info ) noexcept;
};

} // namespace editor


#endif // EDITOR_COMPONENT_HPP
