#include <precompiled_headers.hpp>
#include <property.hpp>


namespace editor {

Property::Connection Property::Connect ( Handler &&handler ) noexcept
{
    _connections.push_back ( std::move ( handler ) );
    return _connections.size () - 1UZ;
}

void Property::Disconnect ( Connection connection ) noexcept
{
    _connections.erase ( _connections.cbegin () + static_cast<ptrdiff_t> ( connection ) );
}

void Property::OnUpdated () noexcept
{
    for ( auto &handler : _connections )
    {
        handler ();
    }
}

} // namespace editor
