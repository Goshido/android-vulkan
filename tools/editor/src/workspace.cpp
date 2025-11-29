#include <precompiled_headers.hpp>
#include <trace.hpp>
#include <workspace.hpp>


namespace editor {

Workspace::Workspace ( MessageQueue& messageQueue ) noexcept:
    _messageQueue ( messageQueue )
{
    // NOTHING
}

void Workspace::Init () noexcept
{
    AV_TRACE ( "Workspace init" )
    // FUCK
}

void Workspace::Destroy () noexcept
{
    AV_TRACE ( "Workspace destroy" )

    constexpr auto clear = [] ( MeshQueue &meshQueue, MeshMap &meshMap ) noexcept {
        for ( auto const &item : meshQueue )
        {
            for ( auto const mesh : item.second )
            {
                delete mesh;
            }
        }

        meshMap.clear ();
        meshQueue.clear ();
    };

    std::lock_guard const lock ( _mutex );
    clear ( _opaqueQueue, _opaqueMap );
    clear ( _stippleQueue, _stippleMap );
}

void Workspace::Draw ( VkCommandBuffer commandBuffer ) noexcept
{
    AV_TRACE ( "Workspace draw" )
    AV_VULKAN_GROUP ( commandBuffer, "Workspace draw" )

    DrawOpaque ( commandBuffer );
    DrawGizmo ( commandBuffer );
    DrawUI ( commandBuffer );
}

void Workspace::Pick ( int32_t /*x*/, int32_t /*y*/, GXMat4 const &/*viewer*/, GXMat4 const &/*projection*/ ) noexcept
{
    AV_TRACE ( "Workspace pick" )
    // FUCK
}

void Workspace::Pick ( Rect const &/*rect*/, GXMat4 const &/*viewer*/, GXMat4 const &/*projection*/ ) noexcept
{
    AV_TRACE ( "Workspace pick" )
    // FUCK
}

MeshNode Workspace::RegisterOpaqueMesh ( MeshGeometryRef &mesh ) noexcept
{
    AV_TRACE ( "Workspace register opaque mesh" )

    auto* m = new MeshInfo;
    m->_material._isStipple = false;
    return Register ( mesh, _opaqueQueue, _opaqueMap, *m );
}

MeshNode Workspace::RegisterStippleMesh ( MeshGeometryRef &mesh ) noexcept
{
    AV_TRACE ( "Workspace register stipple mesh" )

    auto &m = *new MeshInfo;
    m._material._isStipple = true;
    return Register ( mesh, _stippleQueue, _stippleMap, m );
}

void Workspace::Unregister ( MeshNode const &node ) noexcept
{
    AV_TRACE ( "Workspace unregister opaque mesh" )
    MeshInfo const &m = node.GetInternalMeshInfo ();

    if ( m._material._isStipple )
        Unregister ( _stippleQueue, _stippleMap, m );
    else
        Unregister ( _opaqueQueue, _opaqueMap, m );

    delete &m;
}

void Workspace::DrawOpaque ( VkCommandBuffer commandBuffer )
{
    AV_TRACE ( "Opaque" )
    AV_VULKAN_GROUP ( commandBuffer, "Opaque" )
    // FUCK
}

void Workspace::DrawGizmo ( VkCommandBuffer commandBuffer )
{
    AV_TRACE ( "Gizmo" )
    AV_VULKAN_GROUP ( commandBuffer, "Gizmo" )
    // FUCK
}

void Workspace::DrawUI ( VkCommandBuffer commandBuffer )
{
    AV_TRACE ( "UI" )
    AV_VULKAN_GROUP ( commandBuffer, "UI" )
    // FUCK
}

MeshNode Workspace::Register ( MeshGeometryRef &mesh,
    MeshQueue &meshQueue,
    MeshMap &meshMap,
    MeshInfo &node
) noexcept
{
    auto w = MeshGeometryRef::weak_type ( mesh );

    {
        std::lock_guard const lock ( _mutex );
        meshQueue[ mesh ].push_back ( &node );
        meshMap[ &node ] = std::move ( w );
    }

    return MeshNode ( *this, node );
}

void Workspace::Unregister ( MeshQueue &meshQueue, MeshMap &meshMap, MeshInfo const &node ) noexcept
{
    std::lock_guard const lock ( _mutex );
    auto const findResult = _opaqueQueue.find ( meshMap.extract ( &node ).mapped ().lock () );
    Meshes &meshes = findResult->second;

    if ( meshes.size () == 1U )
    {
        AV_ASSERT ( meshes.front () == &node )
        meshQueue.erase ( findResult );
        return;
    }

    for ( auto &mesh : meshes )
    {
        if ( mesh != &node )
            continue;

        mesh = meshes.back ();
        meshes.pop_back ();
        break;
    }
}

} // namespace editor
