#include <precompiled_headers.hpp>
#include <message.hpp>


namespace editor {

Message::Message ( eMessageType type ) noexcept:
    _type ( type )
{
    // NOTHING
}

Message::Message ( eMessageType type, Action &&action ) noexcept:
    _type ( type ),
    _action ( std::move ( action ) )
{
    // NOTHING
}

Message::Message ( eMessageType type, Action &&action, SerialNumber serialNumber ) noexcept:
    _type ( type ),
    _action ( std::move ( action ) ),
    _serialNumber ( serialNumber )
{
    // NOTHING
}

} // namespace editor
