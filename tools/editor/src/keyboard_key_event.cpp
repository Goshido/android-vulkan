#include <precompiled_headers.hpp>
#include <keyboard_key_event.hpp>

GX_DISABLE_COMMON_WARNINGS

#include <variant>

GX_RESTORE_WARNING_STATE


namespace editor {

// Sanity checks
static_assert ( sizeof ( uint8_t ) >= sizeof ( KeyModifier ) );
static_assert ( sizeof ( void* ) >= sizeof ( eKey ) + sizeof ( uint8_t ) );

//----------------------------------------------------------------------------------------------------------------------

KeyboardKeyEvent::KeyboardKeyEvent ( Message const &message ) noexcept
{
    // Avoiding any type punning. Using std::bit_cast to proper repack bits from void* type into eKey + KeyModifier.
    auto const pack = std::bit_cast<size_t> ( message._params );
    _key = static_cast<eKey> ( pack >> 16U );
    _modifier = std::bit_cast<KeyModifier> ( static_cast<uint8_t> ( pack & 0xFFU ) );
}

Message KeyboardKeyEvent::Create ( eMessageType messageType, eKey key, KeyModifier modifier ) noexcept
{
    // Avoiding any type punning. Using std::bit_cast to proper repack bits from eKey + KeyModifier into void* type.

    return {
        ._type = messageType,

        ._params = std::bit_cast<void*> (
            ( static_cast<size_t> ( key ) << 16U ) | static_cast<size_t> ( std::bit_cast<uint8_t> ( modifier ) )
        ),

        ._serialNumber = 0U
    };
}

} // namespace editor
