#include <precompiled_headers.hpp>
#include <scope_quard.hpp>
#include "static_mesh_component.hpp"
#include <trace.hpp>
#include <workspace.hpp>


namespace editor {

namespace {

class CreateActorAction final : public Action
{
    private:
        std::unordered_map<Actor const*, ActorRef>      *_actors = nullptr;
        ActorRef                                        _actor {};
        Actor*                                          _key = _actor.get ();

    public:
        explicit CreateActorAction ( ActorRef &&actor, std::unordered_map<Actor const*, ActorRef> &actors ) noexcept;

        CreateActorAction ( CreateActorAction const & ) = delete;
        CreateActorAction &operator = ( CreateActorAction const & ) = delete;

        CreateActorAction ( CreateActorAction && ) = default;
        CreateActorAction &operator = ( CreateActorAction && ) = default;

        ~CreateActorAction () override = default;

    private:
        void Redo () noexcept override;
        void Undo () noexcept override;
};

CreateActorAction::CreateActorAction ( ActorRef &&actor, std::unordered_map<Actor const*, ActorRef> &actors ) noexcept:
    _actors ( &actors ),
    _actor ( std::move ( actor ) )
{
    // NOTHING
}

void CreateActorAction::Redo () noexcept
{
    _actors->insert ( std::pair ( _key, std::move ( _actor ) ) );
}

void CreateActorAction::Undo () noexcept
{
    auto findResult = _actors->find ( _key );
    _actor = std::move ( findResult->second );
    _actors->erase ( findResult );
}

//----------------------------------------------------------------------------------------------------------------------

class AppendComponentAction final : public Action
{
    private:
        Actor*          _actor = nullptr;
        ComponentRef    _component {};
        Component*      _key = _component.get ();

    public:
        explicit AppendComponentAction ( Actor &actor, ComponentRef &&compoent ) noexcept;

        AppendComponentAction ( AppendComponentAction const & ) = delete;
        AppendComponentAction &operator = ( AppendComponentAction const & ) = delete;

        AppendComponentAction ( AppendComponentAction && ) = default;
        AppendComponentAction &operator = ( AppendComponentAction && ) = default;

        ~AppendComponentAction () override = default;

    private:
        void Redo () noexcept override;
        void Undo () noexcept override;
};

AppendComponentAction::AppendComponentAction ( Actor &actor, ComponentRef &&compoent ) noexcept:
    _actor ( &actor ),
    _component ( std::move ( compoent ) )
{
    // NOTHING
}

void AppendComponentAction::Redo () noexcept
{
    _actor->Append ( std::move ( _component ) );
}

void AppendComponentAction::Undo () noexcept
{
    _component = _actor->Remove ( *_key );
}

} // end of anonymous namespace

//----------------------------------------------------------------------------------------------------------------------

Workspace::Workspace ( MessageQueue& messageQueue ) noexcept:
    _messageQueue ( messageQueue )
{
    // NOTHING
}

void Workspace::Init () noexcept
{
    AV_TRACE ( "Workspace init" )
    // FUCK
    FUCK ();
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

void Workspace::Load ( std::string_view /*scene*/ ) noexcept
{
    // FUCK
}

void Workspace::Close () noexcept
{
    // FUCK
}

void Workspace::DrawOpaque ( [[maybe_unused]] VkCommandBuffer commandBuffer ) noexcept
{
    AV_TRACE ( "Opaque" )
    AV_VULKAN_GROUP ( commandBuffer, "Opaque" )
    // FUCK
}

void Workspace::DrawGizmo ( [[maybe_unused]] VkCommandBuffer commandBuffer,
    [[maybe_unused]] size_t commandBufferIndex
) noexcept
{
    AV_TRACE ( "Gizmo" )
    AV_VULKAN_GROUP ( commandBuffer, "Gizmo" )
    // FUCK
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

    // This will also act as search key for unregister operation.
    auto* nodeMeshInfo = new MeshInfo;

    nodeMeshInfo->_material._isStipple = false;
    return RegisterMesh ( mesh, _opaqueQueue, _opaqueMap, *nodeMeshInfo );
}

MeshNode Workspace::RegisterStippleMesh ( MeshGeometryRef &mesh ) noexcept
{
    AV_TRACE ( "Workspace register stipple mesh" )

    // This will also act as search key for unregister operation.
    auto &m = *new MeshInfo;

    m._material._isStipple = true;
    return RegisterMesh ( mesh, _stippleQueue, _stippleMap, m );
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
    MeshInfo const &n = node.GetMeshInfo ();

    if ( n._material._isStipple )
        UnregisterMesh ( _stippleQueue, _stippleMap, n );
    else
        UnregisterMesh ( _opaqueQueue, _opaqueMap, n );

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

void Workspace::FUCK () noexcept
{
    // FUCK
    ActorRef actor = std::make_unique<Actor> ();
    actor->SetName ( "FUCK" );
    Actor &a = *actor;

    ComponentRef mesh1 = std::make_unique<StaticMeshComponent> ( _messageQueue,
        "meshes/rotating_mesh/sonic-material-1.mesh2"
    );

    mesh1->SetName ( "mesh #1" );

    ComponentRef mesh2 = std::make_unique<StaticMeshComponent> ( _messageQueue,
        "meshes/rotating_mesh/sonic-material-2.mesh2"
    );

    mesh2->SetName ( "mesh #2" );

    ComponentRef mesh3 = std::make_unique<StaticMeshComponent> ( _messageQueue,
        "meshes/rotating_mesh/sonic-material-3.mesh2"
    );

    mesh3->SetName ( "mesh #3" );

    _history.Begin ();
    _history.Append ( std::make_unique<CreateActorAction> ( std::move ( actor ), _actors ) );
    _history.Append ( std::make_unique<AppendComponentAction> ( a, std::move ( mesh1 ) ) );
    _history.Append ( std::make_unique<AppendComponentAction> ( a, std::move ( mesh2 ) ) );
    _history.Append ( std::make_unique<AppendComponentAction> ( a, std::move ( mesh3 ) ) );
    _history.End ();
}

MeshNode Workspace::RegisterMesh ( MeshGeometryRef &mesh,
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

void Workspace::UnregisterMesh ( MeshQueue &meshQueue, MeshMap &meshMap, MeshInfo const &nodeMeshInfo ) noexcept
{
    std::lock_guard const lock ( _mutex );
    auto const findResult = _opaqueQueue.find ( meshMap.extract ( &nodeMeshInfo ).mapped ().lock () );
    Meshes &meshes = findResult->second;

    android_vulkan::ScopeGuard const freeMeshInfo (
        [ &nodeMeshInfo ] () noexcept {
            // It was allocated in RegisterOpaqueMesh|RegisterStippleMesh.
            delete &nodeMeshInfo;
        }
    );

    if ( meshes.size () == 1U )
    {
        AV_ASSERT ( meshes.front () == &nodeMeshInfo )
        meshQueue.erase ( findResult );
        return;
    }

    for ( auto &mesh : meshes )
    {
        if ( mesh != &nodeMeshInfo )
            continue;

        mesh = meshes.back ();
        meshes.pop_back ();
        return;
    }
}

} // namespace editor
