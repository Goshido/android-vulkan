#include <precompiled_headers.hpp>
#include <append_ui_child_element_event.hpp>
#include <av_assert.hpp>
#include <hello_triangle_vertex.hpp>
#include <logger.hpp>
#include <prepend_ui_child_element_event.hpp>
#include <render_session.hpp>
#include <set_text_event.hpp>
#include <trace.hpp>
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

        explicit HelloTriangleJob ( MessageQueue &messageQueue,
            android_vulkan::Renderer &renderer,
            std::mutex &submitMutex
        ) noexcept;

        ~HelloTriangleJob ();

    private:
        void CreateMesh ( std::mutex &submitMutex ) noexcept;
        void CreateProgram () noexcept;
};

HelloTriangleJob::HelloTriangleJob ( MessageQueue &messageQueue,
    android_vulkan::Renderer &renderer,
    std::mutex &submitMutex
) noexcept:
    _renderer ( renderer )
{
    std::thread (
        [ this, &messageQueue, &submitMutex ] () noexcept
        {
            AV_THREAD_NAME ( "Hello triangle job" )
            CreateProgram ();
            CreateMesh ( submitMutex );

            messageQueue.EnqueueBack (
                {
                    ._type = eMessageType::HelloTriangleReady,
                    ._params = this,
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

        result = _geometry->LoadMesh ( _renderer,
            commandBuffer,
            true,
            VK_NULL_HANDLE,
            { reinterpret_cast<uint8_t const*> ( data ), sizeof ( data ) },
            static_cast<uint32_t> ( std::size ( data ) )
        );

        if ( !result ) [[unlikely]]
        {
            _geometry->FreeResources ( _renderer );
            _geometry.reset ();
            return;
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

RenderSession::RenderSession ( MessageQueue &messageQueue,
    android_vulkan::Renderer &renderer,
    UIManager &uiManager
) noexcept:
    _messageQueue ( messageQueue ),
    _renderer ( renderer ),
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

pbr::FontStorage& RenderSession::GetFontStorage () noexcept
{
    return _uiPass.GetFontStorage ();
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
        .queueFamilyIndex = _renderer.GetQueueFamilyIndex ()
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
    pbr::ExposureSpecialization const specData ( _renderer.GetSurfaceSize () );
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
    bool const result = _renderTarget.CreateRenderTarget ( resolution,
        RENDER_TARGET_FORMAT,
        AV_VK_FLAG ( VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT ) | AV_VK_FLAG ( VK_IMAGE_USAGE_SAMPLED_BIT ),
        _renderer
    );

    if ( !result ) [[unlikely]]
        return false;

    VkDevice device = _renderer.GetDevice ();
    _barrier.image = _renderTarget.GetImage ();
    AV_SET_VULKAN_OBJECT_NAME ( device, _barrier.image, VK_OBJECT_TYPE_IMAGE, "Render target" )

    _colorAttachment.imageView = _renderTarget.GetImageView ();
    AV_SET_VULKAN_OBJECT_NAME ( device, _colorAttachment.imageView, VK_OBJECT_TYPE_IMAGE_VIEW, "Render target" )

    if ( auto const idx = _resourceHeap.RegisterNonUISampledImage ( device, _colorAttachment.imageView ); idx )
    {
        [[likely]]
        _renderTargetIdx = *idx;
        return true;
    }

    return false;
}

void RenderSession::EventLoop () noexcept
{
    if ( !InitModules () ) [[unlikely]]
        _broken = true;

    MessageQueue &messageQueue = _messageQueue;

    if ( _broken )
    {
        messageQueue.EnqueueBack (
            {
                ._type = eMessageType::CloseEditor,
                ._params = nullptr,
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
            case eMessageType::HelloTriangleReady:
                OnHelloTriangleReady ( message._params );
            break;

            case eMessageType::RenderFrame:
                OnRenderFrame ();
            break;

            case eMessageType::Shutdown:
                OnShutdown ( std::move ( message ) );
            return;

            case eMessageType::SwapchainCreated:
                OnSwapchainCreated ();
            break;

            case eMessageType::UIAppendChildElement:
                OnUIAppendChildElement ( std::move ( message ) );
            break;

            case eMessageType::UIDeleteElement:
                OnUIDeleteElement ( std::move ( message ) );
            break;

            case eMessageType::UIElementCreated:
                OnUIElementCreated ();
            break;

            case eMessageType::UIShowElement:
                OnUIShowElement ( std::move ( message ) );
            break;

            case eMessageType::UIPrependChildElement:
                OnUIPrependChildElement ( std::move ( message ) );
            break;

            case eMessageType::UIHideElement:
                OnUIHideElement ( std::move ( message ) );
            break;

            case eMessageType::UISetText:
                OnUISetText ( std::move ( message ) );
            break;

            case eMessageType::UIUpdateElement:
                OnUIUpdateElement ( std::move ( message ) );
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
    android_vulkan::Renderer &renderer = _renderer;
    VkDevice device = renderer.GetDevice ();

    new HelloTriangleJob ( _messageQueue, renderer, _submitMutex );

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

    {
        std::lock_guard const lock ( _submitMutex );

        result = _resourceHeap.Init ( renderer, commandBuffer ) &&
            _exposurePass.Init ( renderer, _resourceHeap, pool );

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

    _messageQueue.EnqueueBack (
        {
            ._type = eMessageType::FontStorageReady,
            ._params = nullptr,
            ._serialNumber = 0U
        }
    );

    result = CreateRenderTarget () &&
        _exposurePass.SetTarget ( renderer, _resourceHeap, _renderTarget, _renderTargetIdx ) &&
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

void RenderSession::OnHelloTriangleReady ( void* params ) noexcept
{
    _messageQueue.DequeueEnd ();
    auto* job = static_cast<HelloTriangleJob*> ( params );
    _helloTriangleProgram = std::move ( job->_program );
    _helloTriangleGeometry = std::move ( job->_geometry );
    delete job;
}

void RenderSession::OnRenderFrame () noexcept
{
    AV_TRACE ( "Render frame" )
    _messageQueue.DequeueEnd ();

    if ( _broken ) [[unlikely]]
        return;

    Timestamp const now = std::chrono::steady_clock::now ();
    std::chrono::duration<float> const seconds = now - _timestamp;
    float const deltaTime = seconds.count ();
    _timestamp = now;

    size_t const commandBufferIndex = _writingCommandInfo;
    CommandInfo &commandInfo = _commandInfo[ _writingCommandInfo ];
    _writingCommandInfo = ++_writingCommandInfo % pbr::FIF_COUNT;

    android_vulkan::Renderer &renderer = _renderer;
    pbr::ResourceHeap &resourceHeap = _resourceHeap;
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
        NotifyRecreateSwapchain ();
        return;
    }

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
            NotifyRecreateSwapchain ();
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

    _messageQueue.EnqueueBack (
        {
            ._type = eMessageType::FrameComplete,
            ._params = nullptr,
            ._serialNumber = 0U
        }
    );
}

void RenderSession::OnShutdown ( Message &&refund ) noexcept
{
    AV_TRACE ( "Shutdown" )

    // All existing events should be processed first.
    _messageQueue.DequeueEnd ( std::move ( refund ), MessageQueue::eRefundLocation::Back );

    std::optional<Message::SerialNumber> lastRefund {};

    while ( _uiElements )
    {
        AV_TRACE ( "Event loop" )
        Message message = _messageQueue.DequeueBegin ( lastRefund );

        GX_DISABLE_WARNING ( 4061 )

        switch ( message._type )
        {
            case eMessageType::RunEventLoop:
            case eMessageType::Shutdown:
                // All existing events should be processed first.
                _messageQueue.DequeueEnd ( std::move ( message ), MessageQueue::eRefundLocation::Back );
            break;

            case eMessageType::UIAppendChildElement:
                OnUIAppendChildElement ( std::move ( message ) );
            break;

            case eMessageType::UIDeleteElement:
                OnUIDeleteElement ( std::move ( message ) );
            break;

            case eMessageType::UIElementCreated:
                OnUIElementCreated ();
            break;

            case eMessageType::UIPrependChildElement:
                OnUIPrependChildElement ( std::move ( message ) );
            break;

            case eMessageType::UISetText:
                OnUISetText ( std::move ( message ) );
            break;

            default:
                lastRefund = message._serialNumber;
                _messageQueue.DequeueEnd ( std::move ( message ), MessageQueue::eRefundLocation::Front );
            break;
        }

        GX_ENABLE_WARNING ( 4061 )
    }

    bool const result = android_vulkan::Renderer::CheckVkResult ( vkQueueWaitIdle ( _renderer.GetQueue () ),
        "editor::RenderSession::OnShutdown",
        "Can't wait queue idle"
    );

    if ( !result ) [[unlikely]]
        android_vulkan::LogError ( "Render session error. Can't stop." );

    android_vulkan::Renderer &renderer = _renderer;
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

    if ( _renderTargetIdx ) [[likely]]
        _resourceHeap.UnregisterResource ( std::exchange ( _renderTargetIdx, 0U ) );

    _renderTarget.FreeResources ( renderer );

    _uiPass.OnSwapchainDestroyed ();
    _uiPass.OnDestroyDevice ( renderer );

    _presentRenderPass.OnDestroyDevice ( device );
    _exposurePass.Destroy ( renderer, _resourceHeap );
    _toneMapper.Destroy ( device );
    _resourceHeap.Destroy ( renderer );

    _messageQueue.EnqueueFront (
        Message
        {
            ._type = eMessageType::ModuleStopped,
            ._params = nullptr,
            ._serialNumber = 0U
        }
    );
}

void RenderSession::OnSwapchainCreated () noexcept
{
    _messageQueue.DequeueEnd ();
    android_vulkan::Renderer &renderer = _renderer;

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
        _resourceHeap.UnregisterResource ( std::exchange ( _renderTargetIdx, 0U ) );

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

void RenderSession::OnUIAppendChildElement ( Message &&message ) noexcept
{
    AV_TRACE ( "UI append child element" )
    _messageQueue.DequeueEnd ();


    auto* event = static_cast<AppendUIChildElementEvent*> ( message._params );
    event->Action ();
    delete event;
}

void RenderSession::OnUIDeleteElement ( Message &&message ) noexcept
{
    AV_TRACE ( "UI delete element" )
    _messageQueue.DequeueEnd ();

    delete static_cast<pbr::UIElement*> ( message._params );
    --_uiElements;
}

void RenderSession::OnUIElementCreated () noexcept
{
    AV_TRACE ( "UI element created" )
    _messageQueue.DequeueEnd ();
    ++_uiElements;
}

void RenderSession::OnUIHideElement ( Message &&message ) noexcept
{
    AV_TRACE ( "UI hide element" )
    _messageQueue.DequeueEnd ();

    static_cast<pbr::UIElement*> ( message._params )->Hide ();
}

void RenderSession::OnUIShowElement ( Message &&message ) noexcept
{
    AV_TRACE ( "UI show element" )
    _messageQueue.DequeueEnd ();

    static_cast<pbr::UIElement*> ( message._params )->Show ();
}

void RenderSession::OnUIPrependChildElement ( Message &&message ) noexcept
{
    AV_TRACE ( "UI prepend child element" )
    _messageQueue.DequeueEnd ();

    auto* event = static_cast<PrependUIChildElementEvent*> ( message._params );
    event->Action ();
    delete event;
}

void RenderSession::OnUISetText ( Message &&message ) noexcept
{
    AV_TRACE ( "UI set text" )
    _messageQueue.DequeueEnd ();

    auto &event = *static_cast<SetTextEvent*> ( message._params );
    event.Execute ();
    SetTextEvent::Destroy ( event );
}

void RenderSession::OnUIUpdateElement ( Message &&message ) noexcept
{
    AV_TRACE ( "UI update element" )
    _messageQueue.DequeueEnd ();

    static_cast<pbr::DIVUIElement*> ( message._params )->Update ();
}

void RenderSession::NotifyRecreateSwapchain () const noexcept
{
    _messageQueue.EnqueueBack (
        {
            ._type = eMessageType::RecreateSwapchain,
            ._params = nullptr,
            ._serialNumber = 0U
        }
    );

    _messageQueue.EnqueueBack (
        {
            ._type = eMessageType::FrameComplete,
            ._params = nullptr,
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
