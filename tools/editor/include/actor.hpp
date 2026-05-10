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

        using Components = std::deque<ComponentRef>;

    private:
        Components                              _components {};
        GXQuat                                  _rotation {};
        GXVec3                                  _scale {};
        GXVec3                                  _location {};
        std::string                             _name = std::string ( DEFAULT_NAME );

    public:
        explicit Actor () = default;

        Actor ( Actor const & ) = delete;
        Actor &operator = ( Actor const & ) = delete;

        Actor ( Actor && ) = delete;
        Actor &operator = ( Actor && ) = delete;

        explicit Actor ( SaveState::Container const &info ) noexcept;

        ~Actor () = default;

        void SetName ( std::string_view name ) noexcept;

        void Append ( std::unique_ptr<Component> &&component ) noexcept;
        void Insert ( size_t before, std::unique_ptr<Component> &&component ) noexcept;
        void Save ( SaveState::Container &root ) const noexcept;
};

using ActorRef = std::unique_ptr<Actor>;

} // namespace editor


#endif // EDITOR_ACTOR_HPP
