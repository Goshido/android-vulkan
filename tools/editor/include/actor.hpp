#ifndef EDITOR_ACTOR_HPP
#define EDITOR_ACTOR_HPP


#include "component.hpp"
#include "string_property.hpp"

GX_DISABLE_COMMON_WARNINGS

#include <deque>

GX_RESTORE_WARNING_STATE


namespace editor {

class Actor final
{
    public:
        class ComponentInfo final
        {
            public:
                ComponentRef                    _component {};
                size_t                          _index = 0UZ;

            public:
                // Implemented for std::vector compatibility. Must be avoided. Any usage will raise assert.
                explicit ComponentInfo () noexcept;
                ComponentInfo ( ComponentInfo const &other ) noexcept;
                ComponentInfo &operator = ( ComponentInfo const &other ) noexcept;

                ComponentInfo ( ComponentInfo &&other ) noexcept;
                ComponentInfo &operator = ( ComponentInfo &&other ) noexcept;

                explicit ComponentInfo ( ComponentRef &&component, size_t index ) noexcept;

                ~ComponentInfo () = default;
        };

    private:
        constexpr static std::string_view       DEFAULT_NAME = "actor";

        using Components = std::deque<ComponentRef>;

    private:
        Components                              _components {};
        GXQuat                                  _rotation = GXQuat::IDENTITY;
        GXVec3                                  _scale = GXVec3::ONE;
        GXVec3                                  _location = GXVec3::ZERO;
        StringProperty                          _name { std::string ( DEFAULT_NAME ) };

    public:
        explicit Actor () = default;

        Actor ( Actor const & ) = delete;
        Actor &operator = ( Actor const & ) = delete;

        Actor ( Actor && ) = delete;
        Actor &operator = ( Actor && ) = delete;

        explicit Actor ( SaveState::Container const &info ) noexcept;

        ~Actor () noexcept;

        void SetName ( std::string &&name ) noexcept;

        void Attach ( ComponentInfo &&component ) noexcept;
        [[nodiscard]] ComponentInfo Detach ( Component const &component ) noexcept;

        void Select () noexcept;
        void Deselect () noexcept;

        void Save ( SaveState::Container &root ) const noexcept;
        void ShowProperties ( PropertyTool &propertyTool ) noexcept;

        [[nodiscard]] GXQuat const &GetRotation () const noexcept;
        void SetRotation ( GXQuat const &rotation ) noexcept;
        void SetRotation ( GXMat3 const &rotation ) noexcept;
        void SetRotation ( GXMat4 const &rotation ) noexcept;

        [[nodiscard]] GXVec3 const &GetLocation () const noexcept;
        void SetLocation ( GXVec3 const &location ) noexcept;

        [[nodiscard]] GXVec3 const &GetScale () const noexcept;
        void SetScale ( GXVec3 const &scale ) noexcept;

        void SetLocal ( GXQuat const &rotation, GXVec3 const &location ) noexcept;
        void SetLocal ( GXQuat const &rotation, GXVec3 const &location, GXVec3 const &scale ) noexcept;
        void SetLocal ( GXVec3 const &location, GXVec3 const &scale ) noexcept;

    private:
        void NotifyTransformChanged () noexcept;
};

using ActorRef = std::unique_ptr<Actor>;

} // namespace editor


#endif // EDITOR_ACTOR_HPP
