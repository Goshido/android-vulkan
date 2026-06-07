#include <precompiled_headers.hpp>
#include <message_queue.hpp>
#include <native_renderer.hpp>
#include <resource_heap.hpp>
#include <scope_quard.hpp>
#include <static_mesh_component.hpp>
#include <texture2D_storage.hpp>
#include <trace.hpp>
#include <ui_props.hpp>
#include <workspace.hpp>


namespace editor {

namespace {

constexpr size_t PER_MESH_ELEMENTS = 1'000'000U;
constexpr size_t VISIBLE_INITIAL_SIZE = 1'000U;

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

AV_DX_ALIGNMENT_BEGIN

struct Frame final
{
    GXMat4      _viewProj;
};

struct Transform final
{
    GXVec3      _x;
    GXVec3      _y;
    GXVec3      _z;
    GXVec3      _w;
    uint64_t    _normal;
};

struct Shading final
{
    uint32_t        _albedo;
    uint32_t        _emission;
    uint32_t        _mask;
    uint32_t        _param;
    uint32_t        _normal;
    ColorData       _colors;
};

AV_DX_ALIGNMENT_END

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

Workspace* Workspace::_instance = nullptr;

Workspace::Workspace () noexcept
{
    _instance = this;
}

void Workspace::Init () noexcept
{
    AV_TRACE ( "Workspace init" )

    MessageQueue &messageQueue = MessageQueue::Instance ();

    messageQueue.EnqueueBack (
        {
            ._type = eMessageType::UIAddWidget,

            ._action = [] () noexcept {
                auto* dialogBox = new UIProps ();
                dialogBox->SetRect ( Rect ( 44, 444, 133, 333 ) );

                dialogBox->SetMinSize ( pbr::LengthValue ( pbr::LengthValue::eType::PX, 150.0F ),
                    pbr::LengthValue ( pbr::LengthValue::eType::PX, 90.0F ) );
                return dialogBox;
            },

            ._serialNumber = 0U
        }
    );

    _viewport = new ViewportWidget ();

    messageQueue.EnqueueBack (
        {
            ._type = eMessageType::UIAddWidget,

            ._action = [ viewport = _viewport ] () noexcept {
                return viewport;
            },

            ._serialNumber = 0U
        }
    );

    messageQueue.EnqueueBack (
        {
            ._type = eMessageType::InvokeIO,

            ._action = [ this ] () noexcept -> void* {
                AV_TRACE ( "Making Vulkan resources" )

                _opaqueVisible.reserve ( VISIBLE_INITIAL_SIZE );
                _stippleVisible.reserve ( VISIBLE_INITIAL_SIZE );

                std::unique_ptr<pbr::OpaqueProgram> p = std::make_unique<pbr::OpaqueProgram> ();
                android_vulkan::Renderer &renderer = NativeRenderer::Instance ();

                if ( !p->Init ( renderer.GetDevice (), renderer.GetDefaultDepthFormat () ) ) [[unlikely]]
                    return nullptr;

                _opaqueProgram = std::move ( p );

                std::unique_ptr<pbr::StreamBuffer> frame = std::make_unique<pbr::StreamBuffer> ();

                if ( !frame->Init ( renderer, pbr::FIF_COUNT, sizeof ( Frame ), "Frame stream" ) ) [[unlikely]]
                    return nullptr;

                _frameStream = std::move ( frame );

                std::unique_ptr<pbr::StreamBuffer> transform = std::make_unique<pbr::StreamBuffer> ();

                if ( !transform->Init ( renderer, PER_MESH_ELEMENTS, sizeof ( Transform ), "Transform stream" ) )
                {
                    [[unlikely]]
                    return nullptr;
                }

                _transformStream = std::move ( transform );

                std::unique_ptr<pbr::StreamBuffer> shading = std::make_unique<pbr::StreamBuffer> ();

                if ( !shading->Init ( renderer, PER_MESH_ELEMENTS, sizeof ( Shading ), "Shading stream" ) )
                {
                    [[unlikely]]
                    return nullptr;
                }

                _shadingStream = std::move ( shading );
                return nullptr;
            },

            ._serialNumber = 0U
        }
    );

    Texture2DStorage &storage = Texture2DStorage::Instance ();

    storage.Load ( "pbr/system/white.tga",
        [ this ] ( std::optional<Texture2DRef> &&texture ) noexcept {
            _defaultAlbedo = std::move ( *texture );
        }
    );

    storage.Load ( "pbr/system/black-transparent.tga",
        [ this ] ( std::optional<Texture2DRef> &&texture ) noexcept {
            _defaultEmission = std::move ( *texture );
        }
    );

    storage.Load ( "pbr/system/red-transparent.tga",
        [ this ] ( std::optional<Texture2DRef> &&texture ) noexcept {
            _defaultMask = std::move ( *texture );
        }
    );

    storage.Load ( "pbr/system/red-transparent.tga",
        [ this ] ( std::optional<Texture2DRef> &&texture ) noexcept {
            _defaultParam = std::move ( *texture );
        }
    );

    storage.Load ( "pbr/system/normal.tga",
        [ this ] ( std::optional<Texture2DRef> &&texture ) noexcept {
            _defaultNormal = std::move ( *texture );
        }
    );

    FUCK ();
}

void Workspace::Destroy () noexcept
{
    AV_TRACE ( "Workspace destroy" )
    _history.Clear ();
    _actors.clear ();

    if ( _opaqueProgram ) [[likely]]
    {
        _opaqueProgram->Destroy ( NativeRenderer::Instance ().GetDevice () );
        _opaqueProgram.reset ();
    }

    Texture2DStorage &storage = Texture2DStorage::Instance ();
    storage.Unload ( std::move ( _defaultAlbedo ) );
    storage.Unload ( std::move ( _defaultEmission ) );
    storage.Unload ( std::move ( _defaultMask ) );
    storage.Unload ( std::move ( _defaultParam ) );
    storage.Unload ( std::move ( _defaultNormal ) );

    auto const freeStream = [ &renderer = NativeRenderer::Instance () ] (
        std::unique_ptr<pbr::StreamBuffer> &stream
    ) noexcept {
        if ( !stream ) [[unlikely]]
            return;

        stream->Destroy ( renderer );
        stream.reset ();
    };

    freeStream ( _frameStream );
    freeStream ( _transformStream );
    freeStream ( _shadingStream );

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

void Workspace::ComputeTransform ( float deltaTime ) noexcept
{
    if ( !IsReady () ) [[unlikely]]
        return;

    AV_TRACE ( "Compute transforms" )

    _frameStream->Commit ();
    _transformStream->Commit ();
    _shadingStream->Commit ();

    // FUCK - correct DPI
    _viewport->Update ( deltaTime, 1.0F );

    GXQuat const &orientation = _viewport->GetOrientation ();

    GXMat4 alpha {};
    alpha.FromFast ( orientation, _viewport->GetPosition () );

    GXMat4 beta {};
    beta.Inverse ( alpha );

    Frame frame {};
    frame._viewProj.Multiply ( beta, _viewport->GetProjection () );

    GXProjectionClipPlanes frustum {};
    frustum.From ( frame._viewProj );

    auto const traverse = [
        &shadingStream = *_shadingStream,
        &transformStream = *_transformStream,
        &frustum,
        defaultAlbedo = _defaultAlbedo->_heapIndex,
        defaultEmission = _defaultEmission->_heapIndex,
        defaultMask = _defaultMask->_heapIndex,
        defaultParam = _defaultParam->_heapIndex,
        defaultNormal = _defaultNormal->_heapIndex
    ] ( MeshQueue &queue, std::vector<MeshInstance> &queueVisible ) noexcept {
        GXMat4 local;
        GXAABB bounds;
        queueVisible.clear ();

        for ( auto const &instances : queue )
        {
            uint32_t count = 0U;

            for ( auto const &mesh : instances.second )
            {
                GXVec3 const &scale = mesh->_scale;
                GXQuat const &rotation = mesh->_rotation;

                local.From ( rotation, mesh->_location );
                auto &x = *reinterpret_cast<GXVec3*> ( local._data[ 0U ] );
                auto &y = *reinterpret_cast<GXVec3*> ( local._data[ 1U ] );
                x.Multiply ( x, scale._data[ 0U ] );

                auto &z = *reinterpret_cast<GXVec3*> ( local._data[ 2U ] );
                y.Multiply ( y, scale._data[ 1U ] );
                z.Multiply ( z, scale._data[ 2U ] );

                mesh->_boundLocal.Transform ( bounds, local );

                if ( !frustum.IsVisible ( bounds ) )
                    continue;

                ++count;
                PBRMaterial const &material = mesh->_material;

                Shading const shading
                {
                    ._albedo = !material._albedo ? defaultAlbedo : material._albedo->_heapIndex,
                    ._emission = !material._emission ? defaultEmission : material._emission->_heapIndex,
                    ._mask = !material._mask ? defaultMask : material._mask->_heapIndex,
                    ._param = !material._param ? defaultParam : material._param->_heapIndex,
                    ._normal = !material._normal ? defaultNormal : material._normal->_heapIndex,
                    ._colors = mesh->_colors
                };

                shadingStream.Push ( &shading );

                Transform const transform
                {
                    ._x = x,
                    ._y = y,
                    ._z = z,
                    ._w = *reinterpret_cast<GXVec3*> ( local._data[ 3U ] ),
                    ._normal = rotation.ToTBN64 ()
                };

                transformStream.Push ( &transform );
            }

            if ( !count ) [[unlikely]]
                continue;

            queueVisible.push_back ( 
                {
                    ._mesh = instances.first.get (),
                    ._count = count
                }
            );
        }
    };

    traverse ( _opaqueQueue, _opaqueVisible );
    traverse ( _stippleQueue, _stippleVisible );

    if ( !_opaqueVisible.empty () | !_stippleVisible.empty () ) [[likely]]
    {
        _frameStream->Push ( &frame );
    }
}

void Workspace::UploadToGPU ( VkCommandBuffer commandBuffer ) noexcept
{
    if ( !IsReady () ) [[unlikely]]
        return;

    if ( _opaqueVisible.empty () & _stippleVisible.empty () ) [[unlikely]]
        return;

    AV_TRACE ( "Upload workspace data" )
    AV_VULKAN_GROUP ( commandBuffer, "Upload workspace data" )
    _frameStream->IssueSync ( commandBuffer );
    _transformStream->IssueSync ( commandBuffer );
    _shadingStream->IssueSync ( commandBuffer );
}

void Workspace::FillGBuffer ( [[maybe_unused]] VkCommandBuffer commandBuffer ) noexcept
{
    if ( !IsReady () ) [[unlikely]]
        return;

    if ( _opaqueVisible.empty () & _stippleVisible.empty () ) [[unlikely]]
        return;

    AV_TRACE ( "GBuffer" )
    AV_VULKAN_GROUP ( commandBuffer, "GBuffer" )

    VkPipelineLayout layout = _opaqueProgram->GetPipelineLayout ();
    ResourceHeap::Instance ().Bind ( commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, layout );
    _opaqueProgram->Bind ( commandBuffer );

    // FUCK - Do something with PushConstants structure. It is reused for opaque and stipple program.

    auto traverse = [
        &shadingStream = *_shadingStream,
        &transformStream = *_transformStream,
        commandBuffer = commandBuffer,
        layout = layout,

        pushConstants = pbr::OpaqueProgram::PushConstants
        {
            ._frameStream = _frameStream->AcquireAndConsume ( 1U )
        }
    ] ( std::vector<MeshInstance> const &queueVisible ) mutable noexcept {
        for ( auto const &[ mesh, count ] : queueVisible )
        {
            pushConstants._transformStream = transformStream.AcquireAndConsume ( count );
            pushConstants._shadingStream = shadingStream.AcquireAndConsume ( count );

            android_vulkan::MeshBufferInfo const &info = mesh->GetMeshBufferInfo ();
            pushConstants._positionStream = info._bdaStream0;
            pushConstants._restStream = info._bdaStream1;
            pushConstants._indexStream = info._bdaIndex;
            pushConstants._indexType = static_cast<uint32_t> ( info._indexType );

            vkCmdPushConstants ( commandBuffer,
                layout,
                AV_VK_FLAG ( VK_SHADER_STAGE_VERTEX_BIT ) | AV_VK_FLAG ( VK_SHADER_STAGE_FRAGMENT_BIT ),
                0U,
                sizeof ( pushConstants ),
                &pushConstants
            );

            vkCmdDraw ( commandBuffer, mesh->GetVertexCount (), count, 0U, 0U );
        }
    };

    traverse ( _opaqueVisible );
    traverse ( _stippleVisible );
}

void Workspace::DrawGizmo ( [[maybe_unused]] VkCommandBuffer commandBuffer ) noexcept
{
    if ( !IsReady () ) [[unlikely]]
        return;

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

Workspace &Workspace::Instance () noexcept
{
    return *_instance;
}

void Workspace::FUCK () noexcept
{
    ActorRef actor = std::make_unique<Actor> ();
    actor->SetName ( "FUCK" );
    Actor &a = *actor;

    ComponentRef mesh1 = std::make_unique<StaticMeshComponent> ( "meshes/rotating_mesh/sonic-material-1.mesh2",
        "../editor-assets/textures/sonic-material-1-diffuse.png"
    );

    mesh1->SetName ( "mesh #1" );

    ComponentRef mesh2 = std::make_unique<StaticMeshComponent> ( "meshes/rotating_mesh/sonic-material-2.mesh2",
        "../editor-assets/textures/sonic-material-2-diffuse.png"
    );

    mesh2->SetName ( "mesh #2" );

    ComponentRef mesh3 = std::make_unique<StaticMeshComponent> ( "meshes/rotating_mesh/sonic-material-3.mesh2",
        "../editor-assets/textures/sonic-material-3-diffuse.png"
    );

    mesh3->SetName ( "mesh #3" );

    _history.Begin ();
    _history.Append ( std::make_unique<CreateActorAction> ( std::move ( actor ), _actors ) );
    _history.Append ( std::make_unique<AppendComponentAction> ( a, std::move ( mesh1 ) ) );
    _history.Append ( std::make_unique<AppendComponentAction> ( a, std::move ( mesh2 ) ) );
    _history.Append ( std::make_unique<AppendComponentAction> ( a, std::move ( mesh3 ) ) );
    _history.End ();
}

bool Workspace::IsReady () noexcept
{
    if ( _ready ) [[likely]]
        return true;

    _ready = ( static_cast<bool> ( _viewport ) ) &
        ( static_cast<bool> ( _opaqueProgram ) ) &
        ( static_cast<bool> ( _frameStream ) ) &
        ( static_cast<bool> ( _transformStream ) ) &
        ( static_cast<bool> ( _shadingStream ) ) &
        ( static_cast<bool> ( _defaultAlbedo ) ) &
        ( static_cast<bool> ( _defaultEmission ) ) &
        ( static_cast<bool> ( _defaultMask ) ) &
        ( static_cast<bool> ( _defaultParam ) ) &
        ( static_cast<bool> ( _defaultNormal ) ) &
        ( _opaqueVisible.capacity () > 0U ) &
        ( _stippleVisible.capacity () > 0U );

    return _ready;
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
