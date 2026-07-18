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
        GXQuat                                  _rotation = GXQuat::IDENTITY;
        GXVec3                                  _scale = GXVec3::ONE;
        GXVec3                                  _location = GXVec3::ZERO;
        std::string                             _name = std::string ( DEFAULT_NAME );

    public:
        explicit Actor () = default;

        Actor ( Actor const & ) = delete;
        Actor &operator = ( Actor const & ) = delete;

        Actor ( Actor && ) = delete;
        Actor &operator = ( Actor && ) = delete;

        explicit Actor ( SaveState::Container const &info ) noexcept;

        ~Actor () noexcept;

        void SetName ( std::string_view name ) noexcept;

        void Append ( ComponentRef &&component ) noexcept;
        void Insert ( size_t before, ComponentRef &&component ) noexcept;

        void Select () noexcept;
        void Deselect () noexcept;

        // Method returns just removed component.
        [[nodiscard]] ComponentRef Remove ( Component const &component ) noexcept;

        void Save ( SaveState::Container &root ) const noexcept;

        void SetRotation ( GXQuat const &rotation ) noexcept;
        void SetRotation ( GXMat3 const &rotation ) noexcept;
        void SetRotation ( GXMat4 const &rotation ) noexcept;

        void SetLocation ( GXVec3 const &location ) noexcept;
        void SetScale ( GXVec3 const &scale ) noexcept;

        void SetLocal ( GXMat4 const &local ) noexcept;
        void SetLocal ( GXQuat const &rotation, GXVec3 const &location ) noexcept;
        void SetLocal ( GXQuat const &rotation, GXVec3 const &location, GXVec3 const &scale ) noexcept;

        void GetTransform ( GXMat4 &dst ) noexcept;

    private:
        void NotifyTransformChanged () noexcept;
};

using ActorRef = std::unique_ptr<Actor>;

} // namespace editor


#endif // EDITOR_ACTOR_HPP
