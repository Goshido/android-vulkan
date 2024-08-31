#include <render_session.hpp>
#include <logger.hpp>
#include <trace.hpp>
#include <vulkan_utils.hpp>


namespace editor {

bool RenderSession::Init ( MessageQueue &messageQueue, android_vulkan::Renderer &renderer ) noexcept
{
    AV_TRACE ( "RenderSession: init" )

    _messageQueue = &messageQueue;
    _renderer = &renderer;

    _thread = std::thread (
        [ this ]() noexcept
        {
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

bool RenderSession::AllocateCommandBuffers () noexcept
{
    constexpr VkFenceCreateInfo fenceInfo
    {
        .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
        .pNext = nullptr,
        .flags = VK_FENCE_CREATE_SIGNALED_BIT
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

    VkDevice device = _renderer->GetDevice ();

    for ( size_t i = 0U; i < pbr::DUAL_COMMAND_BUFFER; ++i )
    {
        CommandInfo &info = _commandInfo[ i ];

        bool result = android_vulkan::Renderer::CheckVkResult ( vkCreateFence ( device, &fenceInfo, nullptr, &info._fence ),
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

void RenderSession::EventLoop () noexcept
{
    AV_THREAD_NAME ( "Render session" )

    if ( !InitiModules () ) [[unlikely]]
        _broken = true;

    MessageQueue &messageQueue = *_messageQueue;

    for ( ; ; )
    {
        AV_TRACE ( "Render session" )
        Message message = messageQueue.Dequeue ();

        GX_DISABLE_WARNING ( 4061 )

        switch ( message._type )
        {
            case eMessageType::Shutdown:
                Shutdown ();
            return;

            case eMessageType::RenderFrame:
                RenderFrame ();
            break;

            default:
                messageQueue.EnqueueFront ( std::move ( message ) );
            break;
        }

        GX_ENABLE_WARNING ( 4061 )
    }
}

bool RenderSession::InitiModules () noexcept
{
    AV_TRACE ( "RenderSession: init modules" )

    bool const result = AllocateCommandBuffers () &&
        _presentRenderPass.OnInitDevice () &&
        _presentRenderPass.OnSwapchainCreated ( *_renderer );

    if ( result ) [[unlikely]]
        return true;

    _messageQueue->EnqueueBack (
        Message
        {
            ._type = eMessageType::CloseEditor,
            ._params = nullptr
        }
    );

    return false;
}

void RenderSession::RenderFrame () noexcept
{
    AV_TRACE ( "RenderSession: frame" )

    if ( _broken ) [[unlikely]]
        return;

    static size_t i = 0U;
    android_vulkan::LogDebug ( "RenderFrame #%zu", i++ );
}

void RenderSession::Shutdown () noexcept
{
    AV_TRACE ( "RenderSession: shutdown" )

    VkDevice device = _renderer->GetDevice ();
    FreeCommandBuffers ( device );
    _presentRenderPass.OnSwapchainDestroyed ( device );
    _presentRenderPass.OnDestroyDevice ( device );
}

} // namespace editor
