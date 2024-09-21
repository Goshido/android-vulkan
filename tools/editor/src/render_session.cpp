#include <av_assert.hpp>
#include <blit_program.inc>
#include <hello_triangle_vertex.hpp>
#include <render_session.hpp>
#include <logger.hpp>
#include <trace.hpp>
#include <vulkan_utils.hpp>


namespace editor {

namespace {

constexpr VkFormat RENDER_TARGET_FORMAT = VK_FORMAT_R8G8B8A8_UNORM;
constexpr float DEFAULT_BRIGHTNESS_BALANCE = 0.0F;

} // end of anonymous namespace

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
        [ & ] () noexcept
        {
            AV_THREAD_NAME ( "Hello triangle job" )
            CreateProgram ( renderPass );
            CreateMesh ( submitMutex );

            messageQueue.EnqueueBack (
                Message
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

    result = _geometry->LoadMesh ( reinterpret_cast<uint8_t const*> ( data ),
        sizeof ( data ),
        static_cast<uint32_t> ( std::size ( data ) ),
        _renderer,
        commandBuffer,
        true,
        VK_NULL_HANDLE
    );

    if ( !result ) [[unlikely]]
    {
        _geometry->FreeResources ( _renderer );
        _geometry.reset ();
        return;
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

void HelloTriangleJob::CreateProgram ( VkRenderPass renderPass ) noexcept
{
    AV_TRACE ( "Program" )

    _program = std::make_unique<HelloTriangleProgram> ();

    if ( _program->Init ( _renderer, renderPass, 0U ) ) [[likely]]
        return;

    _program->Destroy ( _renderer.GetDevice () );
    _program.reset ();
}

//----------------------------------------------------------------------------------------------------------------------

bool RenderSession::Init ( MessageQueue &messageQueue, android_vulkan::Renderer &renderer ) noexcept
{
    AV_TRACE ( "RenderSession: init" )

    _messageQueue = &messageQueue;
    _renderer = &renderer;

    _thread = std::thread (
        [ this ]() noexcept
        {
            AV_THREAD_NAME ( "Render session" )
            EventLoop ();
        }
    );

    return true;
}

void RenderSession::Destroy () noexcept
{
    AV_TRACE ( "RenderSession: destroy" )

    if ( _thread.joinable () ) [[likely]]
        _thread.join ();

    _messageQueue = nullptr;
    _renderer = nullptr;
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
        .queueFamilyIndex = _renderer->GetQueueFamilyIndex ()
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
            .dstStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
            .srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
            .dstAccessMask = VK_ACCESS_SHADER_READ_BIT,
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
            .float32 = { 0.0F, 0.0F, 0.0F, 1.0F }
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
    resolution = _renderer->GetSurfaceSize ();
    VkDevice device = _renderer->GetDevice ();

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

    constexpr static VkDescriptorPoolSize const poolSizes[] =
    {
        {
            .type = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,
            .descriptorCount = 1U
        },
        {
            .type = VK_DESCRIPTOR_TYPE_SAMPLER,
            .descriptorCount = 1U
        }
    };

    constexpr VkDescriptorPoolCreateInfo poolInfo
    {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0U,
        .maxSets = 1U,
        .poolSizeCount = static_cast<uint32_t> ( std::size ( poolSizes ) ),
        .pPoolSizes = poolSizes
    };

    bool result = android_vulkan::Renderer::CheckVkResult (
        vkCreateDescriptorPool ( device, &poolInfo, nullptr, &_descriptorPool ),
        "editor::RenderSession::CreateRenderTarget",
        "Can't create descriptor pool"
    );

    if ( !result || !_blitLayout.Init ( device ) ) [[unlikely]]
        return false;

    AV_SET_VULKAN_OBJECT_NAME ( device, _descriptorPool, VK_OBJECT_TYPE_DESCRIPTOR_POOL, "Render session" )

    VkDescriptorSetAllocateInfo const descriptorSetAllocateInfo
    {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
        .pNext = nullptr,
        .descriptorPool = _descriptorPool,
        .descriptorSetCount = 1U,
        .pSetLayouts = &_blitLayout.GetLayout ()
    };

    result = android_vulkan::Renderer::CheckVkResult (
        vkAllocateDescriptorSets ( device, &descriptorSetAllocateInfo, &_descriptorSet ),
        "editor::RenderSession::CreateRenderTarget",
        "Can't allocate descriptor set"
    );

    if ( !result ) [[unlikely]]
        return false;

    AV_SET_VULKAN_OBJECT_NAME ( device, _descriptorSet, VK_OBJECT_TYPE_DESCRIPTOR_SET, "Render session" )

    constexpr VkSamplerCreateInfo samplerInfo
    {
        .sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0U,
        .magFilter = VK_FILTER_NEAREST,
        .minFilter = VK_FILTER_NEAREST,
        .mipmapMode = VK_SAMPLER_MIPMAP_MODE_NEAREST,
        .addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
        .addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
        .addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
        .mipLodBias = 0.0F,
        .anisotropyEnable = VK_FALSE,
        .maxAnisotropy = 1.0F,
        .compareEnable = VK_FALSE,
        .compareOp = VK_COMPARE_OP_ALWAYS,
        .minLod = 0.0F,
        .maxLod = 0.0F,
        .borderColor = VK_BORDER_COLOR_FLOAT_TRANSPARENT_BLACK,
        .unnormalizedCoordinates = VK_FALSE
    };

    if ( !_sampler.Init ( device, samplerInfo, "Point sampler" ) ) [[unlikely]]
        return false;

    VkDescriptorImageInfo const imageInfo
    {
        .sampler = _sampler.GetSampler (),
        .imageView = _renderTarget.GetImageView (),
        .imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
    };

    VkWriteDescriptorSet const writes[] =
    {
        {
            .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
            .pNext = nullptr,
            .dstSet = _descriptorSet,
            .dstBinding = BIND_IMAGE,
            .dstArrayElement = 0U,
            .descriptorCount = 1U,
            .descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,
            .pImageInfo = &imageInfo,
            .pBufferInfo = nullptr,
            .pTexelBufferView = nullptr
        },
        {
            .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
            .pNext = nullptr,
            .dstSet = _descriptorSet,
            .dstBinding = BIND_SAMPLER,
            .dstArrayElement = 0U,
            .descriptorCount = 1U,
            .descriptorType = VK_DESCRIPTOR_TYPE_SAMPLER,
            .pImageInfo = &imageInfo,
            .pBufferInfo = nullptr,
            .pTexelBufferView = nullptr
        }
    };

    vkUpdateDescriptorSets ( device, static_cast<uint32_t> ( std::size ( writes ) ), writes, 0U, nullptr );
    return true;
}

bool RenderSession::CreateRenderTargetImage ( VkExtent2D const &resolution ) noexcept
{
    bool const result = _renderTarget.CreateRenderTarget ( resolution,
        RENDER_TARGET_FORMAT,
        AV_VK_FLAG ( VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT ) | AV_VK_FLAG ( VK_IMAGE_USAGE_SAMPLED_BIT ),
        *_renderer
    );

    if ( !result ) [[unlikely]]
        return false;

    AV_SET_VULKAN_OBJECT_NAME ( _renderer->GetDevice (),
        _renderTarget.GetImage (),
        VK_OBJECT_TYPE_IMAGE,
        "Render target"
    )

    AV_SET_VULKAN_OBJECT_NAME ( _renderer->GetDevice (),
        _renderTarget.GetImageView (),
        VK_OBJECT_TYPE_IMAGE_VIEW,
        "Render target"
    )

    return true;
}

void RenderSession::EventLoop () noexcept
{
    if ( !InitiModules () ) [[unlikely]]
        _broken = true;

    MessageQueue &messageQueue = *_messageQueue;

    if ( _broken )
    {
        messageQueue.EnqueueBack (
            Message
            {
                ._type = eMessageType::CloseEditor,
                ._params = nullptr,
                ._serialNumber = 0U
            }
        );
    }

    std::optional<uint32_t> knownSerialNumber {};

    for ( ; ; )
    {
        AV_TRACE ( "Event loop" )
        Message message = messageQueue.DequeueBegin ( knownSerialNumber );

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

            default:
                knownSerialNumber = message._serialNumber;
                messageQueue.DequeueEnd ( std::move ( message ) );
            break;
        }

        GX_ENABLE_WARNING ( 4061 )
    }
}

bool RenderSession::InitiModules () noexcept
{
    AV_TRACE ( "Init modules" )
    VkDevice device = _renderer->GetDevice ();

    if ( !CreateRenderPass ( device ) ) [[unlikely]]
        return false;

    new HelloTriangleJob ( *_messageQueue, *_renderer, _submitMutex, _renderPassInfo.renderPass );

    return AllocateCommandBuffers ( device ) &&
        _presentRenderPass.OnInitDevice () &&
        _presentRenderPass.OnSwapchainCreated ( *_renderer ) &&

        _blitProgram.Init ( *_renderer,
            _presentRenderPass.GetRenderPass (),
            _presentRenderPass.GetSubpass (),
            pbr::SRGBProgram::GetGammaInfo ( DEFAULT_BRIGHTNESS_BALANCE )
        ) &&

        CreateRenderTarget ();
}

void RenderSession::OnHelloTriangleReady ( void* params ) noexcept
{
    _messageQueue->DequeueEnd ();
    auto* job = static_cast<HelloTriangleJob*> ( params );
    _helloTriangleProgram = std::move ( job->_program );
    _helloTriangleGeometry = std::move ( job->_geometry );
    delete job;
}

void RenderSession::OnRenderFrame () noexcept
{
    AV_TRACE ( "Render frame" )
    _messageQueue->DequeueEnd ();

    if ( _broken ) [[unlikely]]
        return;

    CommandInfo &commandInfo = _commandInfo[ _writingCommandInfo ];
    _writingCommandInfo = ++_writingCommandInfo % pbr::DUAL_COMMAND_BUFFER;

    android_vulkan::Renderer &renderer = *_renderer;
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
        AV_VULKAN_GROUP ( commandBuffer, "Scene" )
        vkCmdBeginRenderPass ( commandBuffer, &_renderPassInfo, VK_SUBPASS_CONTENTS_INLINE );

        vkCmdSetViewport ( commandBuffer, 0U, 1U, &_viewport );
        vkCmdSetScissor ( commandBuffer, 0U, 1U, &_renderPassInfo.renderArea );

        if ( static_cast<bool> ( _helloTriangleGeometry ) & static_cast<bool> ( _helloTriangleGeometry ) ) [[likely]]
        {
            constexpr VkDeviceSize offsets = 0U;
            vkCmdBindVertexBuffers ( commandBuffer, 0U, 1U, &_helloTriangleGeometry->GetVertexBuffer (), &offsets );

            _helloTriangleProgram->Bind ( commandBuffer );
            vkCmdDraw ( commandBuffer, _helloTriangleGeometry->GetVertexCount (), 1U, 0U, 0U );
        }

        vkCmdEndRenderPass ( commandBuffer );
    }

    {
        AV_VULKAN_GROUP ( commandBuffer, "Present" )
        _presentRenderPass.Begin ( commandBuffer );

        _blitProgram.Bind ( commandBuffer );
        _blitProgram.SetDescriptorSet ( commandBuffer, _descriptorSet );

        vkCmdDraw ( commandBuffer, 3U, 1U, 0U, 0U );

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
    }

    _messageQueue->EnqueueBack (
        Message
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
    _messageQueue->DequeueEnd ( std::move ( refund ) );

    bool const result = android_vulkan::Renderer::CheckVkResult ( vkQueueWaitIdle ( _renderer->GetQueue () ),
        "editor::RenderSession::OnShutdown",
        "Can't wait queue idle"
    );

    if ( !result ) [[unlikely]]
        android_vulkan::LogError ( "Render session error. Can't stop." );

    VkDevice device = _renderer->GetDevice ();
    FreeCommandBuffers ( device );

    if ( _renderPassInfo.framebuffer != VK_NULL_HANDLE ) [[likely]]
    {
        vkDestroyFramebuffer ( device, _renderPassInfo.framebuffer, nullptr );
        _renderPassInfo.framebuffer = VK_NULL_HANDLE;
    }

    if ( _descriptorPool != VK_NULL_HANDLE ) [[likely]]
    {
        vkDestroyDescriptorPool ( device, _descriptorPool, nullptr );
        _descriptorPool = VK_NULL_HANDLE;
    }

    _sampler.Destroy ( device );

    if ( _helloTriangleProgram ) [[likely]]
    {
        _helloTriangleProgram->Destroy ( device );
        _helloTriangleProgram.reset ();
    }

    if ( _helloTriangleGeometry ) [[likely]]
    {
        _helloTriangleGeometry->FreeResources ( *_renderer );
        _helloTriangleGeometry.reset ();
    }

    _blitLayout.Destroy ( device );
    _blitProgram.Destroy ( device );

    _renderTarget.FreeResources ( *_renderer );

    if ( _renderPassInfo.renderPass != VK_NULL_HANDLE ) [[likely]]
    {
        vkDestroyRenderPass ( device, _renderPassInfo.renderPass, VK_NULL_HANDLE );
        _renderPassInfo.renderPass = VK_NULL_HANDLE;
    }

    _presentRenderPass.OnSwapchainDestroyed ( device );
    _presentRenderPass.OnDestroyDevice ( device );

    _messageQueue->EnqueueFront (
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
    _messageQueue->DequeueEnd ();

    android_vulkan::Renderer &renderer = *_renderer;
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

    VkDescriptorImageInfo const imageInfo
    {
        .sampler = VK_NULL_HANDLE,
        .imageView = _renderTarget.GetImageView (),
        .imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
    };

    VkWriteDescriptorSet const write
    {
        .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
        .pNext = nullptr,
        .dstSet = _descriptorSet,
        .dstBinding = BIND_IMAGE,
        .dstArrayElement = 0U,
        .descriptorCount = 1U,
        .descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,
        .pImageInfo = &imageInfo,
        .pBufferInfo = nullptr,
        .pTexelBufferView = nullptr
    };

    vkUpdateDescriptorSets ( device, 1U, &write, 0U, nullptr );
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

void RenderSession::NotifyRecreateSwapchain () const noexcept
{
    _messageQueue->EnqueueBack (
        Message
        {
            ._type = eMessageType::RecreateSwapchain,
            ._params = nullptr,
            ._serialNumber = 0U
        }
    );

    _messageQueue->EnqueueBack (
        Message
        {
            ._type = eMessageType::FrameComplete,
            ._params = nullptr,
            ._serialNumber = 0U
        }
    );
}

} // namespace editor
