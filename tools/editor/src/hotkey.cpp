#include <precompiled_headers.hpp>
#include <hotkey.hpp>


namespace editor {

size_t Hotkey::Hasher::operator () ( Hotkey::Trigger const &me ) const noexcept
{
    // Hash function is based on Boost implementation:
    // https://www.boost.org/doc/libs/1_55_0/doc/html/hash/reference.html#boost.hash_combine

    size_t hash = 0U;

    auto const hashCombine = [ this, &hash ] ( uint16_t v ) noexcept
    {
        constexpr size_t magic = 0x9E3779B9U;
        hash ^= _hashServer ( v ) + magic + ( hash << 6U ) + ( hash >> 2U );
    };

    hashCombine (
        static_cast<uint16_t> (
            static_cast<uint32_t> ( me._alt ) |
            static_cast<uint32_t> ( static_cast<uint8_t> ( me._ctrl ) << 1U ) |
            static_cast<uint32_t> ( static_cast<uint8_t> ( me._shift ) << 2U )
        )
    );

    hashCombine ( static_cast<uint16_t> ( me._key ) );
    return hash;
}

//----------------------------------------------------------------------------------------------------------------------

Hotkey::Hotkeys Hotkey::_hotkeys {};
std::recursive_mutex Hotkey::_mutex {};

Hotkey &Hotkey::operator = ( Hotkey &&other ) noexcept
{
    if ( ( this == &other ) | ( ( _trigger._key == NO_KEY ) & ( other._trigger._key == NO_KEY ) ) ) [[unlikely]]
        return *this;

    std::lock_guard const lock ( _mutex );

    if ( _trigger._key != NO_KEY )
        Unregister ();

    if ( other._trigger._key != NO_KEY )
    {
        Handlers &handlers = _hotkeys[ other._trigger ];

        std::find_if ( handlers.begin (),
            handlers.end (),

            [ target = &other ] ( Handler const &element ) noexcept -> bool {
                return element.first == target;
            }
        )->first = this;
    }

    _trigger = std::exchange ( other._trigger, {} );
    return *this;
}

Hotkey::Hotkey ( eKey key, bool alt, bool ctrl, bool shift, Action &&action ) noexcept
{
    _trigger._key = key;
    _trigger._alt = alt;
    _trigger._ctrl = ctrl;
    _trigger._shift = shift;

    std::lock_guard const lock ( _mutex );
    _hotkeys[ _trigger ].push_back ( std::make_pair ( this, std::move ( action ) ) );
}

Hotkey::~Hotkey () noexcept
{
    if ( _trigger._key != NO_KEY )
    {
        std::lock_guard const lock ( _mutex );
        Unregister ();
    }
}

void Hotkey::Process ( eKey key, KeyModifier modifier ) noexcept
{
    Trigger trigger;
    trigger._key = key;
    trigger._alt = modifier.AnyAltPressed ();
    trigger._ctrl = modifier.AnyCtrlPressed ();
    trigger._shift = modifier.AnyShiftPressed ();

    std::lock_guard const lock ( _mutex );

    if ( auto const findResult = _hotkeys.find ( trigger ); findResult != _hotkeys.cend () ) [[likely]]
    {
        findResult->second.back ().second ();
    }
}

void Hotkey::Unregister () noexcept
{
    auto hotkey = _hotkeys.find ( _trigger );
    Handlers &handlers = hotkey->second;

    handlers.erase (
        std::find_if ( handlers.cbegin (),
            handlers.cend (),

            [ me = this ] ( Handler const &element ) noexcept -> bool {
                return element.first == me;
            }
        )
    );

    if ( handlers.empty () ) [[likely]]
    {
        _hotkeys.erase ( hotkey );
    }
}

} // namespace editor
