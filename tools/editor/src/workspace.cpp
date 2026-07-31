#include <precompiled_headers.hpp>
#include <native_renderer.hpp>
#include <platform/windows/pbr/universal_pipeline_layout.hpp>
#include <program_info.hpp>
#include <resource_heap.hpp>
#include <sdf_pixel.hpp>
#include <sdf_shape.hpp>
#include <sdf_vertex.hpp>
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

constexpr size_t GIZMO_ELEMENTS = 128UZ;

constexpr float MAX_GIZMO_RAY_DISTANCE = 2.0e+3F;
constexpr float INVERSE_MAX_GIZMO_RAY_DISTANCE = 1.0F / MAX_GIZMO_RAY_DISTANCE;

//----------------------------------------------------------------------------------------------------------------------

class CreateActorAction final : public Action
{
    private:
        std::unordered_map<Actor const*, ActorRef>      *_actors = nullptr;
        ActorRef                                        _actor {};
        Actor*                                          _key = _actor.get ();

    public:
        CreateActorAction () = delete;

        CreateActorAction ( CreateActorAction const & ) = delete;
        CreateActorAction &operator = ( CreateActorAction const & ) = delete;

        CreateActorAction ( CreateActorAction && ) = default;
        CreateActorAction &operator = ( CreateActorAction && ) = default;

        explicit CreateActorAction ( ActorRef &&actor, std::unordered_map<Actor const*, ActorRef> &actors ) noexcept;

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
        AppendComponentAction () = delete;

        AppendComponentAction ( AppendComponentAction const & ) = delete;
        AppendComponentAction &operator = ( AppendComponentAction const & ) = delete;

        AppendComponentAction ( AppendComponentAction && ) = default;
        AppendComponentAction &operator = ( AppendComponentAction && ) = default;

        explicit AppendComponentAction ( Actor &actor, ComponentRef &&compoent ) noexcept;

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

    _gizmoPrepassPushContants._maxRayDistance = MAX_GIZMO_RAY_DISTANCE;
    _gizmoPrepassPushContants._invMaxRayDistance = INVERSE_MAX_GIZMO_RAY_DISTANCE;

    FUCK ();
}

void Workspace::Destroy () noexcept
{
    AV_TRACE ( "Workspace destroy" )

    _viewport->Destroy ();

    MessageQueue &messageQueue = MessageQueue::Instance ();
    android_vulkan::Renderer &renderer = NativeRenderer::Instance ();
    _selection.Destroy ( messageQueue, renderer );
    FreeIDMask ();

    _delete = {};
    _openWorkspace = {};
    _saveWorkspace = {};
    _saveAsWorkspace = {};
    _undo = {};
    _redo = {};

    _history.Clear ();
    _actors.clear ();

    if ( _gizmoPrepassProgram ) [[likely]]
    {
        MessageQueue::Instance ().EnqueueBack (
            Message ( eMessageType::DestroyProgram,
                [ program = ProgramRef ( _gizmoPrepassProgram.release () ) ] () mutable noexcept -> void* {
                    return &program;
                }
            )
        );
    }

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

    if ( _outlineBlurXProgram ) [[likely]]
    {
        MessageQueue::Instance ().EnqueueBack (
            Message ( eMessageType::DestroyProgram,
                [ program = ProgramRef ( _outlineBlurXProgram.release () ) ] () mutable noexcept -> void* {
                    return &program;
                }
            )
        );
    }

    if ( _outlineBorderProgram ) [[likely]]
    {
        MessageQueue::Instance ().EnqueueBack (
            Message ( eMessageType::DestroyProgram,
                [ program = ProgramRef ( _outlineBorderProgram.release () ) ] () mutable noexcept -> void* {
                    return &program;
                }
            )
        );
    }

    if ( _outlineMaskProgram ) [[likely]]
    {
        MessageQueue::Instance ().EnqueueBack (
            Message ( eMessageType::DestroyProgram,
                [ program = ProgramRef ( _outlineMaskProgram.release () ) ] () mutable noexcept -> void* {
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

    if ( _outlineStream ) [[likely]]
    {
        MessageQueue::Instance ().EnqueueBack (
            Message ( eMessageType::DestroyStreamBuffer,
                [ stream = std::move ( _outlineStream ) ] () mutable noexcept -> void* {
                    return &stream;
                }
            )
        );
    }

    if ( _sdfVertexStream ) [[likely]]
    {
        MessageQueue::Instance ().EnqueueBack (
            Message ( eMessageType::DestroyStreamBuffer,
                [ stream = std::move ( _sdfVertexStream ) ] () mutable noexcept -> void* {
                    return &stream;
                }
            )
        );
    }

    if ( _sdfPixelStream ) [[likely]]
    {
        MessageQueue::Instance ().EnqueueBack (
            Message ( eMessageType::DestroyStreamBuffer,
                [ stream = std::move ( _sdfPixelStream ) ] () mutable noexcept -> void* {
                    return &stream;
                }
            )
        );
    }

    if ( _sdfShapeStream ) [[likely]]
    {
        MessageQueue::Instance ().EnqueueBack (
            Message ( eMessageType::DestroyStreamBuffer,
                [ stream = std::move ( _sdfShapeStream ) ] () mutable noexcept -> void* {
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
    _frameInstance = std::nullopt;
    bool const pendingSelect = _selection.IsSelectionRequested ();

    {
        AV_TRACE ( "Compute transforms" )

        _frameStream->Commit ();
        _transformStream->Commit ();
        _shadingStream->Commit ();
        _outlineStream->Commit ();
        _sdfVertexStream->Commit ();
        _sdfPixelStream->Commit ();
        _sdfShapeStream->Commit ();

        // FUCK - correct DPI
        _viewport->Update ( deltaTime, 1.0F );

        GXQuat const& orientation = _viewport->GetOrientation ();

        GXVec3 const &location = _viewport->GetLocation ();
        GXMat4 alpha {};
        alpha.FromFast ( orientation, location );

        GXMat4 beta{};
        beta.Inverse ( alpha );
        GXMat4 const &projection = _viewport->GetProjection ();
        frame._viewProj.Multiply ( beta, projection );

        GXProjectionClipPlanes frustum{};
        frustum.From ( frame._viewProj );

        if ( pendingSelect ) [[unlikely]]
            ComputeTransformGBufferWithID ( frustum );
        else
            ComputeTransformGBufferOnly ( frustum );

        ComputeTransformOutline ( frustum );
        ComputeTransformGizmo ( projection, location, *reinterpret_cast<GXVec3 const*> ( alpha._data[ 2U ] ) );
    }

    bool const noOpaque = _opaqueVisible.empty ();
    bool const noStipple = _stippleVisible.empty ();
    bool const noOutline = _outlineVisible.empty ();
    bool const noOpaqueAndStipple = noOpaque & noStipple;

    if ( noOpaqueAndStipple & noOutline ) [[unlikely]]
        return;

    _frameStream->Push ( &frame );

    {
        AV_TRACE ( "Upload workspace data" )
        AV_VULKAN_GROUP ( commandBuffer, "Upload workspace data" )

        _frameStream->IssueSync ( commandBuffer );

        if ( !noOutline )
            _outlineStream->IssueSync ( commandBuffer );

        if ( !noOpaqueAndStipple ) [[likely]]
        {
            _transformStream->IssueSync ( commandBuffer );
            _shadingStream->IssueSync ( commandBuffer );

            if ( pendingSelect ) [[unlikely]]
            {
                _idStream->IssueSync ( commandBuffer );
            }
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
    if ( !IsReady () | ( _opaqueVisible.empty () & _stippleVisible.empty () ) ) [[unlikely]]
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

void Workspace::DrawOutline ( VkCommandBuffer commandBuffer ) noexcept
{
    if ( !IsReady () | _outlineVisible.empty () )
        return;

    AV_TRACE ( "Outline" )
    AV_VULKAN_GROUP ( commandBuffer, "Outline" )

    _depInfo.imageMemoryBarrierCount = static_cast<uint32_t> ( std::size ( _outlineBarrier0 ) );
    _depInfo.pImageMemoryBarriers = _outlineBarrier0;
    vkCmdPipelineBarrier2 ( commandBuffer, &_depInfo );

    vkCmdBeginRendering ( commandBuffer, &_idRenderingInfo );
    vkCmdSetViewport ( commandBuffer, 0U, 1U, &_idViewport );
    vkCmdSetScissor ( commandBuffer, 0U, 1U, &_idRenderingInfo.renderArea );

    _outlineMaskProgram->Bind ( commandBuffer );
    pbr::StreamBuffer &stream = *_outlineStream;
    VkPipelineLayout layout = pbr::UniversalPipelineLayout::GetPipelineLayout ();

    pbr::OutlineMaskProgram::PushConstants pushConstants = pbr::OutlineMaskProgram::PushConstants
    {
        ._frameStream = AcquireFrameInstance ()
    };

    for ( auto const &[ mesh, count ] : _outlineVisible )
    {
        pushConstants._outlineStream = stream.AcquireAndConsume ( count );

        android_vulkan::MeshBufferInfo const &info = mesh->GetMeshBufferInfo ();
        pushConstants._positionStream = info._bdaStream0;
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

    vkCmdEndRendering ( commandBuffer );

    _depInfo.imageMemoryBarrierCount = static_cast<uint32_t> ( std::size ( _outlineBarrier1 ) );
    _depInfo.pImageMemoryBarriers = _outlineBarrier1;
    vkCmdPipelineBarrier2 ( commandBuffer, &_depInfo );

    pbr::OutlineBorderProgram &border = *_outlineBorderProgram;
    border.Bind ( commandBuffer );
    border.SetPushConstants ( commandBuffer, &_outlineBorderPushConstants );

    vkCmdDispatch ( commandBuffer, _outlineDispatch.width, _outlineDispatch.height, _outlineDispatch.depth );

    _depInfo.imageMemoryBarrierCount = static_cast<uint32_t> ( std::size ( _outlineBarrier2 ) );
    _depInfo.pImageMemoryBarriers = _outlineBarrier2;
    vkCmdPipelineBarrier2 ( commandBuffer, &_depInfo );

    pbr::OutlineBlurXProgram &blur = *_outlineBlurXProgram;
    blur.Bind ( commandBuffer );
    blur.SetPushConstants ( commandBuffer, &_outlineBlurXPushConstants );

    vkCmdDispatch ( commandBuffer, _outlineDispatch.width, _outlineDispatch.height, _outlineDispatch.depth );

    _depInfo.imageMemoryBarrierCount = 1U;
    _depInfo.pImageMemoryBarriers = &_blurXBarrier;
    vkCmdPipelineBarrier2 ( commandBuffer, &_depInfo );
}

std::optional<uint32_t> Workspace::GetOutlineBlurX () noexcept
{
    if ( !IsReady () | _outlineVisible.empty () )
        return std::nullopt;

    return std::optional<uint32_t> ( _blurX->_sampledIndex );
}

void Workspace::OnGBufferResolutionChanged ( android_vulkan::Texture2D &idImage, uint32_t idResourceIdx ) noexcept
{
    _selection.OnGBufferResolutionChanged ( idImage, idResourceIdx );

    FreeIDMask ();

    VkExtent2D &resolution = _idRenderingInfo.renderArea.extent;
    android_vulkan::Renderer &renderer = NativeRenderer::Instance ();
    resolution = renderer.GetViewportResolution ();
    _outlineBorderPushConstants._resolution = resolution;
    _outlineBlurXPushConstants._resolution = resolution;
    _outlineDispatch = pbr::OutlineBorderProgram::DispatchParams ( resolution );

    _idMask = std::make_shared<Texture2D> ();
    android_vulkan::Texture2D &idMask = _idMask->_resource;

    _idDepth = std::make_shared<Texture2D> ();
    android_vulkan::Texture2D &idDepth = _idDepth->_resource;

    _border = std::make_shared<Texture2D> ();
    android_vulkan::Texture2D &border = _border->_resource;

    _blurX = std::make_shared<Texture2D> ();
    android_vulkan::Texture2D &blurX = _blurX->_resource;

    bool const status =
        idMask.CreateRenderTarget ( resolution,
            VK_FORMAT_R8_UNORM,
            AV_VK_FLAG ( VK_IMAGE_USAGE_SAMPLED_BIT ) | AV_VK_FLAG ( VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT ),
            renderer
        ) &&

        idDepth.CreateRenderTarget ( resolution,
            renderer.GetDefaultDepthFormat (),
            VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
            renderer
        ) &&

        border.CreateRenderTarget ( resolution,
            VK_FORMAT_R8_UNORM,
            AV_VK_FLAG ( VK_IMAGE_USAGE_SAMPLED_BIT ) | AV_VK_FLAG ( VK_IMAGE_USAGE_STORAGE_BIT ),
            renderer
        ) &&

        blurX.CreateRenderTarget ( resolution,
            VK_FORMAT_R8_UNORM,
            AV_VK_FLAG ( VK_IMAGE_USAGE_SAMPLED_BIT ) | AV_VK_FLAG ( VK_IMAGE_USAGE_STORAGE_BIT ),
            renderer
        );

    if ( !status ) [[unlikely]]
    {
        AV_ASSERT ( false )
        return;
    }

    MessageQueue &messageQueue = MessageQueue::Instance ();

    messageQueue.EnqueueBack ( Message ( eMessageType::NewTexture2D,
            [] () noexcept -> void* {
                return std::bit_cast<void*> ( 4UZ );
            }
        )
    );

    _idViewport =
    {
        .x = 0.0F,
        .y = 0.0F,
        .width = static_cast<float> ( resolution.width ),
        .height = static_cast<float> ( resolution.height ),
        .minDepth = 0.0F,
        .maxDepth = 1.0F
    };

    GXVec2 alpha ( 1.0F / _idViewport.width, 1.0F / _idViewport.height );
    _outlineBorderPushConstants._invResolution = alpha;
    _outlineBlurXPushConstants._invResolution = alpha;

    alpha.Multiply ( alpha, 0.5F );
    GXVec2 neg ( alpha );
    neg.Reverse ();

    _outlineBorderPushConstants._halfPixelMove = GXVec4 ( neg._data[ 0U ],
        neg._data[ 1U ],
        alpha._data[ 0U ],
        alpha._data[ 1U ]
    );

    _outlineBlurXPushConstants._halfPixelMove = neg;

    VkDevice device = renderer.GetDevice ();
    VkImage mask = idMask.GetImage ();
    VkImageView maskView = idMask.GetImageView ();
    AV_SET_VULKAN_OBJECT_NAME ( device, mask, VK_OBJECT_TYPE_IMAGE, "ID mask" )
    AV_SET_VULKAN_OBJECT_NAME ( device, maskView, VK_OBJECT_TYPE_IMAGE_VIEW, "ID mask" )
    _idMaskAttachment.imageView = maskView;
    _outlineBarrier0[ 0U ].image = mask;
    _outlineBarrier1[ 0U ].image = mask;

    pbr::ResourceHeap &resourceHeap = ResourceHeap::Instance ();
    auto idx = resourceHeap.RegisterNonUISampledImage ( device, maskView );

    if ( !idx )
    {
        AV_ASSERT ( false )
        return;
    }

    _outlineBorderPushConstants._idMask = *idx;
    _idMask->_sampledIndex = std::move ( idx );

    VkImage borderImage = border.GetImage ();
    VkImageView borderView = border.GetImageView ();
    AV_SET_VULKAN_OBJECT_NAME ( device, borderImage, VK_OBJECT_TYPE_IMAGE, "Border" )
    AV_SET_VULKAN_OBJECT_NAME ( device, borderView, VK_OBJECT_TYPE_IMAGE_VIEW, "Border" )
    _outlineBarrier1[ 1U ].image = borderImage;
    _outlineBarrier2[ 0U ].image = borderImage;
    idx = resourceHeap.RegisterStorageImage ( device, borderView );

    if ( !idx )
    {
        AV_ASSERT ( false )
        return;
    }

    _outlineBorderPushConstants._outline = *idx;
    _border->_storageIndex = std::move ( idx );

    idx = resourceHeap.RegisterNonUISampledImage ( device, borderView );

    if ( !idx )
    {
        AV_ASSERT ( false )
        return;
    }

    _outlineBlurXPushConstants._border = *idx;
    _border->_sampledIndex = std::move ( idx );

    VkImage blurXImage = blurX.GetImage ();
    VkImageView blurXView = blurX.GetImageView ();
    AV_SET_VULKAN_OBJECT_NAME ( device, blurXImage, VK_OBJECT_TYPE_IMAGE, "Blur X" )
    AV_SET_VULKAN_OBJECT_NAME ( device, blurXView, VK_OBJECT_TYPE_IMAGE_VIEW, "Blur X" )
    _outlineBarrier2[ 1U ].image = blurXImage;
    _blurXBarrier.image = blurXImage;
    idx = resourceHeap.RegisterStorageImage ( device, blurXView );

    if ( !idx )
    {
        AV_ASSERT ( false )
        return;
    }

    _outlineBlurXPushConstants._blurX = *idx;
    _blurX->_storageIndex = std::move ( idx );

    idx = resourceHeap.RegisterNonUISampledImage ( device, blurXView );

    if ( !idx )
    {
        AV_ASSERT ( false )
        return;
    }

    _blurX->_sampledIndex = std::move ( idx );

    VkImage depth = idDepth.GetImage ();
    VkImageView depthView = idDepth.GetImageView ();
    AV_SET_VULKAN_OBJECT_NAME ( device, depth, VK_OBJECT_TYPE_IMAGE, "ID depth" )
    AV_SET_VULKAN_OBJECT_NAME ( device, depthView, VK_OBJECT_TYPE_IMAGE_VIEW, "ID depth" )
    _idDepthAttachment.imageView = depthView;
    _outlineBarrier0[ 1U ].image = depth;
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

GBufferMeshNode Workspace::RegisterOpaqueMesh ( MeshGeometryRef &mesh ) noexcept
{
    AV_TRACE ( "Workspace register opaque mesh" )

    // This will also act as search key for unregister operation.
    auto &m = *new GBufferMeshInfo;

    m._material = eMaterial::Opaque;
    return RegisterMesh ( mesh, _opaqueQueue, _opaqueMap, m );
}

GBufferMeshNode Workspace::RegisterStippleMesh ( MeshGeometryRef &mesh ) noexcept
{
    AV_TRACE ( "Workspace register stipple mesh" )

    // This will also act as search key for unregister operation.
    auto &m = *new GBufferMeshInfo;

    m._material = eMaterial::Stipple;
    return RegisterMesh ( mesh, _stippleQueue, _stippleMap, m );
}

OutlineMeshNode Workspace::RegisterOutline ( MeshGeometryRef &mesh ) noexcept
{
    AV_TRACE ( "Workspace register stipple mesh" )

    // This will also act as search key for unregister operation.
    auto* node = new OutlineMeshInfo;

    auto w = MeshGeometryRef::weak_type ( mesh );

    {
        std::lock_guard const lock ( _mutex );
        _outlineQueue[ mesh ].push_back ( node );
        _outlineMap[ node ] = std::move ( w );
    }

    return OutlineMeshNode ( *this, *node );
}

GizmoNode Workspace::RegisterGizmo ( eSDFShape shape, GizmoNode::UpdateHandler &&update ) noexcept
{
    AV_TRACE ( "Workspace register gizmo" )

    // This will also act as search key for unregister operation.
    auto* info = new GizmoInfo;
    info->_shape._type = static_cast<uint32_t> ( shape );

    {
        std::lock_guard const lock ( _mutex );
        _gizmoQueue.push_back ( info );
    }

    return GizmoNode ( *this, *info, std::move ( update ) );
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

void Workspace::UnregisterOpaque ( GBufferMeshNode &node ) noexcept
{
    AV_TRACE ( "Workspace unregister opaque mesh" )
    UnregisterMesh ( _opaqueQueue, _opaqueMap, node );
}

void Workspace::UnregisterStipple ( GBufferMeshNode &node ) noexcept
{
    AV_TRACE ( "Workspace unregister stipple mesh" )
    UnregisterMesh ( _stippleQueue, _stippleMap, node );
}

void Workspace::UnregisterOutline ( OutlineMeshNode &node ) noexcept
{
    AV_TRACE ( "Workspace unregister outline" )
    node._workspace = nullptr;
    OutlineMeshInfo* meshInfo = std::exchange ( node._meshInfo, nullptr );

    std::lock_guard const lock ( _mutex );
    auto const findResult = _outlineQueue.find ( _outlineMap.extract ( meshInfo ).mapped ().lock () );
    OutlineMeshes &meshes = findResult->second;

    if ( meshes.size () == 1U )
    {
        AV_ASSERT ( meshes.front () == meshInfo )
        _outlineQueue.erase ( findResult );
        delete meshInfo;
        return;
    }

    for ( auto &mesh : meshes )
    {
        if ( mesh != meshInfo )
            continue;

        mesh = meshes.back ();
        meshes.pop_back ();
        delete meshInfo;
        return;
    }
}

void Workspace::Unregister ( GizmoNode &node ) noexcept
{
    AV_TRACE ( "Workspace unregister gizmo" )
    node._workspace = nullptr;
    GizmoInfo* gizmoInfo = std::exchange ( node._gizmoInfo, nullptr );
    std::lock_guard const lock ( _mutex );
    _gizmoQueue.erase ( std::find ( _gizmoQueue.cbegin (), _gizmoQueue.cbegin (), gizmoInfo ) );
    delete gizmoInfo;
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

VkDeviceAddress Workspace::AcquireFrameInstance () noexcept
{
    if ( !_frameInstance ) [[unlikely]]
        _frameInstance = std::optional<VkDeviceAddress> ( _frameStream->AcquireAndConsume ( 1U ) );

    return *_frameInstance;
}

void Workspace::FreeIDMask () noexcept
{
    MessageQueue &messageQueue = MessageQueue::Instance ();

    if ( _idDepth ) [[likely]]
    {
        messageQueue.EnqueueBack (
            Message ( eMessageType::DestroyTexture2D,

                [ texture = std::move ( _idDepth ) ] () mutable noexcept -> void* {
                    return &texture;
                }
            )
        );
    }

    if ( _idMask ) [[likely]]
    {
        messageQueue.EnqueueBack (
            Message ( eMessageType::DestroyTexture2D,

                [ texture = std::move ( _idMask ) ] () mutable noexcept -> void* {
                    return &texture;
                }
            )
        );
    }

    if ( _border ) [[likely]]
    {
        messageQueue.EnqueueBack (
            Message ( eMessageType::DestroyTexture2D,

                [ texture = std::move ( _border ) ] () mutable noexcept -> void* {
                    return &texture;
                }
            )
        );
    }

    if ( !_blurX ) [[unlikely]]
        return;

    messageQueue.EnqueueBack (
        Message ( eMessageType::DestroyTexture2D,

            [ texture = std::move ( _blurX ) ] () mutable noexcept -> void* {
                return &texture;
            }
        )
    );
}

void Workspace::FUCK () noexcept
{
    _history.Begin ();

    ActorRef actor = std::make_unique<Actor> ();
    actor->SetName ( "Full" );
    Actor &a0 = *actor;

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
    a0.SetLocation ( GXVec3 ( -1.2F, -1.0F, 3.0F ) );

    _history.Append ( std::make_unique<CreateActorAction> ( std::move ( actor ), _actors ) );
    _history.Append ( std::make_unique<AppendComponentAction> ( a0, std::move ( mesh1 ) ) );
    _history.Append ( std::make_unique<AppendComponentAction> ( a0, std::move ( mesh2 ) ) );
    _history.Append ( std::make_unique<AppendComponentAction> ( a0, std::move ( mesh3 ) ) );

    actor = std::make_unique<Actor> ();
    actor->SetName ( "Part #1" );
    Actor &a1 = *actor;

    mesh1 = std::make_unique<StaticMeshComponent> ( "meshes/rotating_mesh/sonic-material-1.mesh2",
        "../editor-assets/textures/sonic-material-1-diffuse.png"
    );

    mesh1->SetName ( "mesh #1" );

    constexpr GXVec3 parts ( 1.2F, -1.0F, 3.0F );
    a1.SetLocation ( parts );

    _history.Append ( std::make_unique<CreateActorAction> ( std::move ( actor ), _actors ) );
    _history.Append ( std::make_unique<AppendComponentAction> ( a1, std::move ( mesh1 ) ) );

    actor = std::make_unique<Actor> ();
    actor->SetName ( "Part #2" );
    Actor &a2 = *actor;

    mesh2 = std::make_unique<StaticMeshComponent> ( "meshes/rotating_mesh/sonic-material-2.mesh2",
        "../editor-assets/textures/sonic-material-2-diffuse.png"
    );

    mesh2->SetName ( "mesh #2" );
    a2.SetLocation ( parts );

    _history.Append ( std::make_unique<CreateActorAction> ( std::move ( actor ), _actors ) );
    _history.Append ( std::make_unique<AppendComponentAction> ( a2, std::move ( mesh2 ) ) );

    actor = std::make_unique<Actor> ();
    actor->SetName ( "Part #3" );
    Actor &a3 = *actor;

    mesh3 = std::make_unique<StaticMeshComponent> ( "meshes/rotating_mesh/sonic-material-3.mesh2",
        "../editor-assets/textures/sonic-material-3-diffuse.png"
    );

    mesh3->SetName ( "mesh #3" );
    a3.SetLocation ( parts );

    _history.Append ( std::make_unique<CreateActorAction> ( std::move ( actor ), _actors ) );
    _history.Append ( std::make_unique<AppendComponentAction> ( a3, std::move ( mesh3 ) ) );

    _history.End ();
}

void Workspace::ComputeTransformGBufferOnly ( GXProjectionClipPlanes const &frustum ) noexcept
{
    AV_TRACE ( "G-buffer only" )

    auto const traverse = [
        &shadingStream = *_shadingStream,
        &transformStream = *_transformStream,
        &frustum,
        defaultAlbedo = *_defaultAlbedo->_sampledIndex,
        defaultEmission = *_defaultEmission->_sampledIndex,
        defaultMask = *_defaultMask->_sampledIndex,
        defaultParam = *_defaultParam->_sampledIndex,
        defaultNormal = *_defaultNormal->_sampledIndex
    ] ( GBufferMeshQueue &queue, std::vector<MeshInstance> &queueVisible ) noexcept {
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
    AV_TRACE ( "G-buffer with ID" )
    _idStream->Commit ();

    auto const traverse = [
        &shadingStream = *_shadingStream,
        &transformStream = *_transformStream,
        &idStream = *_idStream,
        &frustum,
        defaultAlbedo = *_defaultAlbedo->_sampledIndex,
        defaultEmission = *_defaultEmission->_sampledIndex,
        defaultMask = *_defaultMask->_sampledIndex,
        defaultParam = *_defaultParam->_sampledIndex,
        defaultNormal = *_defaultNormal->_sampledIndex
    ] ( GBufferMeshQueue &queue, std::vector<MeshInstance> &queueVisible ) noexcept {
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

void Workspace::ComputeTransformOutline ( GXProjectionClipPlanes const &frustum ) noexcept
{
    AV_TRACE ( "Outline" )
    pbr::StreamBuffer &stream = *_outlineStream;
    _outlineVisible.clear ();

    for ( auto const &instances : _outlineQueue )
    {
        uint32_t count = 0U;

        for ( auto &mesh : instances.second )
        {
            mesh->_node->Commit ();

            if ( !frustum.IsVisible ( mesh->_boundWorld ) )
                continue;

            ++count;
            stream.Push ( &mesh->_model );
        }

        if ( !count ) [[unlikely]]
            continue;

        _outlineVisible.push_back (
            {
                ._mesh = instances.first.get (),
                ._count = count
            }
        );
    }
}

void Workspace::ComputeTransformGizmo ( GXMat4 const &projection,
    GXVec3 const &cameraLocation,
    GXVec3 const &cameraForward
) noexcept
{
    if ( _gizmoQueue.empty () )
        return;

    AV_TRACE ( "Gizmo" )
    pbr::StreamBuffer &vertexStream = *_sdfVertexStream;
    pbr::StreamBuffer &pixelStream = *_sdfPixelStream;
    pbr::StreamBuffer &shapeStream = *_sdfShapeStream;
    _gizmoVisible = 0U;

    _gizmoPrepassPushContants._toCVV = projection;
    _gizmoPrepassPushContants._cameraLocationWorld = cameraLocation;

    // See <repo>/docs/gizmo-rendering.md#pixel-coverage
    float const t = std::tan ( 0.5F * ViewportWidget::GetFieldOfView () );
    GXVec3 &viWorld = _gizmoPrepassPushContants._viWorld;
    viWorld.Multiply ( cameraForward, ( t + t ) * _outlineBlurXPushConstants._invResolution._data[ 1U ] );

    for ( GizmoInfo* gizmo : _gizmoQueue )
    {
        // FUCK - frustum culling
        gizmo->_node->Commit ( cameraLocation, cameraForward, viWorld );
        vertexStream.Push ( &gizmo->_vertex );
        pixelStream.Push ( &gizmo->_pixel );
        shapeStream.Push ( &gizmo->_shape );
        ++_gizmoVisible;
    }
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
            ._frameStream = AcquireFrameInstance ()
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
            ._frameStream = AcquireFrameInstance (),
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
        static_cast<bool> ( _gizmoPrepassProgram ) &
        static_cast<bool> ( _opaqueProgram ) &
        static_cast<bool> ( _opaqueWithIDProgram ) &
        static_cast<bool> ( _outlineBlurXProgram ) &
        static_cast<bool> ( _outlineBorderProgram ) &
        static_cast<bool> ( _outlineMaskProgram ) &
        static_cast<bool> ( _frameStream ) &
        static_cast<bool> ( _transformStream ) &
        static_cast<bool> ( _shadingStream ) &
        static_cast<bool> ( _idStream ) &
        static_cast<bool> ( _outlineStream ) &
        static_cast<bool> ( _sdfVertexStream ) &
        static_cast<bool> ( _sdfPixelStream ) &
        static_cast<bool> ( _sdfShapeStream ) &
        static_cast<bool> ( _defaultAlbedo ) &
        static_cast<bool> ( _defaultEmission ) &
        static_cast<bool> ( _defaultMask ) &
        static_cast<bool> ( _defaultParam ) &
        static_cast<bool> ( _defaultNormal ) &
        ( static_cast<bool> ( _idDepth ) && _idDepth->_resource.IsInit () ) &
        ( static_cast<bool> ( _idMask ) && _idMask->_resource.IsInit () ) &
        ( static_cast<bool> ( _border ) && _border->_resource.IsInit () ) &
        ( static_cast<bool> ( _blurX ) && _blurX->_resource.IsInit () ) &
        ( _opaqueVisible.capacity () > 0U ) &
        ( _stippleVisible.capacity () > 0U ) &
        ( _outlineVisible.capacity () > 0U ) &
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
                _outlineVisible.reserve ( VISIBLE_INITIAL_SIZE );
                android_vulkan::Renderer &renderer = NativeRenderer::Instance ();

                if ( !_selection.Init ( messageQueue, renderer ) ) [[unlikely]]
                    return nullptr;

                VkDevice device = renderer.GetDevice ();
                VkFormat const depth = renderer.GetDefaultDepthFormat ();
                auto gizmoPrepassProgram = std::make_unique<pbr::GizmoPrepassProgram> ();

                if ( !gizmoPrepassProgram->Init ( device, renderer.GetSurfaceFormat (), depth ) ) [[unlikely]]
                    return nullptr;

                auto gizmoPrepassProgramReady = [ this ] ( ProgramRef program ) noexcept {
                    // NOLINTNEXTLINE - downcast
                    _gizmoPrepassProgram = std::unique_ptr<pbr::GizmoPrepassProgram> (
                        static_cast<pbr::GizmoPrepassProgram*> ( program.release () )
                    );
                };

                messageQueue.EnqueueBack (
                    Message ( eMessageType::NewProgram,
                        [
                            info = ProgramInfo ( std::unique_ptr<pbr::Program> ( gizmoPrepassProgram.release () ),
                                std::move ( gizmoPrepassProgramReady )
                            )
                        ] () mutable noexcept -> void* {
                            return &info;
                        }
                    )
                );

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

                auto outlineBlurProgram = std::make_unique<pbr::OutlineBlurXProgram> ();

                if ( !outlineBlurProgram->Init ( device, nullptr ) ) [[unlikely]]
                    return nullptr;

                auto outlineBlurXProgramReady = [ this ] ( ProgramRef program ) noexcept {
                    // NOLINTNEXTLINE - downcast
                    _outlineBlurXProgram = std::unique_ptr<pbr::OutlineBlurXProgram> (
                        static_cast<pbr::OutlineBlurXProgram*> ( program.release () )
                    );
                };

                messageQueue.EnqueueBack (
                    Message ( eMessageType::NewProgram,
                        [
                            info = ProgramInfo ( std::unique_ptr<pbr::Program> ( outlineBlurProgram.release () ),
                                std::move ( outlineBlurXProgramReady )
                            )
                        ] () mutable noexcept -> void* {
                            return &info;
                        }
                    )
                );

                auto outlineBorderProgram = std::make_unique<pbr::OutlineBorderProgram> ();

                if ( !outlineBorderProgram->Init ( device, nullptr ) ) [[unlikely]]
                    return nullptr;

                auto outlineBorderProgramReady = [ this ] ( ProgramRef program ) noexcept {
                    // NOLINTNEXTLINE - downcast
                    _outlineBorderProgram = std::unique_ptr<pbr::OutlineBorderProgram> (
                        static_cast<pbr::OutlineBorderProgram*> ( program.release () )
                    );
                };

                messageQueue.EnqueueBack (
                    Message ( eMessageType::NewProgram,
                        [
                            info = ProgramInfo ( std::unique_ptr<pbr::Program> ( outlineBorderProgram.release () ),
                                std::move ( outlineBorderProgramReady )
                            )
                        ] () mutable noexcept -> void* {
                            return &info;
                        }
                    )
                );

                auto outlineMaskProgram = std::make_unique<pbr::OutlineMaskProgram> ();

                if ( !outlineMaskProgram->Init ( device, depth ) ) [[unlikely]]
                    return nullptr;

                auto outlineMaskProgramReady = [ this ] ( ProgramRef program ) noexcept {
                    // NOLINTNEXTLINE - downcast
                    _outlineMaskProgram = std::unique_ptr<pbr::OutlineMaskProgram> (
                        static_cast<pbr::OutlineMaskProgram*> ( program.release () )
                    );
                };

                messageQueue.EnqueueBack (
                    Message ( eMessageType::NewProgram,
                        [
                            info = ProgramInfo ( std::unique_ptr<pbr::Program> ( outlineMaskProgram.release () ),
                                std::move ( outlineMaskProgramReady )
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

                StreamBufferRef outline = std::make_unique<pbr::StreamBuffer> ();

                if ( !outline->Init ( renderer, PER_MESH_ELEMENTS, sizeof ( Model ), "Outline stream" ) )
                {
                    [[unlikely]]
                    return nullptr;
                }

                auto outlineReady = [ this ] ( StreamBufferRef buffer ) noexcept {
                    _outlineStream = std::move ( buffer );
                };

                messageQueue.EnqueueBack (
                    Message ( eMessageType::NewStreamBuffer,
                        [
                            info = StreamBufferInfo ( std::move ( outline ), std::move ( outlineReady ) )
                        ] () mutable noexcept -> void* {
                            return &info;
                        }
                    )
                );

                StreamBufferRef sdfVertex = std::make_unique<pbr::StreamBuffer> ();

                if ( !sdfVertex->Init ( renderer, GIZMO_ELEMENTS, sizeof ( SDFVertex ), "SDF vertex stream" ) )
                {
                    [[unlikely]]
                    return nullptr;
                }

                auto sdfVertexReady = [ this ] ( StreamBufferRef buffer ) noexcept {
                    _sdfVertexStream = std::move ( buffer );
                };

                messageQueue.EnqueueBack (
                    Message ( eMessageType::NewStreamBuffer,
                        [
                            info = StreamBufferInfo ( std::move ( sdfVertex ), std::move ( sdfVertexReady ) )
                        ] () mutable noexcept -> void* {
                            return &info;
                        }
                    )
                );

                StreamBufferRef sdfPixel = std::make_unique<pbr::StreamBuffer> ();

                if ( !sdfPixel->Init ( renderer, GIZMO_ELEMENTS, sizeof ( SDFPixel ), "SDF pixel stream" ) )
                {
                    [[unlikely]]
                    return nullptr;
                }

                auto sdfPixelReady = [ this ] ( StreamBufferRef buffer ) noexcept {
                    _sdfPixelStream = std::move ( buffer );
                };

                messageQueue.EnqueueBack (
                    Message ( eMessageType::NewStreamBuffer,
                        [
                            info = StreamBufferInfo ( std::move ( sdfPixel ), std::move ( sdfPixelReady ) )
                        ] () mutable noexcept -> void* {
                            return &info;
                        }
                    )
                );

                StreamBufferRef sdfShape = std::make_unique<pbr::StreamBuffer> ();

                if ( !sdfShape->Init ( renderer, GIZMO_ELEMENTS, sizeof ( SDFShape ), "SDF shape stream" ) )
                {
                    [[unlikely]]
                    return nullptr;
                }

                auto sdfShapeReady = [ this ] ( StreamBufferRef buffer ) noexcept {
                    _sdfShapeStream = std::move ( buffer );
                };

                messageQueue.EnqueueBack (
                    Message ( eMessageType::NewStreamBuffer,
                        [
                            info = StreamBufferInfo ( std::move ( sdfShape ), std::move ( sdfShapeReady ) )
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

    _undo = Hotkey ( eKey::KeyZ,
        false,
        true,
        false,

        [ &history = _history ] () noexcept {
            history.Undo ();
        }
    );

    _redo = Hotkey ( eKey::KeyZ,
        false,
        true,
        true,

        [ &history = _history ] () noexcept {
            history.Redo ();
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

GBufferMeshNode Workspace::RegisterMesh ( MeshGeometryRef &mesh,
    GBufferMeshQueue &meshQueue,
    GBufferMeshMap &meshMap,
    GBufferMeshInfo &node
) noexcept
{
    auto w = MeshGeometryRef::weak_type ( mesh );

    {
        std::lock_guard const lock ( _mutex );
        meshQueue[ mesh ].push_back ( &node );
        meshMap[ &node ] = std::move ( w );
    }

    return GBufferMeshNode ( *this, node );
}

void Workspace::UnregisterMesh ( GBufferMeshQueue &meshQueue,
    GBufferMeshMap &meshMap,
    GBufferMeshNode &node
) noexcept
{
    std::lock_guard const lock ( _mutex );
    node._workspace = nullptr;
    GBufferMeshInfo* meshInfo = std::exchange ( node._meshInfo, nullptr );
    auto const findResult = meshQueue.find ( meshMap.extract ( meshInfo ).mapped ().lock () );
    GBufferMeshes &meshes = findResult->second;

    if ( meshes.size () == 1U )
    {
        AV_ASSERT ( meshes.front () == meshInfo )
        meshQueue.erase ( findResult );
        delete meshInfo;
        return;
    }

    for ( auto &mesh : meshes )
    {
        if ( mesh != meshInfo )
            continue;

        mesh = meshes.back ();
        meshes.pop_back ();
        delete meshInfo;
        return;
    }
}

} // namespace editor
