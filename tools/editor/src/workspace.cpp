#include <precompiled_headers.hpp>
#include <native_renderer.hpp>
#include <platform/windows/pbr/universal_pipeline_layout.hpp>
#include <program_info.hpp>
#include <shading.hpp>
#include <static_mesh_component.hpp>
#include <stream_buffer_info.hpp>
#include <texture2D_storage.hpp>
#include <trace.hpp>
#include <transform.hpp>
#include <ui_props.hpp>
#include <vulkan_utils.hpp>
#include <workspace.hpp>


namespace editor {

namespace {

constexpr size_t PER_MESH_ELEMENTS = 1'000'000UZ;
constexpr size_t VISIBLE_INITIAL_SIZE = 1'000UZ;

//----------------------------------------------------------------------------------------------------------------------

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

    InitWidgets ();
    InitGraphicsResources ();
    InitHotkeys ();

    FUCK ();
}

void Workspace::Destroy () noexcept
{
    AV_TRACE ( "Workspace destroy" )

    _viewport->Destroy ();

    MessageQueue &messageQueue = MessageQueue::Instance ();
    android_vulkan::Renderer &renderer = NativeRenderer::Instance ();
    _selection.Destroy ( messageQueue, renderer );

    _delete = {};
    _openWorkspace = {};
    _saveWorkspace = {};
    _saveAsWorkspace = {};

    _history.Clear ();
    _actors.clear ();

    if ( _opaqueProgram ) [[likely]]
    {
        MessageQueue::Instance ().EnqueueBack (
            Message ( eMessageType::DestroyProgram,
                [ program = ProgramRef ( _opaqueProgram.release () ) ] () mutable noexcept -> void* {
                    return &program;
                }
            )
        );
    }

    if ( _opaqueWithIDProgram ) [[likely]]
    {
        MessageQueue::Instance ().EnqueueBack (
            Message ( eMessageType::DestroyProgram,
                [ program = ProgramRef ( _opaqueWithIDProgram.release () ) ] () mutable noexcept -> void* {
                    return &program;
                }
            )
        );
    }

    if ( _frameStream ) [[likely]]
    {
        MessageQueue::Instance ().EnqueueBack (
            Message ( eMessageType::DestroyStreamBuffer,
                [ stream = std::move ( _frameStream ) ] () mutable noexcept -> void* {
                    return &stream;
                }
            )
        );
    }

    if ( _transformStream ) [[likely]]
    {
        MessageQueue::Instance ().EnqueueBack (
            Message ( eMessageType::DestroyStreamBuffer,
                [ stream = std::move ( _transformStream ) ] () mutable noexcept -> void* {
                    return &stream;
                }
            )
        );
    }

    if ( _shadingStream ) [[likely]]
    {
        MessageQueue::Instance ().EnqueueBack (
            Message ( eMessageType::DestroyStreamBuffer,
                [ stream = std::move ( _shadingStream ) ] () mutable noexcept -> void* {
                    return &stream;
                }
            )
        );
    }

    if ( _idStream ) [[likely]]
    {
        MessageQueue::Instance ().EnqueueBack (
            Message ( eMessageType::DestroyStreamBuffer,
                [ stream = std::move ( _idStream ) ] () mutable noexcept -> void* {
                    return &stream;
                }
            )
        );
    }

    Texture2DStorage &storage = Texture2DStorage::Instance ();
    storage.Unload ( std::move ( _defaultAlbedo ) );
    storage.Unload ( std::move ( _defaultEmission ) );
    storage.Unload ( std::move ( _defaultMask ) );
    storage.Unload ( std::move ( _defaultParam ) );
    storage.Unload ( std::move ( _defaultNormal ) );

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

void Workspace::UploadGPUData ( VkCommandBuffer commandBuffer, float deltaTime ) noexcept
{
    if ( !IsReady () ) [[unlikely]]
        return;

    Frame frame{};
    bool const pendingSelect = _selection.IsSelectionRequested ();

    {
        AV_TRACE ( "Compute transforms" )

        _frameStream->Commit ();
        _transformStream->Commit ();
        _shadingStream->Commit ();

        // FUCK - correct DPI
        _viewport->Update ( deltaTime, 1.0F );

        GXQuat const& orientation = _viewport->GetOrientation ();

        GXMat4 alpha{};
        alpha.FromFast ( orientation, _viewport->GetPosition () );

        GXMat4 beta{};
        beta.Inverse ( alpha );
        frame._viewProj.Multiply ( beta, _viewport->GetProjection () );

        GXProjectionClipPlanes frustum{};
        frustum.From ( frame._viewProj );

        if ( pendingSelect ) [[unlikely]]
        {
            ComputeTransformGBufferWithID ( frustum );
        }
        else
        {
            ComputeTransformGBufferOnly ( frustum );
        }
    }

    if ( _opaqueVisible.empty () & _stippleVisible.empty () ) [[unlikely]]
        return;

    _frameStream->Push ( &frame );

    {
        AV_TRACE ( "Upload workspace data" )
        AV_VULKAN_GROUP ( commandBuffer, "Upload workspace data" )
        _frameStream->IssueSync ( commandBuffer );
        _transformStream->IssueSync ( commandBuffer );
        _shadingStream->IssueSync ( commandBuffer );

        if ( pendingSelect ) [[unlikely]]
        {
            _idStream->IssueSync ( commandBuffer );
        }
    }
}

void Workspace::PrepareIDBuffer ( VkCommandBuffer commandBuffer ) noexcept
{
    if ( IsReady () ) [[likely]]
    {
        _selection.PrepareIDBuffer ( commandBuffer );
    }
}

void Workspace::FillGBuffer ( [[maybe_unused]] VkCommandBuffer commandBuffer ) noexcept
{
    if ( !IsReady () ) [[unlikely]]
        return;

    if ( _opaqueVisible.empty () & _stippleVisible.empty () ) [[unlikely]]
        return;

    if ( _selection.IsSelectionRequested () ) [[unlikely]]
    {
        FillGBufferWithID ( commandBuffer );
        return;
    }

    FillGBufferOnly ( commandBuffer );
}

void Workspace::DrawGizmo ( [[maybe_unused]] VkCommandBuffer commandBuffer ) noexcept
{
    if ( !IsReady () ) [[unlikely]]
        return;

    AV_TRACE ( "Gizmo" )
    AV_VULKAN_GROUP ( commandBuffer, "Gizmo" )
    // FUCK
}

void Workspace::OnGBufferResolutionChanged ( android_vulkan::Texture2D &idImage, uint32_t idResourceIdx ) noexcept
{
    _selection.OnGBufferResolutionChanged ( idImage, idResourceIdx );
}

void Workspace::ComputeSelect ( VkCommandBuffer commandBuffer ) noexcept
{
    if ( IsReady () ) [[likely]]
    {
        _selection.ComputeSelect ( commandBuffer );
    }
}

Selection &Workspace::GetSelection () noexcept
{
    return _selection;
}

MeshNode Workspace::RegisterOpaqueMesh ( MeshGeometryRef &mesh ) noexcept
{
    AV_TRACE ( "Workspace register opaque mesh" )

    // This will also act as search key for unregister operation.
    auto &m = *new MeshInfo;

    m._isStipple = false;
    return RegisterMesh ( mesh, _opaqueQueue, _opaqueMap, m );
}

MeshNode Workspace::RegisterStippleMesh ( MeshGeometryRef &mesh ) noexcept
{
    AV_TRACE ( "Workspace register stipple mesh" )

    // This will also act as search key for unregister operation.
    auto &m = *new MeshInfo;

    m._isStipple = true;
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

void Workspace::Unregister ( MeshNode &node ) noexcept
{
    AV_TRACE ( "Workspace unregister opaque mesh" )
    node._workspace = nullptr;
    MeshInfo &n = *node._meshInfo;

    if ( n._isStipple )
    {
        UnregisterMesh ( _stippleQueue, _stippleMap, n );
        return;
    }

    UnregisterMesh ( _opaqueQueue, _opaqueMap, n );
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

void Workspace::ComputeTransformGBufferOnly ( GXProjectionClipPlanes const &frustum ) noexcept
{
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
        queueVisible.clear ();

        for ( auto const &instances : queue )
        {
            uint32_t count = 0U;

            for ( auto &mesh : instances.second )
            {
                mesh->_node->Commit ( defaultAlbedo, defaultEmission, defaultMask, defaultParam, defaultNormal );

                if ( !frustum.IsVisible ( mesh->_boundWorld ) )
                    continue;

                ++count;
                transformStream.Push ( &mesh->_transform );
                shadingStream.Push ( &mesh->_shading );
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
}

void Workspace::ComputeTransformGBufferWithID ( GXProjectionClipPlanes const &frustum ) noexcept
{
    _idStream->Commit ();

    auto const traverse = [
        &shadingStream = *_shadingStream,
        &transformStream = *_transformStream,
        &idStream = *_idStream,
        &frustum,
        defaultAlbedo = _defaultAlbedo->_heapIndex,
        defaultEmission = _defaultEmission->_heapIndex,
        defaultMask = _defaultMask->_heapIndex,
        defaultParam = _defaultParam->_heapIndex,
        defaultNormal = _defaultNormal->_heapIndex
    ] ( MeshQueue &queue, std::vector<MeshInstance> &queueVisible ) noexcept {
        queueVisible.clear ();

        for ( auto const &instances : queue )
        {
            uint32_t count = 0U;

            for ( auto &mesh : instances.second )
            {
                mesh->_node->Commit ( defaultAlbedo, defaultEmission, defaultMask, defaultParam, defaultNormal );

                if ( !frustum.IsVisible ( mesh->_boundWorld ) )
                    continue;

                ++count;
                transformStream.Push ( &mesh->_transform );
                shadingStream.Push ( &mesh->_shading );
                idStream.Push ( &mesh->_id );
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
}

void Workspace::FillGBufferOnly ( VkCommandBuffer commandBuffer ) noexcept
{
    AV_TRACE ( "GBuffer" )
    AV_VULKAN_GROUP ( commandBuffer, "GBuffer" )

    pbr::OpaqueProgram &program = *_opaqueProgram;
    program.Bind ( commandBuffer );

    // FUCK - Do something with PushConstants structure. It is reused for opaque and stipple program.

    auto traverse = [
        &shadingStream = *_shadingStream,
        &transformStream = *_transformStream,
        commandBuffer = commandBuffer,
        layout = pbr::UniversalPipelineLayout::GetPipelineLayout (),

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
                pbr::UniversalPipelineLayout::GetStages (),
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

void Workspace::FillGBufferWithID ( VkCommandBuffer commandBuffer ) noexcept
{
    AV_TRACE ( "GBuffer with ID" )
    AV_VULKAN_GROUP ( commandBuffer, "GBuffer with ID" )

    pbr::OpaqueWithIDProgram &program = *_opaqueWithIDProgram;
    program.Bind ( commandBuffer );

    // FUCK - Do something with PushConstants structure. It is reused for opaque and stipple program.

    auto traverse = [
        &shadingStream = *_shadingStream,
        &transformStream = *_transformStream,
        &idStream = *_idStream,
        commandBuffer = commandBuffer,
        layout = pbr::UniversalPipelineLayout::GetPipelineLayout (),

        pushConstants = pbr::OpaqueWithIDProgram::PushConstants
        {
            ._frameStream = _frameStream->AcquireAndConsume ( 1U ),
            ._idImage = _selection.GetIDImageResourceIndex ()
        }
    ] ( std::vector<MeshInstance> const &queueVisible ) mutable noexcept {
        for ( auto const &[ mesh, count ] : queueVisible )
        {
            pushConstants._transformStream = transformStream.AcquireAndConsume ( count );
            pushConstants._shadingStream = shadingStream.AcquireAndConsume ( count );
            pushConstants._idStream = idStream.AcquireAndConsume ( count );

            android_vulkan::MeshBufferInfo const &info = mesh->GetMeshBufferInfo ();
            pushConstants._positionStream = info._bdaStream0;
            pushConstants._restStream = info._bdaStream1;
            pushConstants._indexStream = info._bdaIndex;
            pushConstants._indexType = static_cast<uint32_t> ( info._indexType );

            vkCmdPushConstants ( commandBuffer,
                layout,
                pbr::UniversalPipelineLayout::GetStages (),
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

bool Workspace::IsReady () noexcept
{
    if ( _ready ) [[likely]]
        return true;

    _ready = static_cast<bool> ( _viewport ) &
        static_cast<bool> ( _opaqueProgram ) &
        static_cast<bool> ( _opaqueWithIDProgram ) &
        static_cast<bool> ( _frameStream ) &
        static_cast<bool> ( _transformStream ) &
        static_cast<bool> ( _shadingStream ) &
        static_cast<bool> ( _idStream ) &
        static_cast<bool> ( _defaultAlbedo ) &
        static_cast<bool> ( _defaultEmission ) &
        static_cast<bool> ( _defaultMask ) &
        static_cast<bool> ( _defaultParam ) &
        static_cast<bool> ( _defaultNormal ) &
        ( _opaqueVisible.capacity () > 0U ) &
        ( _stippleVisible.capacity () > 0U ) &
        _selection.IsReady ();

    return _ready;
}

void Workspace::InitGraphicsResources () noexcept
{
    MessageQueue &messageQueue = MessageQueue::Instance ();

    messageQueue.EnqueueBack (
        Message ( eMessageType::InvokeIO,
            [ this, &messageQueue ] () noexcept -> void* {
                AV_TRACE ( "Making Vulkan resources" )

                _opaqueVisible.reserve ( VISIBLE_INITIAL_SIZE );
                _stippleVisible.reserve ( VISIBLE_INITIAL_SIZE );
                android_vulkan::Renderer &renderer = NativeRenderer::Instance ();

                if ( !_selection.Init ( messageQueue, renderer ) ) [[unlikely]]
                    return nullptr;

                VkDevice device = renderer.GetDevice ();
                VkFormat const depth = renderer.GetDefaultDepthFormat ();
                auto opaqueProgram = std::make_unique<pbr::OpaqueProgram> ();

                if ( !opaqueProgram->Init ( device, depth ) ) [[unlikely]]
                    return nullptr;

                auto opaqueProgramReady = [ this ] ( ProgramRef program ) noexcept {
                    // NOLINTNEXTLINE - downcast
                    _opaqueProgram = std::unique_ptr<pbr::OpaqueProgram> (
                        static_cast<pbr::OpaqueProgram*> ( program.release () )
                    );
                };

                messageQueue.EnqueueBack (
                    Message ( eMessageType::NewProgram,
                        [
                            info = ProgramInfo ( std::unique_ptr<pbr::Program> ( opaqueProgram.release () ),
                                std::move ( opaqueProgramReady )
                            )
                        ] () mutable noexcept -> void* {
                            return &info;
                        }
                    )
                );

                auto opaqueWithIDProgram = std::make_unique<pbr::OpaqueWithIDProgram> ();

                if ( !opaqueWithIDProgram->Init ( device, depth ) ) [[unlikely]]
                    return nullptr;

                auto opaqueWithIDProgramReady = [ this ] ( ProgramRef program ) noexcept {
                    // NOLINTNEXTLINE - downcast
                    _opaqueWithIDProgram = std::unique_ptr<pbr::OpaqueWithIDProgram> (
                        static_cast<pbr::OpaqueWithIDProgram*> ( program.release () )
                    );
                };

                messageQueue.EnqueueBack (
                    Message ( eMessageType::NewProgram,
                        [
                            info = ProgramInfo ( std::unique_ptr<pbr::Program> ( opaqueWithIDProgram.release () ),
                                std::move ( opaqueWithIDProgramReady )
                            )
                        ] () mutable noexcept -> void* {
                            return &info;
                        }
                    )
                );

                StreamBufferRef frame = std::make_unique<pbr::StreamBuffer> ();

                if ( !frame->Init ( renderer, pbr::FIF_COUNT, sizeof ( Frame ), "Frame stream" ) ) [[unlikely]]
                    return nullptr;

                auto frameReady = [ this ] ( StreamBufferRef buffer ) noexcept {
                    _frameStream = std::move ( buffer );
                };

                messageQueue.EnqueueBack (
                    Message ( eMessageType::NewStreamBuffer,
                        [
                            info = StreamBufferInfo ( std::move ( frame ), std::move ( frameReady ) )
                        ] () mutable noexcept -> void* {
                            return &info;
                        }
                    )
                );

                StreamBufferRef transform = std::make_unique<pbr::StreamBuffer> ();

                if ( !transform->Init ( renderer, PER_MESH_ELEMENTS, sizeof ( Transform ), "Transform stream" ) )
                {
                    [[unlikely]]
                    return nullptr;
                }

                auto transformReady = [ this ] ( StreamBufferRef buffer ) noexcept {
                    _transformStream = std::move ( buffer );
                };

                messageQueue.EnqueueBack (
                    Message ( eMessageType::NewStreamBuffer,
                        [
                            info = StreamBufferInfo ( std::move ( transform ), std::move ( transformReady ) )
                        ] () mutable noexcept -> void* {
                            return &info;
                        }
                    )
                );

                StreamBufferRef shading = std::make_unique<pbr::StreamBuffer> ();

                if ( !shading->Init ( renderer, PER_MESH_ELEMENTS, sizeof ( Shading ), "Shading stream" ) )
                {
                    [[unlikely]]
                    return nullptr;
                }

                auto shadingReady = [ this ] ( StreamBufferRef buffer ) noexcept {
                    _shadingStream = std::move ( buffer );
                };

                messageQueue.EnqueueBack (
                    Message ( eMessageType::NewStreamBuffer,
                        [
                            info = StreamBufferInfo ( std::move ( shading ), std::move ( shadingReady ) )
                        ] () mutable noexcept -> void* {
                            return &info;
                        }
                    )
                );

                StreamBufferRef id = std::make_unique<pbr::StreamBuffer> ();

                if ( !id->Init ( renderer, PER_MESH_ELEMENTS, sizeof ( uint64_t ), "ID stream" ) )
                {
                    [[unlikely]]
                    return nullptr;
                }

                auto idReady = [ this ] ( StreamBufferRef buffer ) noexcept {
                    _idStream = std::move ( buffer );
                };

                messageQueue.EnqueueBack (
                    Message ( eMessageType::NewStreamBuffer,
                        [
                            info = StreamBufferInfo ( std::move ( id ), std::move ( idReady ) )
                        ] () mutable noexcept -> void* {
                            return &info;
                        }
                    )
                );

                return nullptr;
            }
        )
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
}

void Workspace::InitHotkeys () noexcept
{
    _delete = Hotkey ( eKey::KeyDel,
        false,
        false,
        false,

        [] () noexcept {
            android_vulkan::LogDebug ( ">>> Delete" );
        }
    );

    _openWorkspace = Hotkey ( eKey::KeyO,
        false,
        true,
        false,

        [] () noexcept {
            android_vulkan::LogDebug ( ">>> Open" );
        }
    );

    _saveWorkspace = Hotkey ( eKey::KeyS,
        false,
        true,
        false,

        [] () noexcept {
            android_vulkan::LogDebug ( ">>> Save" );
        }
    );

    _saveAsWorkspace = Hotkey ( eKey::KeyS,
        false,
        true,
        true,

        [] () noexcept {
            android_vulkan::LogDebug ( ">>> Save as" );
        }
    );
}

void Workspace::InitWidgets () noexcept
{
    MessageQueue &messageQueue = MessageQueue::Instance ();

    messageQueue.EnqueueBack (
        Message ( eMessageType::UIAppendWidget,
            [] () noexcept {
                auto* dialogBox = new UIProps ();
                dialogBox->SetRect ( Rect ( 44, 444, 133, 333 ) );

                dialogBox->SetMinSize ( pbr::LengthValue ( pbr::LengthValue::eType::PX, 150.0F ),
                    pbr::LengthValue ( pbr::LengthValue::eType::PX, 90.0F ) );
                return dialogBox;
            }
        )
    );

    _viewport = new ViewportWidget ();

    messageQueue.EnqueueBack (
        Message ( eMessageType::UIAppendWidget,
            [ viewport = _viewport ] () noexcept {
                viewport->Init ();
                return viewport;
            }
        )
    );
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

void Workspace::UnregisterMesh ( MeshQueue &meshQueue, MeshMap &meshMap, MeshInfo &nodeMeshInfo ) noexcept
{
    std::lock_guard const lock ( _mutex );
    auto const findResult = _opaqueQueue.find ( meshMap.extract ( &nodeMeshInfo ).mapped ().lock () );
    Meshes &meshes = findResult->second;

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
