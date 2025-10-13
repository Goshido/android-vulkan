#ifndef EDITOR_ACTOR_HPP
#define EDITOR_ACTOR_HPP


#include "component.hpp"

GX_DISABLE_COMMON_WARNINGS

#include <deque>

GX_RESTORE_WARNING_STATE


namespace editor {

class Actor final
{
    private:
        constexpr static std::string_view       DEFAULT_NAME = "actor";

        using Components = std::deque<Component::Ref>;

    private:
        Components                              _components {};
        GXMat4                                  _local = GXMat4::IDENTITY;
        std::string                             _name = std::string ( DEFAULT_NAME );

    public:
        explicit Actor () = default;

        Actor ( Actor const & ) = delete;
        Actor &operator = ( Actor const & ) = delete;

        Actor ( Actor && ) = delete;
        Actor &operator = ( Actor && ) = delete;

        explicit Actor ( SaveState::Container const &info ) noexcept;

        ~Actor () = default;

        void Append ( std::unique_ptr<Component> &&component ) noexcept;
        void Insert ( size_t before, std::unique_ptr<Component> &&component ) noexcept;
        void Save ( SaveState::Container &root ) const noexcept;
};

} // namespace editor


#endif // EDITOR_ACTOR_HPP
