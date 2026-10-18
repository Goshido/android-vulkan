#ifndef EDITOR_MOVE_ACTOR_ACTION_HPP
#define EDITOR_MOVE_ACTOR_ACTION_HPP


#include "action.hpp"
#include "actor.hpp"


namespace editor {

class MoveActorAction final : public Action
{
    public:
        struct Item final
        {
            Actor*      _actor = nullptr;
            GXVec3      _before {};
            GXVec3      _after {};
        };

        using Items = std::vector<Item>;

    private:
        Items           _items {};

    public:
        MoveActorAction () = delete;

        MoveActorAction ( MoveActorAction const & ) = delete;
        MoveActorAction &operator = ( MoveActorAction const & ) = delete;

        MoveActorAction ( MoveActorAction && ) = default;
        MoveActorAction &operator = ( MoveActorAction && ) = default;

        explicit MoveActorAction ( Items &&items ) noexcept;

        ~MoveActorAction () override = default;

    private:
        void Redo () noexcept override;
        void Undo () noexcept override;
};

} // namespace editor


#endif // EDITOR_MOVE_ACTOR_ACTION_HPP
