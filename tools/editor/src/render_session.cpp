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
        // FUCK
        std::unique_ptr<android_vulkan::android::MeshGeometry>      _fuckGeometry {};

        std::unique_ptr<android_vulkan::windows::MeshGeometry>      _geometry {};
        std::unique_ptr<HelloTriangleProgram>                       _program {};

    private:
        VkCommandPool                                               _commandPool = VK_NULL_HANDLE;
        VkFence                                                     _complete = VK_NULL_HANDLE;
        android_vulkan::Renderer                                    &_renderer;

    public:
        HelloTriangleJob () = delete;

        HelloTriangleJob ( HelloTriangleJob const & ) = delete;
        HelloTriangleJob &operator = ( HelloTriangleJob const & ) = delete;

        HelloTriangleJob ( HelloTriangleJob && ) = delete;
        HelloTriangleJob &operator = ( HelloTriangleJob && ) = delete;

        explicit HelloTriangleJob ( MessageQueue &messageQueue,
            android_vulkan::Renderer &renderer,
            std::mutex &submitMutex,
            VkRenderPass renderPass
        ) noexcept;

        ~HelloTriangleJob ();

    private:
        void CreateMesh ( std::mutex &submitMutex ) noexcept;
        void CreateProgram ( VkRenderPass renderPass ) noexcept;
};

HelloTriangleJob::HelloTriangleJob ( MessageQueue &messageQueue,
    android_vulkan::Renderer &renderer,
    std::mutex &submitMutex,
    VkRenderPass renderPass
) noexcept:
    _renderer ( renderer )
{
    std::thread (
        [ this, &messageQueue, &submitMutex, renderPass ] () noexcept
        {
            AV_THREAD_NAME ( "Hello triangle job" )
            CreateProgram ( renderPass );
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

        _geometry = std::make_unique<android_vulkan::windows::MeshGeometry> ();

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

        // FUCK
        _fuckGeometry = std::make_unique<android_vulkan::android::MeshGeometry> ();

        result = _fuckGeometry->LoadMesh ( _renderer,
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

            _fuckGeometry->FreeResources ( _renderer );
            _fuckGeometry.reset ();
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

        _fuckGeometry->FreeResources ( _renderer );
        _fuckGeometry.reset ();
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

        _fuckGeometry->FreeResources ( _renderer );
        _fuckGeometry.reset ();
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
        _fuckGeometry->FreeTransferResources ( _renderer );
        return;
    }

    _geometry->FreeResources ( _renderer );
    _geometry.reset ();

    _fuckGeometry->FreeResources ( _renderer );
    _fuckGeometry.reset ();
}

void HelloTriangleJob::CreateProgram ( VkRenderPass renderPass ) noexcept
{
    AV_TRACE ( "Program" )

    _program = std::make_unique<HelloTriangleProgram> ();

    if ( _program->Init ( _renderer, renderPass, 0U ) ) [[likely]]
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

pbr::FontStorage &RenderSession::GetFontStorage () noexcept
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

    for ( size_t i = 0U; i < pbr::DUAL_COMMAND_BUFFER; ++i )
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
        if ( commandInfo._pool != VK_NULL_HANDLE ) [[likely]]
        {
            vkDestroyCommandPool ( device, commandInfo._pool, nullptr );
            commandInfo._pool = VK_NULL_HANDLE;
        }

        if ( commandInfo._acquire != VK_NULL_HANDLE ) [[likely]]
        {
            vkDestroySemaphore ( device, commandInfo._acquire, nullptr );
            commandInfo._acquire = VK_NULL_HANDLE;
        }

        if ( commandInfo._fence == VK_NULL_HANDLE ) [[unlikely]]
            continue;

        vkDestroyFence ( device, commandInfo._fence, nullptr );
        commandInfo._fence = VK_NULL_HANDLE;
    }
}

bool RenderSession::CreateFramebuffer ( VkDevice device, VkExtent2D const &resolution ) noexcept
{
    VkFramebufferCreateInfo const info
    {
        .sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0U,
        .renderPass = _renderPassInfo.renderPass,
        .attachmentCount = 1U,
        .pAttachments = &_renderTarget.GetImageView (),
        .width = resolution.width,
        .height = resolution.height,
        .layers = 1U
    };

    bool const result = android_vulkan::Renderer::CheckVkResult (
        vkCreateFramebuffer ( device, &info, nullptr, &_renderPassInfo.framebuffer ),
        "editor::RenderSession::CreateFramebuffer",
        "Can't create framebuffer"
    );

    if ( !result ) [[unlikely]]
        return false;

    AV_SET_VULKAN_OBJECT_NAME ( device, _renderPassInfo.framebuffer, VK_OBJECT_TYPE_FRAMEBUFFER, "Render session" )
    return true;
}

bool RenderSession::CreateRenderPass ( VkDevice device ) noexcept
{
    VkAttachmentDescription const attachment
    {
        .flags = 0U,
        .format = RENDER_TARGET_FORMAT,
        .samples = VK_SAMPLE_COUNT_1_BIT,
        .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
        .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
        .stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
        .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
        .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
        .finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
    };

    constexpr static VkAttachmentReference colorReference
    {
        .attachment = 0U,
        .layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
    };

    constexpr VkSubpassDescription subpass
    {
        .flags = 0U,
        .pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS,
        .inputAttachmentCount = 0U,
        .pInputAttachments = nullptr,
        .colorAttachmentCount = 1U,
        .pColorAttachments = &colorReference,
        .pResolveAttachments = nullptr,
        .pDepthStencilAttachment = nullptr,
        .preserveAttachmentCount = 0U,
        .pPreserveAttachments = nullptr
    };

    constexpr VkSubpassDependency const dependencies[] =
    {
        {
            .srcSubpass = VK_SUBPASS_EXTERNAL,
            .dstSubpass = 0U,
            .srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
            .dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
            .srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
            .dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
            .dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT
        },
        {
            .srcSubpass = 0U,
            .dstSubpass = VK_SUBPASS_EXTERNAL,
            .srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,

            .dstStageMask = AV_VK_FLAG ( VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT ) |
                AV_VK_FLAG ( VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT ),

            .srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
            .dstAccessMask = AV_VK_FLAG ( VK_ACCESS_SHADER_READ_BIT ),
            .dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT
        }
    };

    VkRenderPassCreateInfo const renderPassInfo
    {
        .sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0U,
        .attachmentCount = 1U,
        .pAttachments = &attachment,
        .subpassCount = 1U,
        .pSubpasses = &subpass,
        .dependencyCount = static_cast<uint32_t> ( std::size ( dependencies ) ),
        .pDependencies = dependencies
    };

    bool const result = android_vulkan::Renderer::CheckVkResult (
        vkCreateRenderPass ( device, &renderPassInfo, nullptr, &_renderPassInfo.renderPass ),
        "editor::RenderSession::CreateRenderPass",
        "Can't create render pass"
    );

    if ( !result ) [[unlikely]]
        return false;

    AV_SET_VULKAN_OBJECT_NAME ( device, _renderPassInfo.renderPass, VK_OBJECT_TYPE_RENDER_PASS, "Render session" )

    constexpr static VkClearValue clearValue
    {
        .color
        {
            .float32 = { 0.5F, 0.5F, 0.5F, 1.0F }
        }
    };

    _renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    _renderPassInfo.pNext = nullptr;
    _renderPassInfo.clearValueCount = 1U;
    _renderPassInfo.pClearValues = &clearValue;

    _renderPassInfo.renderArea.offset =
    {
        .x = 0,
        .y = 0
    };

    return true;
}

bool RenderSession::CreateRenderTarget () noexcept
{
    VkExtent2D &resolution = _renderPassInfo.renderArea.extent;
    resolution = _renderer.GetSurfaceSize ();
    VkDevice device = _renderer.GetDevice ();

    if ( !CreateRenderTargetImage ( resolution ) || !CreateFramebuffer ( device, resolution ) ) [[unlikely]]
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

    AV_SET_VULKAN_OBJECT_NAME ( _renderer.GetDevice (),
        _renderTarget.GetImage (),
        VK_OBJECT_TYPE_IMAGE,
        "Render target"
    )

    AV_SET_VULKAN_OBJECT_NAME ( _renderer.GetDevice (),
        _renderTarget.GetImageView (),
        VK_OBJECT_TYPE_IMAGE_VIEW,
        "Render target"
    )

    return true;
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

    if ( !CreateRenderPass ( device ) ) [[unlikely]]
        return false;

    new HelloTriangleJob ( _messageQueue, renderer, _submitMutex, _renderPassInfo.renderPass );

    bool result = AllocateCommandBuffers ( device ) &&
        _presentRenderPass.OnInitDevice () &&
        _presentRenderPass.OnSwapchainCreated ( renderer );

    if ( !result ) [[unlikely]]
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

    result =
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

        result = _defaultTextureManager.Init ( renderer, pool ) &&
            _exposurePass.Init ( renderer, pool ) &&
            _resourceHeap.Init ( renderer, commandBuffer );

        if ( !result ) [[unlikely]]
        {
            return false;
        }
    }

    VkRenderPass renderPass = _presentRenderPass.GetRenderPass ();
    constexpr uint32_t subpass = pbr::PresentRenderPass::GetSubpass ();

    result = _samplerManager.Init ( device ) &&
        _uiPass.OnInitDevice ( renderer, _samplerManager, _defaultTextureManager.GetTransparent ()->GetImageView () ) &&

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
        _exposurePass.SetTarget ( renderer, _renderTarget ) &&
        _toneMapper.Init ( renderer ) &&

        _toneMapper.SetTarget ( renderer,
            renderPass,
            subpass,
            _renderTarget.GetImageView (),
            _exposurePass.GetExposure (),
            _samplerManager.GetClampToEdgeSampler ()
        ) &&

        _toneMapper.SetBrightness ( renderer, renderPass, subpass, DEFAULT_BRIGHTNESS_BALANCE ) &&
        _uiPass.OnSwapchainCreated ( renderer, renderPass, subpass ) &&
        _uiPass.SetBrightness ( renderer, renderPass, subpass, DEFAULT_BRIGHTNESS_BALANCE ) &&

        android_vulkan::Renderer::CheckVkResult ( vkQueueWaitIdle ( queue ),
            "editor::RenderSession::InitModules",
            "Can't wait queue idle"
        );

    if ( !result ) [[unlikely]]
        return false;

    vkFreeCommandBuffers ( device, pool, 1U, &commandBuffer );
    _exposurePass.FreeTransferResources ( device, pool );
    _defaultTextureManager.FreeTransferResources ( renderer, pool );
    _timestamp = std::chrono::system_clock::now ();
    return true;
}

void RenderSession::OnHelloTriangleReady ( void* params ) noexcept
{
    _messageQueue.DequeueEnd ();
    auto* job = static_cast<HelloTriangleJob*> ( params );
    _helloTriangleProgram = std::move ( job->_program );
    _fuckHelloTriangleGeometry = std::move ( job->_fuckGeometry );
    _helloTriangleGeometry = std::move ( job->_geometry );
    delete job;
}

void RenderSession::OnRenderFrame () noexcept
{
    AV_TRACE ( "Render frame" )
    _messageQueue.DequeueEnd ();

    if ( _broken ) [[unlikely]]
        return;

    Timestamp const now = std::chrono::system_clock::now ();
    std::chrono::duration<float> const seconds = now - _timestamp;
    float const deltaTime = seconds.count ();
    _timestamp = now;

    size_t const commandBufferIndex = _writingCommandInfo;
    CommandInfo &commandInfo = _commandInfo[ _writingCommandInfo ];
    _writingCommandInfo = ++_writingCommandInfo % pbr::DUAL_COMMAND_BUFFER;

    android_vulkan::Renderer &renderer = _renderer;
    _uiManager.RenderUI ( renderer, _uiPass );

    VkDevice device = renderer.GetDevice ();

    if ( !PrepareCommandBuffer ( device, commandInfo ) ) [[unlikely]]
    {
        // FUCK
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

        // FUCK
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
        // FUCK
        AV_ASSERT ( false )
        return;
    }

    {
        AV_VULKAN_GROUP ( commandBuffer, "Upload" )

        if ( !_uiPass.UploadGPUData ( renderer, commandBuffer, commandBufferIndex ) ) [[unlikely]]
        {
            // FUCK
            AV_ASSERT ( false )
            return;
        }

        _toneMapper.UploadGPUData ( renderer, commandBuffer );
    }

    {
        AV_VULKAN_GROUP ( commandBuffer, "Scene" )
        vkCmdBeginRenderPass ( commandBuffer, &_renderPassInfo, VK_SUBPASS_CONTENTS_INLINE );

        vkCmdSetViewport ( commandBuffer, 0U, 1U, &_viewport );
        vkCmdSetScissor ( commandBuffer, 0U, 1U, &_renderPassInfo.renderArea );
        result = static_cast<bool> ( _fuckHelloTriangleGeometry ) & static_cast<bool> ( _fuckHelloTriangleGeometry );

        if ( result ) [[likely]]
        {
            constexpr VkDeviceSize const offsets = 0U;

            vkCmdBindVertexBuffers ( commandBuffer,
                0U,
                1U,
                &_fuckHelloTriangleGeometry->GetMeshBufferInfo ()._buffer,
                &offsets
            );

            _helloTriangleProgram->Bind ( commandBuffer );
            vkCmdDraw ( commandBuffer, _fuckHelloTriangleGeometry->GetVertexCount (), 1U, 0U, 0U );
        }

        vkCmdEndRenderPass ( commandBuffer );
    }

    _exposurePass.Execute ( commandBuffer, deltaTime );

    {
        AV_VULKAN_GROUP ( commandBuffer, "Present" )
        _presentRenderPass.Begin ( commandBuffer );

        _toneMapper.Execute ( commandBuffer );

        if ( !_uiPass.Execute ( commandBuffer, commandBufferIndex ) ) [[unlikely]]
        {
            // FUCK
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
        // FUCK
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

            // FUCK
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

    if ( _renderPassInfo.framebuffer != VK_NULL_HANDLE ) [[likely]]
        vkDestroyFramebuffer ( device, std::exchange ( _renderPassInfo.framebuffer, VK_NULL_HANDLE ), nullptr );

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

    if ( _fuckHelloTriangleGeometry ) [[likely]]
    {
        _fuckHelloTriangleGeometry->FreeResources ( renderer );
        _fuckHelloTriangleGeometry.reset ();
    }

    _renderTarget.FreeResources ( renderer );

    if ( _renderPassInfo.renderPass != VK_NULL_HANDLE ) [[likely]]
        vkDestroyRenderPass ( device, std::exchange ( _renderPassInfo.renderPass, VK_NULL_HANDLE ), nullptr );

    _uiPass.OnSwapchainDestroyed ();
    _uiPass.OnDestroyDevice ( renderer );

    _presentRenderPass.OnSwapchainDestroyed ( device );
    _presentRenderPass.OnDestroyDevice ( device );

    _exposurePass.Destroy ( renderer );
    _toneMapper.Destroy ( renderer );

    _defaultTextureManager.Destroy ( renderer );
    _samplerManager.Destroy ( device );

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
    VkDevice device = renderer.GetDevice ();
    _presentRenderPass.OnSwapchainDestroyed ( device );

    if ( !_presentRenderPass.OnSwapchainCreated ( renderer ) ) [[unlikely]]
    {
        // FUCK
        AV_ASSERT ( false )
    }

    VkExtent2D &resolution = _renderPassInfo.renderArea.extent;
    resolution = renderer.GetSurfaceSize ();

    _viewport =
    {
        .x = 0.0F,
        .y = 0.0F,
        .width = static_cast<float> ( resolution.width ),
        .height = static_cast<float> ( resolution.height ),
        .minDepth = 0.0F,
        .maxDepth = 1.0F
    };

    vkDestroyFramebuffer ( device, _renderPassInfo.framebuffer, nullptr );
    _renderPassInfo.framebuffer = VK_NULL_HANDLE;

    _renderTarget.FreeResources ( renderer );

    if ( !CreateRenderTargetImage ( resolution ) || !CreateFramebuffer ( device, resolution ) ) [[unlikely]]
    {
        // FUCK
        AV_ASSERT ( false )
        return;
    }

    _uiPass.OnSwapchainDestroyed ();
    VkRenderPass renderPass = _presentRenderPass.GetRenderPass ();
    constexpr uint32_t subpass = pbr::PresentRenderPass::GetSubpass ();

    bool const result = _exposurePass.SetTarget ( renderer, _renderTarget ) &&
        _uiPass.OnSwapchainCreated ( renderer, renderPass, subpass ) &&

        _toneMapper.SetTarget ( renderer,
            renderPass,
            subpass,
            _renderTarget.GetImageView (),
            _exposurePass.GetExposure (),
            _samplerManager.GetClampToEdgeSampler ()
        );

    if ( result ) [[likely]]
        return;

    // FUCK
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

void RenderSession::OnUIElementCreated () noexcept
{
    AV_TRACE ( "UI element created" )
    _messageQueue.DequeueEnd ();
    ++_uiElements;
}

void RenderSession::OnUIDeleteElement ( Message &&message ) noexcept
{
    AV_TRACE ( "UI delete element" )
    _messageQueue.DequeueEnd ();
    delete static_cast<pbr::UIElement*> ( message._params );
    --_uiElements;
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
