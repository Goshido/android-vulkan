#include <precompiled_headers.hpp>
#include <property.hpp>


namespace editor {

Property::Connection Property::Connect ( Handler &&handler ) noexcept
{
    _connections.insert ( std::pair ( _id, std::move ( handler ) ) );
    return _id++;
}

void Property::Disconnect ( Connection connection ) noexcept
{
    _connections.erase ( connection );
}

void Property::OnUpdated () noexcept
{
    for ( auto &handler : _connections )
    {
        handler.second ();
    }
}

} // namespace editor
