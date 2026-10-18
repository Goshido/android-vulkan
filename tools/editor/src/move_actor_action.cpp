#include <precompiled_headers.hpp>
#include <move_actor_action.hpp>
#include <trace.hpp>
#include <workspace.hpp>


namespace editor {

MoveActorAction::MoveActorAction ( Items &&items ) noexcept:
    _items ( std::move ( items ) )
{
    // NOTHING
}

void MoveActorAction::Redo () noexcept
{
    AV_TRACE ( "Redo move" )

    for ( Item &item : _items )
        item._actor->SetLocation ( item._after );

    Workspace::Instance ().OnContentCommitted ();
}

void MoveActorAction::Undo () noexcept
{
    AV_TRACE ( "Undo move" )

    for ( Item &item : _items )
        item._actor->SetLocation ( item._before );

    Workspace::Instance ().OnContentCommitted ();
}

} // namespace editor
