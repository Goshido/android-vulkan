#ifndef EDITOR_HISTORY_HPP
#define EDITOR_HISTORY_HPP


#include "action.hpp"
#include <memory>


namespace editor {

class History final
{
    private:
        class Transaction final
        {
            private:
                std::deque<std::unique_ptr<Action>>     _actions {};

            public:
                explicit Transaction () = default;

                Transaction ( Transaction const & ) = delete;
                Transaction &operator = ( Transaction const & ) = delete;

                Transaction ( Transaction && ) = default;
                Transaction &operator = ( Transaction && ) = default;

                ~Transaction () noexcept;

                void Append ( std::unique_ptr<Action> &&action ) noexcept;

                void Redo () noexcept;
                void Undo () noexcept;
        };

        using Transactions = std::deque<Transaction>;
        using Pointer = Transactions::iterator;

    private:
        std::optional<Transaction>                      _active {};
        Transactions                                    _transactions {};
        Pointer                                         _redo = _transactions.end ();
        Pointer                                         _undo = _redo;

    public:
        explicit History () = default;

        History ( History const & ) = delete;
        History &operator = ( History const & ) = delete;

        History ( History && ) = delete;
        History &operator = ( History && ) = delete;

        ~History () noexcept;

        void Begin () noexcept;
        void Append ( std::unique_ptr<Action> &&action ) noexcept;
        void End () noexcept;

        void Clear () noexcept;

        void Redo () noexcept;
        void Undo () noexcept;
};

} // namespace editor


#endif // EDITOR_HISTORY_HPP
