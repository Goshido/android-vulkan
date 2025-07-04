#include <precompiled_headers.hpp>
#include <av_assert.hpp>
#include <history.hpp>


namespace editor {

History::Transaction::~Transaction () noexcept
{
    for ( auto &action : std::ranges::reverse_view ( _actions ) )
    {
        action.reset ();
    }
}

void History::Transaction::Append ( std::unique_ptr<Action> &&action ) noexcept
{
    _actions.push_back ( std::move ( action ) );
}

void History::Transaction::Redo () noexcept
{
    for ( auto &action : _actions )
    {
        action->Redo ();
    }
}

void History::Transaction::Undo () noexcept
{
    for ( auto &action : std::ranges::reverse_view ( _actions ) )
    {
        action->Undo ();
    }
}

//----------------------------------------------------------------------------------------------------------------------

History::~History () noexcept
{
    Clear ();
}

void History::Begin () noexcept
{
    AV_ASSERT ( !_active )
    _active = std::optional<Transaction> {};
}

void History::Append ( std::unique_ptr<Action> &&action ) noexcept
{
    AV_ASSERT ( _active )
    _active->Append ( std::move ( action ) );
}

void History::End () noexcept
{
    AV_ASSERT ( _active )
    bool const eraseAll = _undo == _transactions.cend ();

    while ( !_transactions.empty () & ( eraseAll || _undo != _transactions.cend () - 1U ) )
        _transactions.pop_back ();

    Transaction &transaction = *_active;
    transaction.Redo ();

    _transactions.push_back ( std::move ( transaction ) );
    _redo = _transactions.end ();
    _undo = _redo - 1U;

    _active = std::nullopt;
}

void History::Clear () noexcept
{
    while ( !_transactions.empty () )
        _transactions.pop_back ();

    _redo = _transactions.end ();
    _undo = _redo;
}

void History::Redo () noexcept
{
    if ( _redo != _transactions.cend () ) [[likely]]
    {
        _redo->Redo ();
        _undo = _redo++;
    }
}

void History::Undo () noexcept
{
    auto const end = _transactions.end ();

    if ( _undo == end ) [[unlikely]]
        return;

    _undo->Undo ();

    if ( _undo == _transactions.cbegin () ) [[unlikely]]
    {
        _undo = end;
        return;
    }

    --_undo;
}

} // namespace editor
