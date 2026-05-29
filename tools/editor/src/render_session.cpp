#include <precompiled_headers.hpp>
#include <av_assert.hpp>
#include <hello_triangle_vertex.hpp>
#include <logger.hpp>
#include <message_queue.hpp>
#include <native_renderer.hpp>
#include <render_session.hpp>
#include <resource_heap.hpp>
#include <trace.hpp>
#include <ui_manager.hpp>
#include <vulkan_utils.hpp>


namespace editor {

namespace {

constexpr float DEFAULT_BRIGHTNESS_BALANCE = 0.0F;
constexpr VkFormat RENDER_TARGET_FORMAT = VK_FORMAT_R16G16B16A16_SFLOAT;

//----------------------------------------------------------------------------------------------------------------------

class HelloTriangleJob final
{
    public:
        std::unique_ptr<android_vulkan::MeshGeometry>       _geometry {};
        std::unique_ptr<HelloTriangleProgram>               _program {};

    private:
        VkCommandPool                                       _commandPool = VK_NULL_HANDLE;
        VkFence                                             _complete = VK_NULL_HANDLE;
        android_vulkan::Renderer                            &_renderer;

    public:
        HelloTriangleJob () = delete;

        HelloTriangleJob ( HelloTriangleJob const & ) = delete;
        HelloTriangleJob &operator = ( HelloTriangleJob const & ) = delete;

        HelloTriangleJob ( HelloTriangleJob && ) = delete;
        HelloTriangleJob &operator = ( HelloTriangleJob && ) = delete;

        explicit HelloTriangleJob ( android_vulkan::Renderer &renderer, std::mutex &submitMutex ) noexcept;

        ~HelloTriangleJob ();

    private:
        void CreateMesh ( std::mutex &submitMutex ) noexcept;
        void CreateProgram () noexcept;
};

HelloTriangleJob::HelloTriangleJob ( android_vulkan::Renderer &renderer, std::mutex &submitMutex ) noexcept:
    _renderer ( renderer )
{
    std::thread (
        [ this, &submitMutex ] () noexcept
        {
            AV_THREAD_NAME ( "Hello triangle job" )
            CreateProgram ();
            CreateMesh ( submitMutex );

            MessageQueue::Instance ().EnqueueBack (
                {
                    ._type = eMessageType::HelloTriangleReady,

                    ._action = [ value = this ] () noexcept {
                        return value;
                    },

                    ._serialNumber = 0U
                }
            );
        }
    ).detach ();
}

HelloTriangleJob::~HelloTriangleJob ()
{
    VkDevice device = _renderer.GetDevice ();

    if ( _commandPool != VK_NULL_HANDLE ) [[likely]]
        vkDestroyCommandPool ( device, _commandPool, nullptr );

    if ( _complete != VK_NULL_HANDLE ) [[likely]]
    {
        vkDestroyFence ( device, _complete, nullptr );
    }
}

void HelloTriangleJob::CreateMesh ( std::mutex &submitMutex ) noexcept
{
    AV_TRACE ( "Mesh" )

    VkCommandPoolCreateInfo const poolInfo
    {
        .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
        .pNext = nullptr,
        .flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT,
        .queueFamilyIndex = _renderer.GetQueueFamilyIndex ()
    };

    VkDevice device = _renderer.GetDevice ();

    bool result = android_vulkan::Renderer::CheckVkResult (
        vkCreateCommandPool ( device, &poolInfo, nullptr, &_commandPool ),
        "editor::HelloTriangleJob::CreateMesh",
        "Can't create lead command pool"
    );

    if ( !result ) [[unlikely]]
        return;

    AV_SET_VULKAN_OBJECT_NAME ( device, _commandPool, VK_OBJECT_TYPE_COMMAND_POOL, "Hello triangle" )

    VkCommandBufferAllocateInfo bufferAllocateInfo
    {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
        .pNext = nullptr,
        .commandPool = _commandPool,
        .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
        .commandBufferCount = 1U
    };

    VkCommandBuffer commandBuffer;

    result = android_vulkan::Renderer::CheckVkResult (
        vkAllocateCommandBuffers ( device, &bufferAllocateInfo, &commandBuffer ),
        "editor::HelloTriangleJob::CreateMesh",
        "Can't allocate command buffer"
    );

    if ( !result ) [[unlikely]]
        return;

    AV_SET_VULKAN_OBJECT_NAME ( device, commandBuffer, VK_OBJECT_TYPE_COMMAND_BUFFER, "Hello triangle" )

    constexpr VkCommandBufferBeginInfo beginInfo
    {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
        .pNext = nullptr,
        .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
        .pInheritanceInfo = nullptr
    };

    result = android_vulkan::Renderer::CheckVkResult ( vkBeginCommandBuffer ( commandBuffer, &beginInfo ),
        "editor::HelloTriangleJob::CreateMesh",
        "Can't begin command buffer"
    );

    if ( !result ) [[unlikely]]
        return;

    {
        AV_VULKAN_GROUP ( commandBuffer, "Hello triangle" )

        constexpr HelloTriangleVertex const data[] =
        {
            {
                ._vertex = GXVec2 ( -0.75F, 0.75F ),
                ._color = GXVec3 ( 0.0F, 0.0F, 1.0F )
            },

            {
                ._vertex = GXVec2 ( 0.0F, -0.75F ),
                ._color = GXVec3 ( 1.0F, 0.0F, 0.0F )
            },

            {
                ._vertex = GXVec2 ( 0.75F, 0.75F ),
                ._color = GXVec3 ( 0.0F, 1.0F, 0.0F )
            }
        };

        _geometry = std::make_unique<android_vulkan::MeshGeometry> ();

        android_vulkan::MeshGeometry::LoadResult loadResult = _geometry->LoadMesh ( _renderer,
            { reinterpret_cast<uint8_t const*> ( data ), sizeof ( data ) },
            static_cast<uint32_t> ( std::size ( data ) )
        );

        if ( !loadResult ) [[unlikely]]
        {
            _geometry->FreeResources ( _renderer );
            _geometry.reset ();
            return;
        }

        if ( !_geometry->UploadToGPU ( _renderer, commandBuffer, VK_NULL_HANDLE, std::move ( *loadResult ) ) )
        {
            [[unlikely]]
            _geometry->FreeResources ( _renderer );
            _geometry.reset ();
        }
    }

    result = android_vulkan::Renderer::CheckVkResult ( vkEndCommandBuffer ( commandBuffer ),
        "editor::HelloTriangleJob::CreateMesh",
        "Can't end command buffer"
    );

    if ( !result ) [[unlikely]]
    {
        _geometry->FreeResources ( _renderer );
        _geometry.reset ();
        return;
    }

    constexpr VkFenceCreateInfo fenceInfo
    {
        .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0U
    };

    result = android_vulkan::Renderer::CheckVkResult (
        vkCreateFence ( device, &fenceInfo, nullptr, &_complete ),
        "editor::HelloTriangleJob::CreateMesh",
        "Can't create fence"
    );

    if ( !result ) [[unlikely]]
        return;

    AV_SET_VULKAN_OBJECT_NAME ( device, _complete, VK_OBJECT_TYPE_FENCE, "Hello triangle" )

    VkSubmitInfo const submitInfo
    {
        .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
        .pNext = nullptr,
        .waitSemaphoreCount = 0U,
        .pWaitSemaphores = nullptr,
        .pWaitDstStageMask = nullptr,
        .commandBufferCount = 1U,
        .pCommandBuffers = &commandBuffer,
        .signalSemaphoreCount = 0U,
        .pSignalSemaphores = nullptr
    };

    {
        std::lock_guard const lock ( submitMutex );

        result = android_vulkan::Renderer::CheckVkResult (
            vkQueueSubmit ( _renderer.GetQueue (), 1U, &submitInfo, _complete ),
            "editor::HelloTriangleJob::CreateMesh",
            "Can't submit command"
        );
    }

    if ( !result ) [[unlikely]]
    {
        _geometry->FreeResources ( _renderer );
        _geometry.reset ();
        return;
    }

    result = android_vulkan::Renderer::CheckVkResult (
        vkWaitForFences ( device, 1U, &_complete, VK_TRUE, std::numeric_limits<uint64_t>::max () ),
        "editor::HelloTriangleJob::CreateMesh",
        "Can't wait fence"
    );

    if ( result ) [[likely]]
    {
        _geometry->FreeTransferResources ( _renderer );
        return;
    }

    _geometry->FreeResources ( _renderer );
    _geometry.reset ();
}

void HelloTriangleJob::CreateProgram () noexcept
{
    AV_TRACE ( "Program" )

    _program = std::make_unique<HelloTriangleProgram> ();

    if ( _program->Init ( _renderer.GetDevice (), RENDER_TARGET_FORMAT ) ) [[likely]]
        return;

    _program->Destroy ( _renderer.GetDevice () );
    _program.reset ();
}

} // end of anonymous namespace

//----------------------------------------------------------------------------------------------------------------------

RenderSession::RenderSession ( UIManager &uiManager, Workspace &workspace ) noexcept:
    _workspace ( workspace ),
    _uiManager ( uiManager )
{
   // NOTHING
}

void RenderSession::Init () noexcept
{
    AV_TRACE ( "RenderSession: init" )

    _thread = std::thread (
        [ this ]() noexcept
        {
            AV_THREAD_NAME ( "Render session" )
            EventLoop ();
        }
    );
}

void RenderSession::Destroy () noexcept
{
    AV_TRACE ( "RenderSession: destroy" )

    if ( _thread.joinable () ) [[likely]]
    {
        _thread.join ();
    }
}

bool RenderSession::AllocateCommandBuffers ( VkDevice device ) noexcept
{
    constexpr VkFenceCreateInfo fenceInfo
    {
        .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0U
    };

    constexpr VkSemaphoreCreateInfo semaphoreInfo
    {
        .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0U
    };

    VkCommandPoolCreateInfo const poolInfo
    {
        .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
        .pNext = nullptr,
        .flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT,
        .queueFamilyIndex = NativeRenderer::Instance ().GetQueueFamilyIndex ()
    };

    VkCommandBufferAllocateInfo bufferAllocateInfo
    {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
        .pNext = nullptr,
        .commandPool = VK_NULL_HANDLE,
        .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
        .commandBufferCount = 1U
    };

    for ( size_t i = 0U; i < pbr::FIF_COUNT; ++i )
    {
        CommandInfo &info = _commandInfo[ i ];
        info._inUse = false;

        bool result = android_vulkan::Renderer::CheckVkResult (
            vkCreateFence ( device, &fenceInfo, nullptr, &info._fence ),
            "editor::RenderSession::AllocateCommandBuffers",
            "Can't create fence"
        );

        if ( !result ) [[unlikely]]
            return false;

        AV_SET_VULKAN_OBJECT_NAME ( device, info._fence, VK_OBJECT_TYPE_FENCE, "Frame in flight #%zu", i )

        result = android_vulkan::Renderer::CheckVkResult (
            vkCreateSemaphore ( device, &semaphoreInfo, nullptr, &info._acquire ),
            "editor::RenderSession::AllocateCommandBuffers",
            "Can't create render target acquired semaphore"
        );

        if ( !result ) [[unlikely]]
            return false;

        AV_SET_VULKAN_OBJECT_NAME ( device,
            info._acquire,
            VK_OBJECT_TYPE_SEMAPHORE,
            "Frame in flight #%zu",
            i
        )

        result = android_vulkan::Renderer::CheckVkResult (
            vkCreateCommandPool ( device, &poolInfo, nullptr, &info._pool ),
            "editor::RenderSession::AllocateCommandBuffers",
            "Can't create lead command pool"
        );

        if ( !result ) [[unlikely]]
            return false;

        AV_SET_VULKAN_OBJECT_NAME ( device,
            info._pool,
            VK_OBJECT_TYPE_COMMAND_POOL,
            "Frame in flight #%zu",
            i
        )

        bufferAllocateInfo.commandPool = info._pool;

        result = android_vulkan::Renderer::CheckVkResult (
            vkAllocateCommandBuffers ( device, &bufferAllocateInfo, &info._buffer ),
            "editor::RenderSession::AllocateCommandBuffers",
            "Can't allocate command buffer"
        );

        if ( !result ) [[unlikely]]
            return false;

        AV_SET_VULKAN_OBJECT_NAME ( device,
            info._buffer,
            VK_OBJECT_TYPE_COMMAND_BUFFER,
            "Frame in flight #%zu",
            i
        )
    }

    return true;
}

void RenderSession::FreeCommandBuffers ( VkDevice device ) noexcept
{
    for ( auto &commandInfo : _commandInfo )
    {
        if ( VkCommandPool &pool = commandInfo._pool; pool != VK_NULL_HANDLE ) [[likely]]
            vkDestroyCommandPool ( device, std::exchange ( pool, VK_NULL_HANDLE ), nullptr );

        if ( VkSemaphore &acquire = commandInfo._acquire; acquire != VK_NULL_HANDLE ) [[likely]]
            vkDestroySemaphore ( device, std::exchange ( acquire, VK_NULL_HANDLE ), nullptr );

        if ( VkFence &fence = commandInfo._fence; fence != VK_NULL_HANDLE ) [[likely]]
        {
            vkDestroyFence ( device, std::exchange ( fence, VK_NULL_HANDLE ), nullptr );
        }
    }
}

bool RenderSession::CreateRenderTarget () noexcept
{
    VkExtent2D &resolution = _renderingInfo.renderArea.extent;
    pbr::ExposureSpecialization const specData ( NativeRenderer::Instance ().GetSurfaceSize () );
    resolution = specData._mip0Resolution;

    if ( !CreateRenderTargetImage ( resolution ) ) [[unlikely]]
        return false;

    _viewport =
    {
        .x = 0.0F,
        .y = 0.0F,
        .width = static_cast<float> ( resolution.width ),
        .height = static_cast<float> ( resolution.height ),
        .minDepth = 0.0F,
        .maxDepth = 1.0F
    };

    return true;
}

bool RenderSession::CreateRenderTargetImage ( VkExtent2D const &resolution ) noexcept
{
    android_vulkan::Renderer &renderer = NativeRenderer::Instance ();

    bool const result = _renderTarget.CreateRenderTarget ( resolution,
        RENDER_TARGET_FORMAT,
        AV_VK_FLAG ( VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT ) | AV_VK_FLAG ( VK_IMAGE_USAGE_SAMPLED_BIT ),
        renderer
    );

    if ( !result ) [[unlikely]]
        return false;

    VkDevice device = renderer.GetDevice ();
    _barrier.image = _renderTarget.GetImage ();
    AV_SET_VULKAN_OBJECT_NAME ( device, _barrier.image, VK_OBJECT_TYPE_IMAGE, "Render target" )

    _colorAttachment.imageView = _renderTarget.GetImageView ();
    AV_SET_VULKAN_OBJECT_NAME ( device, _colorAttachment.imageView, VK_OBJECT_TYPE_IMAGE_VIEW, "Render target" )

    auto const idx = ResourceHeap::Instance ().RegisterNonUISampledImage ( device, _colorAttachment.imageView );

    if ( !idx ) [[unlikely]]
        return false;

    _renderTargetIdx = *idx;
    return true;
}

void RenderSession::EventLoop () noexcept
{
    if ( !InitModules () ) [[unlikely]]
        _broken = true;

    MessageQueue &messageQueue = MessageQueue::Instance ();

    if ( _broken )
    {
        messageQueue.EnqueueBack (
            {
                ._type = eMessageType::CloseEditor,
                ._action = nullptr,
                ._serialNumber = 0U
            }
        );
    }
    else
    {
      messageQueue.EnqueueBack (
            {
                ._type = eMessageType::ModuleStarted,
                ._action = nullptr,
                ._serialNumber = 0U
            }
        );
    }

    std::optional<Message::SerialNumber> lastRefund {};

    for ( ; ; )
    {
        AV_TRACE ( "Event loop" )
        Message message = messageQueue.DequeueBegin ( lastRefund );

        GX_DISABLE_WARNING ( 4061 )

        switch ( message._type )
        {
            case eMessageType::DestroyMesh:
                OnDestroyMesh ( messageQueue, std::move ( message ) );
            break;

            case eMessageType::DestroyTexture2D:
                OnDestroyTexture2D ( messageQueue, std::move ( message ) );
            break;

            case eMessageType::HelloTriangleReady:
                OnHelloTriangleReady ( message._action () );
            break;

            case eMessageType::InvokeRenderSession:
                OnInvokeRenderSession ( messageQueue, std::move ( message ) );
            break;

            case eMessageType::RenderFrame:
                OnRenderFrame ( messageQueue );
            break;

            case eMessageType::Shutdown:
                OnShutdown ( messageQueue, std::move ( message ) );
            return;

            case eMessageType::SwapchainCreated:
                OnSwapchainCreated ( messageQueue );
            break;

            case eMessageType::UIAppendChildElement:
                OnUIAppendChildElement ( messageQueue, std::move ( message ) );
            break;

            case eMessageType::UIDeleteElement:
                OnUIDeleteElement ( messageQueue, std::move ( message ) );
            break;

            case eMessageType::UIElementCreated:
                OnUIElementCreated ( messageQueue );
            break;

            case eMessageType::UIShowElement:
                OnUIShowElement ( messageQueue, std::move ( message ) );
            break;

            case eMessageType::UIPrependChildElement:
                OnUIPrependChildElement ( messageQueue, std::move ( message ) );
            break;

            case eMessageType::UIHideElement:
                OnUIHideElement ( messageQueue, std::move ( message ) );
            break;

            case eMessageType::UISetText:
                OnUISetText ( messageQueue, std::move ( message ) );
            break;

            case eMessageType::UIUpdateElement:
                OnUIUpdateElement ( messageQueue, std::move ( message ) );
            break;

            case eMessageType::UploadMesh:
                OnUploadMesh ( messageQueue, std::move ( message ) );
            break;

            case eMessageType::UploadTexture2D:
                OnUploadTexture2D ( messageQueue, std::move ( message ) );
            break;

            default:
                lastRefund = message._serialNumber;
                messageQueue.DequeueEnd ( std::move ( message ), MessageQueue::eRefundLocation::Front );
            break;
        }

        GX_ENABLE_WARNING ( 4061 )
    }
}

bool RenderSession::InitModules () noexcept
{
    AV_TRACE ( "Init modules" )
    android_vulkan::Renderer &renderer = NativeRenderer::Instance ();
    VkDevice device = renderer.GetDevice ();

    new HelloTriangleJob ( renderer, _submitMutex );

    if ( !AllocateCommandBuffers ( device ) || !_presentRenderPass.OnSwapchainCreated ( renderer ) ) [[unlikely]]
        return false;

    VkCommandPool pool = _commandInfo[ 0U ]._pool;

    VkCommandBufferAllocateInfo const allocateInfo
    {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
        .pNext = nullptr,
        .commandPool = pool,
        .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
        .commandBufferCount = 1U
    };

    constexpr VkCommandBufferBeginInfo beginCommandBuffer
    {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
        .pNext = nullptr,
        .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
        .pInheritanceInfo = nullptr
    };

    VkCommandBuffer commandBuffer;

    bool result =
        android_vulkan::Renderer::CheckVkResult (
            vkAllocateCommandBuffers ( device, &allocateInfo, &commandBuffer ),
            "editor::RenderSession::InitModules",
            "Can't allocate command buffer"
        ) &&

        android_vulkan::Renderer::CheckVkResult ( vkBeginCommandBuffer ( commandBuffer, &beginCommandBuffer ),
            "editor::RenderSession::InitModules",
            "Can't begin command buffer"
        );

    if ( !result ) [[unlikely]]
        return false;

    AV_SET_VULKAN_OBJECT_NAME ( device, commandBuffer, VK_OBJECT_TYPE_COMMAND_BUFFER, "Engine init" )
    pbr::ResourceHeap &resourceHeap = ResourceHeap::Instance ();

    {
        std::lock_guard const lock ( _submitMutex );
        result = resourceHeap.Init ( renderer, commandBuffer ) && _exposurePass.Init ( renderer, resourceHeap, pool );

        if ( !result ) [[unlikely]]
        {
            return false;
        }
    }

    result = _uiPass.OnInitDevice ( renderer ) &&

        android_vulkan::Renderer::CheckVkResult ( vkEndCommandBuffer ( commandBuffer ),
            "editor::RenderSession::InitModules",
            "Can't end command buffer"
        );

    if ( !result ) [[unlikely]]
        return false;

    VkSubmitInfo const submit
    {
        .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
        .pNext = nullptr,
        .waitSemaphoreCount = 0U,
        .pWaitSemaphores = nullptr,
        .pWaitDstStageMask = nullptr,
        .commandBufferCount = 1U,
        .pCommandBuffers = &commandBuffer,
        .signalSemaphoreCount = 0U,
        .pSignalSemaphores = nullptr
    };

    VkQueue queue = renderer.GetQueue ();

    {
        std::lock_guard const lock ( _submitMutex );

        result = android_vulkan::Renderer::CheckVkResult ( vkQueueSubmit ( queue, 1U, &submit, VK_NULL_HANDLE ),
            "editor::RenderSession::InitModules",
            "Can't submit command buffer"
        );

        if ( !result ) [[unlikely]]
        {
            return false;
        }
    }

    result = CreateRenderTarget () &&
        _exposurePass.SetTarget ( renderer, resourceHeap, _renderTarget, _renderTargetIdx ) &&
        _toneMapper.SetBrightness ( renderer, DEFAULT_BRIGHTNESS_BALANCE ) &&
        _uiPass.OnSwapchainCreated ( renderer ) &&
        _uiPass.SetBrightness ( renderer, DEFAULT_BRIGHTNESS_BALANCE ) &&
        _toneMapper.SetTarget ( renderer, _renderTargetIdx, _exposurePass.GetExposure () ) &&

        android_vulkan::Renderer::CheckVkResult ( vkQueueWaitIdle ( queue ),
            "editor::RenderSession::InitModules",
            "Can't wait queue idle"
        );

    if ( !result ) [[unlikely]]
        return false;

    vkFreeCommandBuffers ( device, pool, 1U, &commandBuffer );
    _exposurePass.FreeTransferResources ( device, pool );
    _timestamp = std::chrono::steady_clock::now ();
    return true;
}

void RenderSession::FreeMeshTransferQueue ( MessageQueue &messageQueue, size_t commandBufferIndex ) noexcept
{
    auto &queue = _meshStorage._freeTransferQueue[ commandBufferIndex ];

    if ( queue.empty () ) [[likely]]
        return;

    AV_TRACE ( "Free mesh transfer resources" )
    android_vulkan::Renderer &renderer = NativeRenderer::Instance ();

    for ( auto &mesh : queue )
    {
        messageQueue.EnqueueBack (
            {
                ._type = eMessageType::InvokeIO,

                ._action = [ &renderer, mesh = mesh ] () noexcept -> void* {
                    android_vulkan::MeshGeometry &m = *mesh;
                    AV_TRACE ( "Free transfer resource (%s)", m.GetName ().c_str () );
                    m.FreeTransferResources ( renderer );
                    return nullptr;
                },

                ._serialNumber = 0U
            }
        );
    }

    queue.clear ();
}

void RenderSession::FreeTexture2DTransferQueue ( MessageQueue &messageQueue, size_t commandBufferIndex ) noexcept
{
    auto &queue = _texture2DStorage._freeTransferQueue[ commandBufferIndex ];

    if ( queue.empty () ) [[likely]]
        return;

    AV_TRACE ( "Free texture 2D transfer resources" )
    android_vulkan::Renderer &renderer = NativeRenderer::Instance ();

    for ( auto &texture2D : queue )
    {
        messageQueue.EnqueueBack (
            {
                ._type = eMessageType::InvokeIO,

                ._action = [ &renderer, texture2D = texture2D ] () noexcept -> void* {
                    android_vulkan::Texture2D &t = texture2D->_resource;
                    AV_TRACE ( "Free texture 2D resource (%s)", t.GetName ().c_str () );
                    t.FreeTransferResources ( renderer );
                    return nullptr;
                },

                ._serialNumber = 0U
            }
        );
    }

    queue.clear ();
}

void RenderSession::DestroyMeshes (  MessageQueue &messageQueue, size_t commandBufferIndex ) noexcept
{
    auto &toDestroy = _meshStorage._toDestroy;

    // Destroy on next frame.
    auto &scheduleToDestroy = _meshStorage._destroyQueue[ ( commandBufferIndex + 1U ) % pbr::FIF_COUNT ];

    for ( auto &item : toDestroy )
        scheduleToDestroy.push_back ( std::move ( item ) );

    toDestroy.clear ();
    auto &destroyQueue = _meshStorage._destroyQueue[ commandBufferIndex ];

    if ( destroyQueue.empty () ) [[likely]]
        return;

    AV_TRACE ( "Destroy meshes" )
    android_vulkan::Renderer &renderer = NativeRenderer::Instance ();

    for ( auto &mesh : destroyQueue )
    {
        AV_TRACE ( "Destroy mesh (%s)", mesh->GetName ().c_str () );

        // Calling method by pointer C++ syntax
        ( messageQueue.*_enqueueHandle ) (
            {
                ._type = eMessageType::InvokeIO,

                ._action = [ this, &messageQueue, &renderer, mesh = std::move ( mesh ) ] () noexcept -> void* {
                    android_vulkan::MeshGeometry &m = *mesh;
                    AV_TRACE ( "Destroy mesh (%s)", m.GetName ().c_str () );
                    m.FreeResources ( renderer );

                    // Calling method by pointer C++ syntax
                    ( messageQueue.*_enqueueHandle ) (
                        {
                            ._type = eMessageType::InvokeRenderSession,

                            ._action = [ this ] () noexcept -> void* {
                                AV_TRACE ( "Mesh destroy complete" );
                                --_meshStorage._count;
                                return nullptr;
                            },

                            ._serialNumber = 0U
                        }
                    );

                    return nullptr;
                },

                ._serialNumber = 0U
            }
        );
    }

    destroyQueue.clear ();
}

void RenderSession::DestroyTexture2DInstances ( MessageQueue &messageQueue, size_t commandBufferIndex ) noexcept
{
    auto &toDestroy = _texture2DStorage._toDestroy;

    // Destroy on next frame.
    auto &scheduleToDestroy = _texture2DStorage._destroyQueue[ ( commandBufferIndex + 1U ) % pbr::FIF_COUNT ];

    for ( auto &item : toDestroy )
        scheduleToDestroy.push_back ( std::move ( item ) );

    toDestroy.clear ();
    auto &destroyQueue = _texture2DStorage._destroyQueue[ commandBufferIndex ];

    if ( destroyQueue.empty () ) [[likely]]
        return;

    AV_TRACE ( "Destroy textures" )
    android_vulkan::Renderer &renderer = NativeRenderer::Instance ();

    for ( auto &texture2D : destroyQueue )
    {
        AV_TRACE ( "Destroy 2D textures (%s)", texture2D->_resource.GetName ().c_str () );

        // Calling method by pointer C++ syntax
        ( messageQueue.*_enqueueHandle ) (
            {
                ._type = eMessageType::InvokeIO,

                ._action = [ this,
                    &messageQueue,
                    &renderer,
                    texture2D = std::move ( texture2D )
                ] () noexcept -> void* {
                    android_vulkan::Texture2D &t = texture2D->_resource;
                    AV_TRACE ( "Destroy 2D texture (%s)", t.GetName ().c_str () );
                    t.FreeResources ( renderer );

                    // Calling method by pointer C++ syntax
                    ( messageQueue.*_enqueueHandle ) (
                        {
                            ._type = eMessageType::InvokeRenderSession,

                            ._action = [ this ] () noexcept -> void* {
                                AV_TRACE ( "Mesh destroy complete" );
                                --_texture2DStorage._count;
                                return nullptr;
                            },

                            ._serialNumber = 0U
                        }
                    );

                    return nullptr;
                },

                ._serialNumber = 0U
            }
        );
    }

    destroyQueue.clear ();
}

void RenderSession::UploadMeshes ( VkCommandBuffer commandBuffer, size_t commandBufferIndex ) noexcept
{
    auto &uploadQueue = _meshStorage._uploadQueue;

    if ( uploadQueue.empty () ) [[likely]]
        return;

    AV_TRACE ( "Upload meshes" )
    AV_VULKAN_GROUP ( commandBuffer, "Upload meshes" )

    android_vulkan::Renderer &renderer = NativeRenderer::Instance ();
    auto &transferQueue = _meshStorage._freeTransferQueue[ commandBufferIndex ];

    for ( auto &info : uploadQueue )
    {
        MeshGeometryRef &mRef = info._mesh;
        android_vulkan::MeshGeometry &m = *mRef;

        AV_TRACE ( "Upload '%s'", m.GetName ().c_str () );
        AV_VULKAN_GROUP ( commandBuffer, "Upload '%s'", m.GetName ().c_str () );

        if ( !m.UploadToGPU ( renderer, commandBuffer, VK_NULL_HANDLE, std::move ( info._info ) ) ) [[unlikely]]
        {
            info._result ( std::nullopt );
            continue;
        }

        transferQueue.push_back ( mRef );
        ++_meshStorage._count;
        info._result ( std::optional<MeshGeometryRef> { std::move ( mRef ) } );
    }

    uploadQueue.clear ();
}

void RenderSession::UploadTexture2DInstances ( VkCommandBuffer commandBuffer, size_t commandBufferIndex ) noexcept
{
    auto &uploadQueue = _texture2DStorage._uploadQueue;

    if ( uploadQueue.empty () ) [[likely]]
        return;

    AV_TRACE ( "Upload texture 2D instances" )
    AV_VULKAN_GROUP ( commandBuffer, "Upload texture 2D instances" )

    android_vulkan::Renderer &renderer = NativeRenderer::Instance ();
    auto &transferQueue = _texture2DStorage._freeTransferQueue[ commandBufferIndex ];

    for ( auto &info : uploadQueue )
    {
        Texture2DRef &tRef = info._texture;
        android_vulkan::Texture2D &t = tRef->_resource;

        AV_TRACE ( "Upload '%s'", t.GetName ().c_str () );
        AV_VULKAN_GROUP ( commandBuffer, "Upload '%s'", t.GetName ().c_str () );

        if ( !t.UploadToGPU ( renderer, commandBuffer, true, VK_NULL_HANDLE ) ) [[unlikely]]
        {
            info._result ( std::nullopt );
            continue;
        }

        transferQueue.push_back ( tRef );
        ++_texture2DStorage._count;
        info._result ( std::optional<Texture2DRef> { std::move ( tRef ) } );
    }

    uploadQueue.clear ();
}

void RenderSession::OnHelloTriangleReady ( void* params ) noexcept
{
    MessageQueue::Instance ().DequeueEnd ();
    auto* job = static_cast<HelloTriangleJob*> ( params );
    _helloTriangleProgram = std::move ( job->_program );
    _helloTriangleGeometry = std::move ( job->_geometry );
    delete job;
}

void RenderSession::OnDestroyMesh ( MessageQueue &messageQueue, Message &&message ) noexcept
{
    AV_TRACE ( "Destroy mesh" )
    messageQueue.DequeueEnd ();
    _meshStorage._toDestroy.push_back ( std::move ( *static_cast<MeshGeometryRef*> ( message._action () ) ) );
}

void RenderSession::OnDestroyTexture2D ( MessageQueue &messageQueue, Message &&message ) noexcept
{
    AV_TRACE ( "Destroy mesh" )
    messageQueue.DequeueEnd ();
    _texture2DStorage._toDestroy.push_back ( std::move ( *static_cast<Texture2DRef*> ( message._action () ) ) );
}

void RenderSession::OnInvokeRenderSession ( MessageQueue &messageQueue, Message &&message ) noexcept
{
    AV_TRACE ( "Invoke" )
    messageQueue.DequeueEnd ();
    std::ignore = message._action ();
}

void RenderSession::OnRenderFrame ( MessageQueue &messageQueue ) noexcept
{
    AV_TRACE ( "Render frame" )
    messageQueue.DequeueEnd ();

    if ( _broken ) [[unlikely]]
        return;

    Timestamp const now = std::chrono::steady_clock::now ();
    std::chrono::duration<float> const seconds = now - _timestamp;
    float const deltaTime = seconds.count ();
    _timestamp = now;

    size_t const commandBufferIndex = _writingCommandInfo;
    CommandInfo &commandInfo = _commandInfo[ _writingCommandInfo ];
    _writingCommandInfo = ++_writingCommandInfo % pbr::FIF_COUNT;

    android_vulkan::Renderer &renderer = NativeRenderer::Instance ();
    _uiManager.ComputeLayout ( renderer, _uiPass );
    VkDevice device = renderer.GetDevice ();

    if ( !PrepareCommandBuffer ( device, commandInfo ) ) [[unlikely]]
    {
        AV_ASSERT ( false )
        return;
    }

    VkResult vulkanResult = _presentRenderPass.AcquirePresentTarget ( renderer, commandInfo._acquire );

    if ( vulkanResult == VK_ERROR_OUT_OF_DATE_KHR ) [[unlikely]]
    {
        NotifyRecreateSwapchain ( messageQueue );
        return;
    }

    FreeMeshTransferQueue ( messageQueue, commandBufferIndex );
    FreeTexture2DTransferQueue ( messageQueue, commandBufferIndex );

    DestroyMeshes ( messageQueue, commandBufferIndex );
    DestroyTexture2DInstances ( messageQueue, commandBufferIndex );

    if ( ( vulkanResult != VK_SUCCESS ) & ( vulkanResult != VK_SUBOPTIMAL_KHR ) ) [[unlikely]]
    {
        [[maybe_unused]] bool const result = android_vulkan::Renderer::CheckVkResult ( vulkanResult,
            "editor::RenderSession::OnRenderFrame",
            "Can't acquire present image"
        );

        AV_ASSERT ( false )
        return;
    }

    VkCommandBuffer commandBuffer = commandInfo._buffer;
    commandInfo._inUse = true;

    constexpr VkCommandBufferBeginInfo beginInfo
    {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
        .pNext = nullptr,
        .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
        .pInheritanceInfo = nullptr
    };

    bool result = android_vulkan::Renderer::CheckVkResult ( vkBeginCommandBuffer ( commandBuffer, &beginInfo ),
        "editor::RenderSession::OnRenderFrame",
        "Can't begin main render pass"
    );

    if ( !result ) [[unlikely]]
    {
        AV_ASSERT ( false )
        return;
    }

    _workspace.ComputeTransform ( deltaTime );

    UploadMeshes ( commandBuffer, commandBufferIndex );
    UploadTexture2DInstances ( commandBuffer, commandBufferIndex );

    pbr::ResourceHeap &resourceHeap = ResourceHeap::Instance ();
    resourceHeap.UploadGPUData ( commandBuffer );

    {
        AV_VULKAN_GROUP ( commandBuffer, "Upload" )

        if ( !_uiPass.UploadGPUFontData ( renderer, commandBuffer ) ) [[unlikely]]
        {
            AV_ASSERT ( false )
            return;
        }

        _uiManager.Submit ( renderer, _uiPass );
        _uiPass.UploadGPUGeometryData ( renderer, commandBuffer );
    }

    _workspace.UploadToGPU ( commandBuffer );
    _workspace.DrawOpaque ( commandBuffer );

    {
        AV_VULKAN_GROUP ( commandBuffer, "Scene" )

        _barrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
        _barrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
        _barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        _barrier.newLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

        vkCmdPipelineBarrier ( commandBuffer, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
            VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
            VK_DEPENDENCY_BY_REGION_BIT,
            0U,
            nullptr,
            0U,
            nullptr,
            1U,
            &_barrier
        );

        vkCmdBeginRendering ( commandBuffer, &_renderingInfo );
        vkCmdSetViewport ( commandBuffer, 0U, 1U, &_viewport );
        vkCmdSetScissor ( commandBuffer, 0U, 1U, &_renderingInfo.renderArea );

        if ( static_cast<bool> ( _helloTriangleGeometry ) ) [[likely]]
        {
            _helloTriangleProgram->Bind ( commandBuffer );

            HelloTriangleProgram::PushConstants const geometry
            {
                ._bda = _helloTriangleGeometry->GetMeshBufferInfo ()._bdaStream0
            };

            vkCmdPushConstants ( commandBuffer,
                _helloTriangleProgram->GetPipelineLayout (),
                VK_SHADER_STAGE_VERTEX_BIT,
                0U,
                sizeof ( HelloTriangleProgram::PushConstants ),
                &geometry
            );

            vkCmdDraw ( commandBuffer, _helloTriangleGeometry->GetVertexCount (), 1U, 0U, 0U );
        }

        vkCmdEndRendering ( commandBuffer );

        _barrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
        _barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
        _barrier.oldLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        _barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

        constexpr VkPipelineStageFlags dstStages = AV_VK_FLAG ( VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT ) |
            AV_VK_FLAG ( VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT );

        vkCmdPipelineBarrier ( commandBuffer, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
            dstStages,
            VK_DEPENDENCY_BY_REGION_BIT,
            0U,
            nullptr,
            0U,
            nullptr,
            1U,
            &_barrier
        );
    }

    _exposurePass.Execute ( commandBuffer, deltaTime, resourceHeap );

    {
        AV_VULKAN_GROUP ( commandBuffer, "Present" )
        _workspace.DrawGizmo ( commandBuffer );

        _presentRenderPass.Begin ( renderer, commandBuffer );
        _toneMapper.Execute ( commandBuffer, resourceHeap );

        if ( !_uiPass.Execute ( commandBuffer, commandBufferIndex ) ) [[unlikely]]
        {
            AV_ASSERT ( false )
            return;
        }
    }

    std::optional<VkResult> const presentResult = _presentRenderPass.End ( renderer,
        commandBuffer,
        commandInfo._acquire,
        commandInfo._fence,
        &_submitMutex
    );

    if ( !presentResult ) [[unlikely]]
    {
        AV_ASSERT ( false )
        return;
    }

    GX_DISABLE_WARNING ( 4061 )

    switch ( vulkanResult = *presentResult; vulkanResult )
    {
        case VK_SUCCESS:
            // NOTHING
        break;

        case VK_SUBOPTIMAL_KHR:
            [[fallthrough]];

        case VK_ERROR_OUT_OF_DATE_KHR:
            NotifyRecreateSwapchain ( messageQueue );
        return;

        default:
            result = android_vulkan::Renderer::CheckVkResult ( vulkanResult,
                "editor::RenderSession::OnRenderFrame",
                "Can't present frame"
            );

            AV_ASSERT ( false )
        return;
    }

    GX_ENABLE_WARNING ( 4061 )

    messageQueue.EnqueueBack (
        {
            ._type = eMessageType::FrameComplete,
            ._action = nullptr,
            ._serialNumber = 0U
        }
    );
}

void RenderSession::OnShutdown ( MessageQueue &messageQueue, Message &&refund ) noexcept
{
    AV_TRACE ( "Shutdown" )

    // All existing events should be processed first.
    messageQueue.DequeueEnd ( std::move ( refund ), MessageQueue::eRefundLocation::Back );

    _enqueueHandle = &MessageQueue::EnqueueFront;
    android_vulkan::Renderer &renderer = NativeRenderer::Instance ();

    bool const result = android_vulkan::Renderer::CheckVkResult ( vkQueueWaitIdle ( renderer.GetQueue () ),
        "editor::RenderSession::OnShutdown",
        "Can't wait queue idle"
    );

    if ( !result ) [[unlikely]]
        android_vulkan::LogError ( "Render session error. Can't stop." );

    std::optional<Message::SerialNumber> lastRefund {};

    while ( _uiElements | _meshStorage._count | _texture2DStorage._count )
    {
        AV_TRACE ( "Event loop" )
        Message message = messageQueue.DequeueBegin ( lastRefund );

        GX_DISABLE_WARNING ( 4061 )

        switch ( message._type )
        {
            case eMessageType::RunEventLoop:
                [[fallthrough]];
            case eMessageType::Shutdown:
                // All existing events should be processed first.
                messageQueue.DequeueEnd ( std::move ( message ), MessageQueue::eRefundLocation::Back );
                DestroyMeshes ( messageQueue, _writingCommandInfo );

                DestroyTexture2DInstances ( messageQueue,
                    std::exchange ( _writingCommandInfo, ( _writingCommandInfo + 1U ) % pbr::FIF_COUNT )
                );
            break;

            case eMessageType::DestroyMesh:
                OnDestroyMesh ( messageQueue, std::move ( message ) );
            break;

            case eMessageType::DestroyTexture2D:
                OnDestroyTexture2D ( messageQueue, std::move ( message ) );
            break;

            case eMessageType::InvokeRenderSession:
                OnInvokeRenderSession ( messageQueue, std::move ( message ) );
            break;

            case eMessageType::UIAppendChildElement:
                OnUIAppendChildElement ( messageQueue, std::move ( message ) );
            break;

            case eMessageType::UIDeleteElement:
                OnUIDeleteElement ( messageQueue, std::move ( message ) );
            break;

            case eMessageType::UIElementCreated:
                OnUIElementCreated ( messageQueue );
            break;

            case eMessageType::UIPrependChildElement:
                OnUIPrependChildElement ( messageQueue, std::move ( message ) );
            break;

            case eMessageType::UISetText:
                OnUISetText ( messageQueue, std::move ( message ) );
            break;

            default:
                lastRefund = message._serialNumber;
                messageQueue.DequeueEnd ( std::move ( message ), MessageQueue::eRefundLocation::Front );
            break;
        }

        GX_ENABLE_WARNING ( 4061 )
    }

    VkDevice device = renderer.GetDevice ();
    FreeCommandBuffers ( device );

    if ( _helloTriangleProgram ) [[likely]]
    {
        _helloTriangleProgram->Destroy ( device );
        _helloTriangleProgram.reset ();
    }

    if ( _helloTriangleGeometry ) [[likely]]
    {
        _helloTriangleGeometry->FreeResources ( renderer );
        _helloTriangleGeometry.reset ();
    }

    pbr::ResourceHeap &resourceHeap = ResourceHeap::Instance ();

    if ( _renderTargetIdx ) [[likely]]
        resourceHeap.UnregisterResource ( std::exchange ( _renderTargetIdx, 0U ) );

    _renderTarget.FreeResources ( renderer );

    _uiPass.OnSwapchainDestroyed ();
    _uiPass.OnDestroyDevice ( renderer );

    _presentRenderPass.OnDestroyDevice ( device );
    _exposurePass.Destroy ( renderer, resourceHeap );
    _toneMapper.Destroy ( device );
    resourceHeap.Destroy ( renderer );

    _meshStorage = {};
    _texture2DStorage = {};

    messageQueue.EnqueueFront (
        {
            ._type = eMessageType::ModuleStopped,
            ._action = nullptr,
            ._serialNumber = 0U
        }
    );
}

void RenderSession::OnSwapchainCreated ( MessageQueue &messageQueue ) noexcept
{
    AV_TRACE ( "Swapchain created" )
    messageQueue.DequeueEnd ();
    android_vulkan::Renderer &renderer = NativeRenderer::Instance ();

    if ( !_presentRenderPass.OnSwapchainCreated ( renderer ) ) [[unlikely]]
    {
         AV_ASSERT ( false )
        return;
    }

    pbr::ExposureSpecialization const specData ( renderer.GetSurfaceSize () );
    VkExtent2D &resolution = _renderingInfo.renderArea.extent;
    resolution = specData._mip0Resolution;

    _viewport =
    {
        .x = 0.0F,
        .y = 0.0F,
        .width = static_cast<float> ( resolution.width ),
        .height = static_cast<float> ( resolution.height ),
        .minDepth = 0.0F,
        .maxDepth = 1.0F
    };

    if ( _renderTargetIdx ) [[likely]]
        ResourceHeap::Instance ().UnregisterResource ( std::exchange ( _renderTargetIdx, 0U ) );

    _renderTarget.FreeResources ( renderer );

    if ( !CreateRenderTargetImage ( resolution ) ) [[unlikely]]
    {
        AV_ASSERT ( false )
        return;
    }

    _uiPass.OnSwapchainDestroyed ();

    bool const result = _uiPass.OnSwapchainCreated ( renderer ) &&
        _toneMapper.SetTarget ( renderer, _renderTargetIdx, _exposurePass.GetExposure () );

    if ( result ) [[likely]]
        return;

    android_vulkan::LogError ( "editor::RenderSession::OnSwapchainCreated - Can't create UI pass." );
    AV_ASSERT ( false )
}

void RenderSession::OnUIAppendChildElement ( MessageQueue &messageQueue, Message &&message ) noexcept
{
    AV_TRACE ( "UI append child element" )
    messageQueue.DequeueEnd ();
    std::ignore = message._action ();
}

void RenderSession::OnUIDeleteElement ( MessageQueue &messageQueue, Message &&message ) noexcept
{
    AV_TRACE ( "UI delete element" )
    messageQueue.DequeueEnd ();
    std::ignore = message._action ();
    --_uiElements;
}

void RenderSession::OnUIElementCreated ( MessageQueue &messageQueue ) noexcept
{
    AV_TRACE ( "UI element created" )
    messageQueue.DequeueEnd ();
    ++_uiElements;
}

void RenderSession::OnUIHideElement ( MessageQueue &messageQueue, Message &&message ) noexcept
{
    AV_TRACE ( "UI hide element" )
    messageQueue.DequeueEnd ();
    std::ignore = message._action ();
}

void RenderSession::OnUIShowElement ( MessageQueue &messageQueue, Message &&message ) noexcept
{
    AV_TRACE ( "UI show element" )
    messageQueue.DequeueEnd ();
    std::ignore = message._action ();
}

void RenderSession::OnUIPrependChildElement ( MessageQueue &messageQueue, Message &&message ) noexcept
{
    AV_TRACE ( "UI prepend child element" )
    messageQueue.DequeueEnd ();
    std::ignore = message._action ();
}

void RenderSession::OnUISetText ( MessageQueue &messageQueue, Message &&message ) noexcept
{
    AV_TRACE ( "UI set text" )
    messageQueue.DequeueEnd ();
    std::ignore = message._action ();
}

void RenderSession::OnUIUpdateElement ( MessageQueue &messageQueue, Message &&message ) noexcept
{
    AV_TRACE ( "UI update element" )
    messageQueue.DequeueEnd ();
    std::ignore = message._action ();
}

void RenderSession::OnUploadMesh ( MessageQueue &messageQueue, Message &&message ) noexcept
{
    AV_TRACE ( "Upload mesh" )
    messageQueue.DequeueEnd ();
    _meshStorage._uploadQueue.push_back ( std::move ( *static_cast<MeshUploadInfo*> ( message._action () ) ) );
}

void RenderSession::OnUploadTexture2D ( MessageQueue &messageQueue, Message &&message ) noexcept
{
    AV_TRACE ( "Upload texture 2D" )
    messageQueue.DequeueEnd ();

    _texture2DStorage._uploadQueue.push_back (
        std::move ( *static_cast<Texture2DUploadInfo*> ( message._action () ) )
    );
}

void RenderSession::NotifyRecreateSwapchain ( MessageQueue &messageQueue ) const noexcept
{
    messageQueue.EnqueueBack (
        {
            ._type = eMessageType::RecreateSwapchain,
            ._action = nullptr,
            ._serialNumber = 0U
        }
    );

    messageQueue.EnqueueBack (
        {
            ._type = eMessageType::FrameComplete,
            ._action = nullptr,
            ._serialNumber = 0U
        }
    );
}

bool RenderSession::PrepareCommandBuffer ( VkDevice device, CommandInfo &info ) noexcept
{
    if ( !info._inUse ) [[unlikely]]
        return true;

    info._inUse = false;

    return android_vulkan::Renderer::CheckVkResult (
            vkWaitForFences ( device, 1U, &info._fence, VK_TRUE, std::numeric_limits<uint64_t>::max () ),
            "editor::RenderSession::PrepareCommandBuffer",
            "Can't wait fence"
        ) &&

        android_vulkan::Renderer::CheckVkResult ( vkResetFences ( device, 1U, &info._fence ),
            "editor::RenderSession::PrepareCommandBuffer",
            "Can't reset fence"
        ) &&

        android_vulkan::Renderer::CheckVkResult (
            vkResetCommandPool ( device, info._pool, 0U ),
            "editor::RenderSession::PrepareCommandBuffer",
            "Can't reset command pool"
        );
}

} // namespace editor
