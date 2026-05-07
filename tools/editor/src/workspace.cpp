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

    constexpr auto clearAlpha = [] ( auto &queue, auto &map ) noexcept {
        for ( auto const &item : queue )
        {
            for ( auto const info : item.second )
            {
                delete info;
            }
        }

        map.clear ();
        queue.clear ();
    };

    constexpr auto clearBeta = [] ( auto &queue ) noexcept {
        for ( auto const item : queue )
            delete item;

        queue.clear ();
    };

    std::lock_guard const lock ( _mutex );

    clearAlpha ( _opaqueQueue, _opaqueMap );
    clearAlpha ( _stippleQueue, _stippleMap );
    clearAlpha ( _gizmoQueue, _gizmoMap );

    clearBeta ( _pointLightQueue );
    clearBeta ( _reflectionProbeLocalQueue );
    clearBeta ( _reflectionProbeGlobalQueue );
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

GizmoNode Workspace::RegisterGizmo ( MeshGeometryRef &mesh ) noexcept
{
    AV_TRACE ( "Workspace register gizmo" )

    auto &g = *new GizmoInfo;

    // FUCK move to ipp
    auto w = MeshGeometryRef::weak_type ( mesh );

    {
        std::lock_guard const lock ( _mutex );
        _gizmoQueue[ mesh ].push_back ( &g );
        _gizmoMap[ &g ] = std::move ( w );
    }

    return GizmoNode ( *this, g );
}

PointLightNode Workspace::RegisterPointLight () noexcept
{
    AV_TRACE ( "Workspace register point light" )

    auto &node = *new PointLightInfo;

    {
        std::lock_guard const lock ( _mutex );
        _pointLightQueue.insert ( &node );
    }

    return PointLightNode ( *this, node );
}

ReflectionProbeLocalNode Workspace::RegisterReflectionProbeLocal () noexcept
{
    AV_TRACE ( "Workspace register reflection probe local" )

    auto &node = *new ReflectionProbeLocalInfo;

    {
        std::lock_guard const lock ( _mutex );
        _reflectionProbeLocalQueue.insert ( &node );
    }

    return ReflectionProbeLocalNode ( *this, node );
}

ReflectionProbeGlobalNode Workspace::RegisterReflectionProbeGlobal () noexcept
{
    AV_TRACE ( "Workspace register reflection probe global" )

    auto &node = *new ReflectionProbeGlobalInfo;

    {
        std::lock_guard const lock ( _mutex );
        _reflectionProbeGlobalQueue.insert ( &node );
    }

    return ReflectionProbeGlobalNode ( *this, node );
}

void Workspace::Unregister ( MeshNode const &node ) noexcept
{
    AV_TRACE ( "Workspace unregister opaque mesh" )
    MeshInfo const &n = node.GetInternalInfo ();

    if ( n._material._isStipple )
        Unregister ( _stippleQueue, _stippleMap, n );
    else
        Unregister ( _opaqueQueue, _opaqueMap, n );

    delete &n;
}

// FUCK move to ipp
void Workspace::Unregister ( GizmoNode const &node ) noexcept
{
    GizmoInfo const &g = node.GetInternalInfo ();

    std::lock_guard const lock ( _mutex );
    auto const findResult = _gizmoQueue.find ( _gizmoMap.extract ( &g ).mapped ().lock () );
    Gizmos &gizmos = findResult->second;

    if ( gizmos.size () == 1U )
    {
        AV_ASSERT ( gizmos.front () == &g )
        _gizmoQueue.erase ( findResult );
        return;
    }

    for ( auto &gizmo : gizmos )
    {
        if ( gizmo != &g )
            continue;

        gizmo = gizmos.back ();
        gizmos.pop_back ();
        break;
    }
}

void Workspace::Unregister ( PointLightNode &node ) noexcept
{
    AV_TRACE ( "Workspace unregister point light" )
    PointLightInfo &n = node.GetInternalInfo ();

    {
        std::lock_guard const lock ( _mutex );
        _pointLightQueue.erase ( &n );
    }

    delete &n;
}

void Workspace::Unregister ( ReflectionProbeLocalNode &node ) noexcept
{
    AV_TRACE ( "Workspace unregister reflection probe local" )
    ReflectionProbeLocalInfo &n = node.GetInternalInfo ();

    {
        std::lock_guard const lock ( _mutex );
        _reflectionProbeLocalQueue.erase ( &n );
    }

    delete &n;
}

void Workspace::Unregister ( ReflectionProbeGlobalNode &node ) noexcept
{
    AV_TRACE ( "Workspace unregister reflection probe global" )
    ReflectionProbeGlobalInfo &n = node.GetInternalInfo ();

    {
        std::lock_guard const lock ( _mutex );
        _reflectionProbeGlobalQueue.erase ( &n );
    }

    delete &n;
}

void Workspace::DrawOpaque ( [[maybe_unused]] VkCommandBuffer commandBuffer )
{
    AV_TRACE ( "Opaque" )
    AV_VULKAN_GROUP ( commandBuffer, "Opaque" )
    // FUCK
}

void Workspace::DrawGizmo ( [[maybe_unused]] VkCommandBuffer commandBuffer )
{
    AV_TRACE ( "Gizmo" )
    AV_VULKAN_GROUP ( commandBuffer, "Gizmo" )
    // FUCK
}

void Workspace::DrawUI ( [[maybe_unused]] VkCommandBuffer commandBuffer )
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
