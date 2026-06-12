#ifndef EDITOR_HOTKEY_HPP
#define EDITOR_HOTKEY_HPP


#include "keys.hpp"

GX_DISABLE_COMMON_WARNINGS

#include <deque>
#include <functional>
#include <mutex>
#include <unordered_map>

GX_RESTORE_WARNING_STATE


namespace editor {

class Hotkey final
{
    public:
        using Action = std::move_only_function<void ()>;

    private:
        constexpr static eKey NO_KEY = eKey::LeftMouseButton;

        struct Trigger final
        {
            eKey                        _key = NO_KEY;

            bool                        _alt : 1 = false;
            bool                        _ctrl : 1 = false;
            bool                        _shift : 1 = false;

            // Needed for direct bitfield comparison
            [[nodiscard]] bool operator == ( Trigger const & ) const = default;
        };

        class Hasher final
        {
            private:
                std::hash<uint16_t>     _hashServer {};

            public:
                Hasher () = default;

                Hasher ( Hasher const & ) = default;
                Hasher &operator = ( Hasher const & ) = delete;

                Hasher ( Hasher && ) = default;
                Hasher &operator = ( Hasher && ) = delete;

                ~Hasher () = default;

                // hash function. std::unordered_map requirement.
                [[nodiscard]] size_t operator () ( Trigger const &me ) const noexcept;
        };

        using Handler = std::pair<Hotkey*, Action>;
        using Handlers = std::deque<Handler>;
        using Hotkeys = std::unordered_map<Trigger, Handlers, Hasher>;

    private:
        Trigger                         _trigger {};

        static Hotkeys                  _hotkeys;
        static std::recursive_mutex     _mutex;

    public:
        Hotkey () = default;

        Hotkey ( Hotkey const & ) = delete;
        Hotkey &operator = ( Hotkey const & ) = delete;

        Hotkey ( Hotkey &&other ) = delete;
        Hotkey &operator = ( Hotkey &&other ) noexcept;

        explicit Hotkey ( eKey key, bool alt, bool ctrl, bool shift, Action &&action ) noexcept;

        ~Hotkey () noexcept;

        static void Process ( eKey key, KeyModifier modifier ) noexcept;

    private:
        void Unregister () noexcept;
};

} // namespace editor


#endif // EDITOR_HOTKEY_HPP
