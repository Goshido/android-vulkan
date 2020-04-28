#include <rotating_mesh/game.h>

AV_DISABLE_COMMON_WARNINGS

#include <cassert>

AV_RESTORE_WARNING_STATE

#include <vulkan_utils.h>


namespace rotating_mesh {

Game::Game ():
    _commandPool ( VK_NULL_HANDLE ),
//    _constantBuffer ( VK_NULL_HANDLE ),
//    _constantBufferDeviceMemory ( VK_NULL_HANDLE ),
//    _descriptorPool ( VK_NULL_HANDLE ),
//    _diffuseTexture ( VK_NULL_HANDLE ),
//    _diffuseView ( VK_NULL_HANDLE ),
//    _diffuseDeviceMemory ( VK_NULL_HANDLE ),
//    _mesh ( VK_NULL_HANDLE ),
//    _meshDeviceMemory ( VK_NULL_HANDLE ),
//    _pipeline ( VK_NULL_HANDLE ),
//    _pipelineLayout ( VK_NULL_HANDLE ),
    _presentationImageFence ( VK_NULL_HANDLE ),
//    _renderPass ( VK_NULL_HANDLE ),
    _renderPassEndSemaphore ( VK_NULL_HANDLE )
//    _sampler ( VK_NULL_HANDLE ),
//    _vertexShaderModule ( VK_NULL_HANDLE ),
//    _fragmentShaderModule ( VK_NULL_HANDLE )
{
    // NOTHING
}

bool Game::IsReady ()
{
    assert ( !"Game::IsReady - Implement me!" );
    return false;
}

bool Game::OnInit ( android_vulkan::Renderer &renderer )
{
    assert ( !"Game::OnInit - Implement me!" );

    if ( !CreateSyncPrimitives ( renderer ) )
    {
        OnDestroy ( renderer );
        return false;
    }

    if ( !CreateCommandPool ( renderer ) )
    {
        OnDestroy ( renderer );
        return false;
    }

    return false;
}

bool Game::OnFrame ( android_vulkan::Renderer &renderer, double /*deltaTime*/ )
{
    uint32_t presentationImageIndex = UINT32_MAX;

    if ( !BeginFrame ( presentationImageIndex, renderer ) )
        return false;

    assert ( !"Game::OnFrame - Implement me!" );

    return EndFrame ( presentationImageIndex, renderer );
}

bool Game::OnDestroy ( android_vulkan::Renderer &renderer )
{
    assert ( !"Game::OnDestroy - Implement me!" );

    DestroyCommandPool ( renderer );
    DestroySyncPrimitives ( renderer );

    return false;
}

bool Game::BeginFrame ( uint32_t &presentationImageIndex, android_vulkan::Renderer &renderer )
{
    const VkDevice device = renderer.GetDevice ();

    bool result = renderer.CheckVkResult (
        vkAcquireNextImageKHR ( device,
            renderer.GetSwapchain (),
            UINT64_MAX,
            VK_NULL_HANDLE,
            _presentationImageFence,
            &presentationImageIndex
        ),

        "Game::BeginFrame",
        "Can't get presentation image index"
    );

    if ( !result )
        return false;

    result = renderer.CheckVkResult ( vkWaitForFences ( device, 1U, &_presentationImageFence, VK_TRUE, UINT64_MAX ),
        "Game::BeginFrame",
        "Can't wain for presentation image index fence"
    );

    if ( !result )
        return false;

    return renderer.CheckVkResult ( vkResetFences ( device, 1U, &_presentationImageFence ),
        "Game::BeginFrame",
        "Can't resent presentation image index fence"
    );
}

bool Game::EndFrame ( uint32_t presentationImageIndex, android_vulkan::Renderer &renderer )
{
    VkResult presentResult;

    VkPresentInfoKHR presentInfo;
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    presentInfo.pNext = nullptr;
    presentInfo.waitSemaphoreCount = 1U;
    presentInfo.pWaitSemaphores = &_renderPassEndSemaphore;
    presentInfo.pResults = &presentResult;
    presentInfo.swapchainCount = 1U;
    presentInfo.pSwapchains = &renderer.GetSwapchain ();
    presentInfo.pImageIndices = &presentationImageIndex;

    const VkResult mainResult = vkQueuePresentKHR ( renderer.GetQueue (), &presentInfo );

    // Note vkQueuePresentKHR may return VK_SUBOPTIMAL_KHR right before application is minimized.
    if ( mainResult == VK_SUBOPTIMAL_KHR )
        return true;

    if ( !renderer.CheckVkResult ( mainResult, "Game::EndFrame", "Can't present frame" ) )
        return false;

    return renderer.CheckVkResult ( presentResult, "Game::EndFrame", "Present queue has been failed" );
}

bool Game::CreateCommandPool ( android_vulkan::Renderer &renderer )
{
    VkCommandPoolCreateInfo poolInfo;
    poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    poolInfo.pNext = nullptr;
    poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    poolInfo.queueFamilyIndex = renderer.GetQueueFamilyIndex ();

    const bool result = renderer.CheckVkResult (
        vkCreateCommandPool ( renderer.GetDevice (), &poolInfo, nullptr, &_commandPool ),
        "Game::CreateCommandPool",
        "Can't create command pool"
    );

    if ( !result )
        return false;

    AV_REGISTER_COMMAND_POOL ( "Game::_commandPool" )
    return true;
}

void Game::DestroyCommandPool ( android_vulkan::Renderer &renderer )
{
    if ( _commandPool == VK_NULL_HANDLE )
        return;

    vkDestroyCommandPool ( renderer.GetDevice (), _commandPool, nullptr );
    _commandPool = VK_NULL_HANDLE;
    AV_UNREGISTER_COMMAND_POOL ( "Game::_commandPool" )
}

bool Game::CreateConstantBuffer ( android_vulkan::Renderer& /*renderer*/ )
{
    assert ( !"Game::CreateConstantBuffer - Implement me!" );
    return false;
}

void Game::DestroyConstantBuffer ( android_vulkan::Renderer& /*renderer*/ )
{
    assert ( !"Game::DestroyConstantBuffer - Implement me!" );
}

bool Game::CreateMesh ( android_vulkan::Renderer& /*renderer*/ )
{
    assert ( !"Game::CreateMesh - Implement me!" );
    return false;
}

void Game::DestroyMesh ( android_vulkan::Renderer& /*renderer*/ )
{
    assert ( !"Game::DestroyMesh - Implement me!" );
}

bool Game::CreatePipeline ( android_vulkan::Renderer& /*renderer*/ )
{
    assert ( !"Game::CreatePipeline - Implement me!" );
    return false;
}

void Game::DestroyPipeline ( android_vulkan::Renderer& /*renderer*/ )
{
    assert ( !"Game::DestroyPipeline - Implement me!" );
}

bool Game::CreateRenderPass ( android_vulkan::Renderer& /*renderer*/ )
{
    assert ( !"Game::CreateRenderPass - Implement me!" );
    return false;
}

void Game::DestroyRenderPass ( android_vulkan::Renderer& /*renderer*/ )
{
    assert ( !"Game::DestroyRenderPass - Implement me!" );
}

bool Game::CreateSyncPrimitives ( android_vulkan::Renderer &renderer )
{
    const VkDevice device = renderer.GetDevice ();

    VkFenceCreateInfo fenceInfo;
    fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceInfo.pNext = nullptr;
    fenceInfo.flags = 0U;

    bool result = renderer.CheckVkResult ( vkCreateFence ( device, &fenceInfo, nullptr, &_presentationImageFence ),
        "Game::CreateSyncPrimitives",
        "Can't create presentation fence"
    );

    if ( !result )
        return false;

    AV_REGISTER_FENCE ( "Game::_presentationImageFence" )

    VkSemaphoreCreateInfo semaphoreInfo;
    semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
    semaphoreInfo.pNext = nullptr;
    semaphoreInfo.flags = 0U;

    result = renderer.CheckVkResult ( vkCreateSemaphore ( device, &semaphoreInfo, nullptr, &_renderPassEndSemaphore ),
        "Game::CreateSyncPrimitives",
        "Can't create render pass end semaphore"
    );

    if ( !result )
        return false;

    AV_REGISTER_SEMAPHORE ( "Game::_renderPassEndSemaphore" )
    return true;
}

void Game::DestroySyncPrimitives ( android_vulkan::Renderer &renderer )
{
    assert ( !"Game::DestroySyncPrimitives - Implement me!" );

    const VkDevice device = renderer.GetDevice ();

    if ( _renderPassEndSemaphore != VK_NULL_HANDLE )
    {
        vkDestroySemaphore ( device, _renderPassEndSemaphore, nullptr );
        _renderPassEndSemaphore = VK_NULL_HANDLE;
        AV_UNREGISTER_SEMAPHORE ( "Game::_renderPassEndSemaphore" )
    }

    if ( _presentationImageFence == VK_NULL_HANDLE )
        return;

    vkDestroyFence ( device, _presentationImageFence, nullptr );
    _presentationImageFence = VK_NULL_HANDLE;
    AV_UNREGISTER_FENCE ( "Game::_presentationImageFence" )
}

bool Game::CreateTexture ( android_vulkan::Renderer& /*renderer*/ )
{
    assert ( !"Game::CreateTexture - Implement me!" );
    return false;
}

void Game::DestroyTexture ( android_vulkan::Renderer& /*renderer*/ )
{
    assert ( !"Game::DestroyTexture - Implement me!" );
}

bool Game::InitCommandBuffers ( android_vulkan::Renderer& /*renderer*/ )
{
    assert ( !"Game::InitCommandBuffers - Implement me!" );
    return false;
}

} // namespace rotating_mesh
