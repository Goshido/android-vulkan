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
        using Components = std::deque<Component::Ref>;

    private:
        Components      _components {};
        GXMat4          _local {};
        std::string     _name {};

    public:
        explicit Actor () noexcept;

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
