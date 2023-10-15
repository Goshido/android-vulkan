#include <pbr/node_link.hpp>
#include <logger.hpp>


namespace pbr {

void NodeLink::RegisterNode ( NodeLink* node ) noexcept
{
    if ( std::find ( _nodes.cbegin (), _nodes.cend (), node ) == _nodes.cend () )
    {
        _nodes.push_back ( node );
        return;
    }

    android_vulkan::LogWarning ( "pbr::NodeLink::RegisterNode - Node already exists." );
}

void NodeLink::UnregisterNode ( NodeLink* node ) noexcept
{
    if ( auto const findResult = std::find ( _nodes.cbegin (), _nodes.cend (), node ); findResult  != _nodes.cend () )
    {
        _nodes.erase ( findResult );
        return;
    }

    android_vulkan::LogWarning ( "pbr::NodeLink::UnregisterNode - Node does not exist." );
}

void NodeLink::UnregisterSelf () noexcept
{
    for ( auto* node : _nodes )
    {
        node->RegisterNode ( this );
    }
}

} // namespace pbr
