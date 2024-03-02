#include <rotating_mesh/game.hpp>

GX_DISABLE_COMMON_WARNINGS

#include <array>
#include <cmath>
#include <thread>

GX_RESTORE_WARNING_STATE

#include <vertex_info.hpp>
#include <vulkan_utils.hpp>


namespace rotating_mesh {

namespace {

constexpr char const* VERTEX_SHADER = "shaders/static_mesh.vs.spv";
constexpr char const* VERTEX_SHADER_ENTRY_POINT = "VS";

constexpr char const* FRAGMENT_SHADER_ENTRY_POINT = "PS";

constexpr std::string_view MATERIAL_1_DIFFUSE = "textures/rotating_mesh/sonic-material-1-diffuse.ktx";
constexpr char const* MATERIAL_1_MESH = "meshes/rotating_mesh/sonic-material-1.mesh";

constexpr std::string_view MATERIAL_2_DIFFUSE = "textures/rotating_mesh/sonic-material-2-diffuse.ktx";
constexpr char const* MATERIAL_2_MESH = "meshes/rotating_mesh/sonic-material-2.mesh";
constexpr std::string_view MATERIAL_2_NORMAL = "textures/rotating_mesh/sonic-material-2-normal.png";

constexpr std::string_view MATERIAL_3_DIFFUSE = "textures/rotating_mesh/sonic-material-3-diffuse.ktx";
constexpr char const* MATERIAL_3_MESH = "meshes/rotating_mesh/sonic-material-3.mesh";
constexpr std::string_view MATERIAL_3_NORMAL = "textures/rotating_mesh/sonic-material-3-normal.png";

constexpr float ROTATION_SPEED = GX_MATH_HALF_PI;
constexpr float FIELD_OF_VIEW = 60.0F;
constexpr float Z_NEAR = 0.1F;
constexpr float Z_FAR = 1.0e+3F;

} // end of anonymous namespace

//----------------------------------------------------------------------------------------------------------------------

Game::Game ( char const* fragmentShader ) noexcept:
    _fragmentShader ( fragmentShader )
{
    // NOTHING
}

bool Game::CreateSamplers ( VkDevice device ) noexcept
{
    VkSamplerCreateInfo samplerInfo
    {
        .sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0U,
        .magFilter = VK_FILTER_LINEAR,
        .minFilter = VK_FILTER_LINEAR,
        .mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR,
        .addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT,
        .addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT,
        .addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT,
        .mipLodBias = 0.0F,
        .anisotropyEnable = VK_FALSE,
        .maxAnisotropy = 1.0F,
        .compareEnable = VK_FALSE,
        .compareOp = VK_COMPARE_OP_ALWAYS,
        .minLod = 0.0F,
        .maxLod = VK_LOD_CLAMP_NONE,
        .borderColor = VK_BORDER_COLOR_FLOAT_TRANSPARENT_BLACK,
        .unnormalizedCoordinates = VK_FALSE
    };

    bool result = android_vulkan::Renderer::CheckVkResult (
        vkCreateSampler ( device, &samplerInfo, nullptr, &_sampler ),
        "Game::CreateSamplers",
        "Can't create sampler"
    );

    if ( !result )
        return false;

    AV_SET_VULKAN_OBJECT_NAME ( device, _sampler, VK_OBJECT_TYPE_SAMPLER, "Game::_sampler" )
    return true;
}

void Game::DestroySamplers ( VkDevice device ) noexcept
{
    if ( _sampler != VK_NULL_HANDLE )
    {
        vkDestroySampler ( device, _sampler, nullptr );
        _sampler = VK_NULL_HANDLE;
    }
}

void Game::DestroyTextures ( android_vulkan::Renderer &renderer ) noexcept
{
    for ( auto &item : _drawcalls )
    {
        item._diffuse.FreeResources ( renderer );
        item._normal.FreeResources ( renderer );
    }
}

bool Game::CreateCommonTextures ( android_vulkan::Renderer &renderer, VkCommandBuffer* commandBuffers ) noexcept
{
    constexpr std::string_view const textureFiles[ MATERIAL_COUNT ] =
    {
        MATERIAL_1_DIFFUSE,
        MATERIAL_2_DIFFUSE,
        MATERIAL_3_DIFFUSE
    };

    for ( size_t i = 0U; i < MATERIAL_COUNT; ++i )
    {
        auto &drawcall = _drawcalls[ i ];

        bool const result = drawcall._diffuse.UploadData ( renderer,
            textureFiles[ i ],
            android_vulkan::eFormat::sRGB,
            true,
            commandBuffers[ i ],
            VK_NULL_HANDLE
        );

        if ( !result )
        {
            return false;
        }
    }

    Drawcall &secondMaterial = _drawcalls[ 1U ];

    bool result = secondMaterial._normal.UploadData ( renderer,
        MATERIAL_2_NORMAL,
        android_vulkan::eFormat::Unorm,
        true,
        commandBuffers[ 3U ],
        VK_NULL_HANDLE
    );

    if ( !result )
        return false;

    Drawcall &thirdMaterial = _drawcalls[ 2U ];

    result = thirdMaterial._normal.UploadData ( renderer,
        MATERIAL_3_NORMAL,
        android_vulkan::eFormat::Unorm,
        true,
        commandBuffers[ 4U ],
        VK_NULL_HANDLE
    );

    if ( !result )
        return false;

    Drawcall &firstMaterial = _drawcalls[ 0U ];
    constexpr uint8_t const defaultNormal[] = { 128U, 128U, 255U, 128U };

    return firstMaterial._normal.UploadData ( renderer,
        defaultNormal,
        std::size ( defaultNormal ),
        VkExtent2D { .width = 1U, .height = 1U },
        VK_FORMAT_R8G8B8A8_UNORM,
        false,
        commandBuffers[ 5U ],
        VK_NULL_HANDLE
    );
}

bool Game::CreateMeshes ( android_vulkan::Renderer &renderer, VkCommandBuffer* commandBuffers ) noexcept
{
    constexpr char const* meshFiles[ MATERIAL_COUNT ] =
        {
            MATERIAL_1_MESH,
            MATERIAL_2_MESH,
            MATERIAL_3_MESH,
        };

    for ( size_t i = 0U; i < MATERIAL_COUNT; ++i )
    {
        bool const result = _drawcalls[ i ]._mesh.LoadMesh ( meshFiles[ i ],
            renderer,
            commandBuffers[ i ],
            VK_NULL_HANDLE
        );

        if ( result )
            continue;

        return false;
    }

    return true;
}

bool Game::IsReady () noexcept
{
    return _pipeline != VK_NULL_HANDLE;
}

bool Game::OnFrame ( android_vulkan::Renderer &renderer, double deltaTime ) noexcept
{
    size_t imageIndex = SIZE_MAX;

    size_t const commandBufferIndex = _writingCommandInfo;
    CommandInfo &commandInfo = _commandInfo[ _writingCommandInfo ];
    _writingCommandInfo = ++_writingCommandInfo % DUAL_COMMAND_BUFFER;

    VkFence &fence = commandInfo._fence;
    VkDevice device = renderer.GetDevice ();

    bool result = android_vulkan::Renderer::CheckVkResult (
        vkWaitForFences ( device, 1U, &fence, VK_TRUE, std::numeric_limits<uint64_t>::max () ),
        "Game::OnFrame",
        "Can't wait fence"
    );

    if ( !result ) [[unlikely]]
        return false;

    if ( !BeginFrame ( renderer, imageIndex, commandInfo._acquire ) )
        return false;

    result = android_vulkan::Renderer::CheckVkResult ( vkResetFences ( device, 1U, &fence ),
        "Game::OnFrame",
        "Can't reset fence"
    );

    if ( !result ) [[unlikely]]
        return false;

    result = android_vulkan::Renderer::CheckVkResult (
        vkResetCommandPool ( device, commandInfo._pool, 0U ),
        "Game::OnFrame",
        "Can't reset command pool"
    );

    if ( !result ) [[unlikely]]
        return false;

    VkCommandBuffer commandBuffer = commandInfo._buffer;

    constexpr VkCommandBufferBeginInfo bufferBeginInfo
    {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
        .pNext = nullptr,
        .flags = 0U,
        .pInheritanceInfo = nullptr
    };

    VkClearValue clearValues[ 2U ];
    VkClearValue &colorTarget = clearValues[ 0U ];
    std::memset ( &colorTarget.color, 0, sizeof ( colorTarget.color ) );

    VkClearValue &depthStencilTarget = clearValues[ 1U ];
    depthStencilTarget.depthStencil.depth = 0.0F;
    depthStencilTarget.depthStencil.stencil = 0x00000000U;

    VkRenderPassBeginInfo renderPassBeginInfo
    {
        .sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
        .pNext = nullptr,
        .renderPass = _renderPass,
        .framebuffer = _framebufferInfo[ imageIndex ]._framebuffer,

        .renderArea = {
            .offset
            {
                .x = 0,
                .y = 0
            },

            .extent = renderer.GetSurfaceSize (),
        },

        .clearValueCount = static_cast<uint32_t> ( std::size ( clearValues ) ),
        .pClearValues = clearValues,
    };

    result = android_vulkan::Renderer::CheckVkResult ( vkBeginCommandBuffer ( commandBuffer, &bufferBeginInfo ),
        "Game::OnFrame",
        "Can't begin command buffer"
    );

    if ( !result )
        return false;

    VkDescriptorSet onceDS = _onceDS[ commandBufferIndex ];
    UpdateUniformBuffer ( renderer, commandBuffer, _onceDS[ commandBufferIndex ], deltaTime );

    vkCmdBeginRenderPass ( commandBuffer, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE );

    vkCmdBindPipeline ( commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, _pipeline );
    constexpr VkDeviceSize offset = 0U;

    VkDescriptorSet standardSets[] = { _fixedDS, onceDS };

    vkCmdBindDescriptorSets ( commandBuffer,
        VK_PIPELINE_BIND_POINT_GRAPHICS,
        _pipelineLayout,
        0U,
        static_cast<uint32_t> ( std::size ( standardSets ) ),
        standardSets,
        0U,
        nullptr
    );

    for ( auto &item : _drawcalls )
    {
        vkCmdBindDescriptorSets ( commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, _pipelineLayout, 2U, 1U,
            &item._descriptorSet, 0U, nullptr
        );

        android_vulkan::MeshGeometry &mesh = item._mesh;

        vkCmdBindVertexBuffers ( commandBuffer, 0U, 1U, &mesh.GetVertexBuffer (), &offset );
        vkCmdDraw ( commandBuffer, mesh.GetVertexCount (), 1U, 0U, 0U );
    }

    vkCmdEndRenderPass ( commandBuffer );

    result = android_vulkan::Renderer::CheckVkResult ( vkEndCommandBuffer ( commandBuffer ),
        "Game::CreateCommandBuffers",
        "Can't end command buffer"
    );

    if ( !result )
        return false;

    constexpr VkPipelineStageFlags waitStage =
        AV_VK_FLAG ( VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT ) |
        AV_VK_FLAG ( VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT ) |
        AV_VK_FLAG ( VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT );

    VkSubmitInfo const submitInfo
    {
        .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
        .pNext = nullptr,
        .waitSemaphoreCount = 1U,
        .pWaitSemaphores = &commandInfo._acquire,
        .pWaitDstStageMask = &waitStage,
        .commandBufferCount = 1U,
        .pCommandBuffers = &commandInfo._buffer,
        .signalSemaphoreCount = 1U,
        .pSignalSemaphores = &_framebufferInfo[ imageIndex ]._renderPassEnd
    };

    result = android_vulkan::Renderer::CheckVkResult (
        vkQueueSubmit ( renderer.GetQueue (), 1U, &submitInfo, commandInfo._fence ),
        "Game::OnFrame",
        "Can't submit command buffer"
    );

    if ( !result )
        return false;

    return EndFrame ( renderer, static_cast<uint32_t> ( imageIndex ) );
}

bool Game::OnInitDevice ( android_vulkan::Renderer &renderer ) noexcept
{
    return CreateCommandPool ( renderer ) &&
        CreateUniformBuffer ( renderer ) &&
        CreateSamplers ( renderer.GetDevice () ) &&
        LoadGPUContent ( renderer, _commandInfo->_pool ) &&
        CreateShaderModules ( renderer ) &&
        CreateCommonDescriptorSetLayouts ( renderer ) &&
        CreatePipelineLayout ( renderer.GetDevice () ) &&
        CreateDescriptorSet ( renderer );
}

void Game::OnDestroyDevice ( android_vulkan::Renderer &renderer ) noexcept
{
    VkDevice device = renderer.GetDevice ();

    DestroyDescriptorSet ( device );
    DestroyPipelineLayout ( device );
    DestroyCommonDescriptorSetLayouts ( device );
    DestroyShaderModules ( device );
    DestroySamplers ( device );
    DestroyMeshes ( renderer );
    DestroyTextures ( renderer );
    DestroyUniformBuffer ( renderer );
    DestroyCommandPool ( device );
}

bool Game::OnSwapchainCreated ( android_vulkan::Renderer &renderer ) noexcept
{
    VkExtent2D const &resolution = renderer.GetViewportResolution ();

    _projectionMatrix.Perspective ( GXDegToRad ( FIELD_OF_VIEW ),
        static_cast<float> ( resolution.width ) / static_cast<float> ( resolution.height ),
        Z_NEAR,
        Z_FAR
    );

    return CreateRenderPass ( renderer ) &&
        CreateFramebuffers ( renderer ) &&
        CreatePipeline ( renderer );
}

void Game::OnSwapchainDestroyed ( android_vulkan::Renderer &renderer ) noexcept
{
    VkDevice device = renderer.GetDevice ();
    DestroyPipeline ( device );
    DestroyFramebuffers ( renderer );
    DestroyRenderPass ( device );
}

bool Game::BeginFrame ( android_vulkan::Renderer &renderer, size_t &imageIndex, VkSemaphore acquire ) noexcept
{
    VkDevice device = renderer.GetDevice ();
    uint32_t i = UINT32_MAX;

    bool result = android_vulkan::Renderer::CheckVkResult (
        vkAcquireNextImageKHR ( device,
            renderer.GetSwapchain (),
            UINT64_MAX,
            acquire,
            VK_NULL_HANDLE,
            &i
        ),

        "Game::BeginFrame",
        "Can't get presentation image index"
    );

    if ( !result )
        return false;

    imageIndex = static_cast<size_t> ( i );
    return true;
}

bool Game::EndFrame ( android_vulkan::Renderer &renderer, uint32_t presentationImageIndex ) noexcept
{
    VkResult presentResult;

    VkPresentInfoKHR const presentInfo
    {
        .sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
        .pNext = nullptr,
        .waitSemaphoreCount = 1U,
        .pWaitSemaphores = &_framebufferInfo[ presentationImageIndex ]._renderPassEnd,
        .swapchainCount = 1U,
        .pSwapchains = &renderer.GetSwapchain (),
        .pImageIndices = &presentationImageIndex,
        .pResults = &presentResult
    };

    bool const result = android_vulkan::Renderer::CheckVkResult (
        vkQueuePresentKHR ( renderer.GetQueue (), &presentInfo ),
        "Game::EndFrame",
        "Can't present frame"
    );

    if ( !result ) [[unlikely]]
        return false;

    return android_vulkan::Renderer::CheckVkResult ( presentResult, "Game::EndFrame", "Present queue has been failed" );
}

bool Game::CreateCommandPool ( android_vulkan::Renderer &renderer ) noexcept
{
    VkDevice device = renderer.GetDevice ();

    VkCommandPoolCreateInfo const createInfo
    {
        .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
        .pNext = nullptr,
        .flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT,
        .queueFamilyIndex = renderer.GetQueueFamilyIndex ()
    };

    VkCommandBufferAllocateInfo bufferAllocateInfo
    {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
        .pNext = nullptr,
        .commandPool = VK_NULL_HANDLE,
        .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
        .commandBufferCount = 1U
    };

    for ( size_t i = 0U; i < DUAL_COMMAND_BUFFER; ++i )
    {
        CommandInfo &info = _commandInfo[ i ];

        constexpr VkSemaphoreCreateInfo semaphoreInfo
        {
            .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
            .pNext = nullptr,
            .flags = 0U
        };

        bool result = android_vulkan::Renderer::CheckVkResult (
            vkCreateSemaphore ( device, &semaphoreInfo, nullptr, &info._acquire ),
            "Game::CreateCommandPool",
            "Can't create render target acquired semaphore"
        );

        if ( !result )
            return false;

        AV_SET_VULKAN_OBJECT_NAME ( device, info._acquire, VK_OBJECT_TYPE_SEMAPHORE, "Frame in flight #%zu", i )

        constexpr VkFenceCreateInfo fenceInfo
        {
            .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
            .pNext = nullptr,
            .flags = VK_FENCE_CREATE_SIGNALED_BIT
        };

        result = android_vulkan::Renderer::CheckVkResult (
            vkCreateFence ( device, &fenceInfo, nullptr, &info._fence ),
            "Game::CreateCommandPool",
            "Can't create fence"
        );

        if ( !result ) [[unlikely]]
            return false;

        AV_SET_VULKAN_OBJECT_NAME ( device, info._fence, VK_OBJECT_TYPE_FENCE, "Frame in flight #%zu", i )

        result = android_vulkan::Renderer::CheckVkResult (
            vkCreateCommandPool ( device, &createInfo, nullptr, &info._pool ),
            "Game::CreateCommandPool",
            "Can't create command pool"
        );

        if ( !result ) [[unlikely]]
            return false;

        AV_SET_VULKAN_OBJECT_NAME ( device, info._pool, VK_OBJECT_TYPE_COMMAND_POOL, "Frame in flight #%zu", i )

        bufferAllocateInfo.commandPool = info._pool;

        result = android_vulkan::Renderer::CheckVkResult (
            vkAllocateCommandBuffers ( device, &bufferAllocateInfo, &info._buffer ),
            "Game::CreateCommandPool",
            "Can't allocate command buffer"
        );

        if ( !result ) [[unlikely]]
            return false;

        AV_SET_VULKAN_OBJECT_NAME ( device, info._buffer, VK_OBJECT_TYPE_COMMAND_BUFFER, "Frame in flight #%zu", i )
    }

    return true;
}

void Game::DestroyCommandPool ( VkDevice device ) noexcept
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

bool Game::CreateCommonDescriptorSetLayouts ( android_vulkan::Renderer &renderer ) noexcept
{
    constexpr VkDescriptorSetLayoutBinding fixedBinding
    {
        .binding = 0U,
        .descriptorType = VK_DESCRIPTOR_TYPE_SAMPLER,
        .descriptorCount = 1U,
        .stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT,
        .pImmutableSamplers = nullptr
    };

    VkDescriptorSetLayoutCreateInfo dsInfo {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0U,
        .bindingCount = 1U,
        .pBindings = &fixedBinding
    };

    VkDevice device = renderer.GetDevice ();

    bool result = android_vulkan::Renderer::CheckVkResult (
        vkCreateDescriptorSetLayout ( device, &dsInfo, nullptr, &_fixedDSLayout ),
        "Game::CreateCommonDescriptorSetLayouts",
        "Can't create fixed descriptor set layout"
    );

    if ( !result ) [[unlikely]]
        return false;

    AV_SET_VULKAN_OBJECT_NAME ( device, _fixedDSLayout, VK_OBJECT_TYPE_DESCRIPTOR_SET_LAYOUT, "Game::_fixedDSLayout" )

    constexpr VkDescriptorSetLayoutBinding onceBinding
    {
        .binding = 0U,
        .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
        .descriptorCount = 1U,
        .stageFlags = VK_SHADER_STAGE_VERTEX_BIT,
        .pImmutableSamplers = nullptr
    };

    dsInfo.bindingCount = 1U;
    dsInfo.pBindings = &onceBinding;

    result = android_vulkan::Renderer::CheckVkResult (
        vkCreateDescriptorSetLayout ( device, &dsInfo, nullptr, &_onceDSLayout ),
        "Game::CreateCommonDescriptorSetLayouts",
        "Can't create once descriptor set layout"
    );

    if ( !result ) [[unlikely]]
        return false;

    AV_SET_VULKAN_OBJECT_NAME ( device, _onceDSLayout, VK_OBJECT_TYPE_DESCRIPTOR_SET_LAYOUT, "Game::_onceDSLayout" )
    return CreateMaterialDescriptorSetLayout ( renderer );
}

void Game::DestroyCommonDescriptorSetLayouts ( VkDevice device ) noexcept
{
    if ( _fixedDSLayout != VK_NULL_HANDLE )
    {
        vkDestroyDescriptorSetLayout ( device, _fixedDSLayout, nullptr );
        _fixedDSLayout = VK_NULL_HANDLE;
    }

    if ( _materialDSLayout != VK_NULL_HANDLE )
    {
        vkDestroyDescriptorSetLayout ( device, _materialDSLayout, nullptr );
        _materialDSLayout = VK_NULL_HANDLE;
    }

    if ( _onceDSLayout == VK_NULL_HANDLE )
        return;

    vkDestroyDescriptorSetLayout ( device, _onceDSLayout, nullptr );
    _onceDSLayout = VK_NULL_HANDLE;
}

void Game::DestroyDescriptorSet ( VkDevice device ) noexcept
{
    if ( _descriptorPool == VK_NULL_HANDLE )
        return;

    vkDestroyDescriptorPool ( device, _descriptorPool, nullptr );
    _descriptorPool = VK_NULL_HANDLE;
    AV_UNREGISTER_DESCRIPTOR_POOL ( "Game::_descriptorPool" )
}

void Game::DestroyMeshes ( android_vulkan::Renderer &renderer ) noexcept
{
    for ( auto &item : _drawcalls )
    {
        item._mesh.FreeResources ( renderer );
    }
}

bool Game::CreateFramebuffers ( android_vulkan::Renderer &renderer ) noexcept
{
    VkDevice device = renderer.GetDevice ();
    VkExtent2D const &resolution = renderer.GetSurfaceSize ();

    bool result = _depthStencilRenderTarget.CreateRenderTarget ( resolution,
        renderer.GetDefaultDepthStencilFormat (),
        VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
        renderer
    );

    if ( !result ) [[unlikely]]
        return false;

    size_t const framebufferCount = renderer.GetPresentImageCount ();
    _framebufferInfo.reserve ( framebufferCount );
    VkFramebuffer framebuffer;

    VkImageView attachments[ 2U ] = { VK_NULL_HANDLE, _depthStencilRenderTarget.GetImageView () };

    VkFramebufferCreateInfo framebufferInfo {
        .sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0U,
        .renderPass = _renderPass,
        .attachmentCount = static_cast<uint32_t> ( std::size ( attachments ) ),
        .pAttachments = attachments,
        .width = resolution.width,
        .height = resolution.height,
        .layers = 1U,
    };

    for ( size_t i = 0U; i < framebufferCount; ++i )
    {
        attachments[ 0U ] = renderer.GetPresentImageView ( i );

        result = android_vulkan::Renderer::CheckVkResult (
            vkCreateFramebuffer ( device, &framebufferInfo, nullptr, &framebuffer ),
            "Game::CreateFramebuffers",
            "Can't create a framebuffer"
        );

        if ( !result ) [[unlikely]]
            return false;

        AV_SET_VULKAN_OBJECT_NAME ( device, framebuffer, VK_OBJECT_TYPE_FRAMEBUFFER, "Swapchain image #%zu", i )

        constexpr VkSemaphoreCreateInfo semaphoreInfo
        {
            .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
            .pNext = nullptr,
            .flags = 0U
        };

        VkSemaphore semaphore;

        result = android_vulkan::Renderer::CheckVkResult (
            vkCreateSemaphore ( device, &semaphoreInfo, nullptr, &semaphore ),
            "Game::CreateFramebuffers",
            "Can't create render pass end semaphore"
        );

        if ( !result ) [[unlikely]]
            return false;

        AV_SET_VULKAN_OBJECT_NAME ( device, semaphore, VK_OBJECT_TYPE_SEMAPHORE, "Swapchain image #%zu", i )

        _framebufferInfo.push_back (
            FramebufferInfo {
                ._framebuffer = framebuffer,
                ._renderPassEnd = semaphore
            }
        );
    }

    return true;
}

void Game::DestroyFramebuffers ( android_vulkan::Renderer &renderer ) noexcept
{
    if ( !_framebufferInfo.empty () )
    {
        VkDevice device = renderer.GetDevice ();

        for ( auto const &info : _framebufferInfo )
        {
            if ( info._framebuffer != VK_NULL_HANDLE ) [[likely]]
                vkDestroyFramebuffer ( device, info._framebuffer, nullptr );

            if ( info._renderPassEnd != VK_NULL_HANDLE ) [[likely]]
            {
                vkDestroySemaphore ( device, info._renderPassEnd, nullptr );
            }
        }

        _framebufferInfo.clear ();
    }

    _depthStencilRenderTarget.FreeResources ( renderer );
}

bool Game::CreatePipelineLayout ( VkDevice device ) noexcept
{
    VkDescriptorSetLayout const layouts[] =
    {
        _fixedDSLayout,
        _onceDSLayout,
        _materialDSLayout
    };

    VkPipelineLayoutCreateInfo const layoutInfo
    {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0U,
        .setLayoutCount = static_cast<uint32_t> ( std::size ( layouts ) ),
        .pSetLayouts = layouts,
        .pushConstantRangeCount = 0U,
        .pPushConstantRanges = nullptr
    };

    bool const result = android_vulkan::Renderer::CheckVkResult (
        vkCreatePipelineLayout ( device, &layoutInfo, nullptr, &_pipelineLayout ),
        "Game::CreatePipelineLayout",
        "Can't create pipeline layout"
    );

    if ( !result ) [[unlikely]]
        return false;

    AV_SET_VULKAN_OBJECT_NAME ( device,
        _pipelineLayout,
        VK_OBJECT_TYPE_PIPELINE_LAYOUT,
        "PointLightLightupProgram::_pipelineLayout"
    )

    return true;
}

bool Game::CreatePipeline ( android_vulkan::Renderer &renderer ) noexcept
{
    VkPipelineShaderStageCreateInfo stageInfo[ 2U ];
    VkPipelineShaderStageCreateInfo &vertexStage = stageInfo[ 0U ];
    vertexStage.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    vertexStage.pNext = nullptr;
    vertexStage.flags = 0U;
    vertexStage.pSpecializationInfo = nullptr;
    vertexStage.stage = VK_SHADER_STAGE_VERTEX_BIT;
    vertexStage.module = _vertexShaderModule;
    vertexStage.pName = VERTEX_SHADER_ENTRY_POINT;

    VkPipelineShaderStageCreateInfo &fragmentStage = stageInfo[ 1U ];
    fragmentStage.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    fragmentStage.pNext = nullptr;
    fragmentStage.flags = 0U;
    fragmentStage.pSpecializationInfo = nullptr;
    fragmentStage.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    fragmentStage.module = _fragmentShaderModule;
    fragmentStage.pName = FRAGMENT_SHADER_ENTRY_POINT;

    VkPipelineInputAssemblyStateCreateInfo assemblyInfo;
    assemblyInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    assemblyInfo.pNext = nullptr;
    assemblyInfo.flags = 0U;
    assemblyInfo.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    assemblyInfo.primitiveRestartEnable = VK_FALSE;

    VkVertexInputAttributeDescription attributeDescriptions[ 5U ];
    VkVertexInputAttributeDescription &vertexDescription = attributeDescriptions[ 0U ];
    vertexDescription.location = 0U;
    vertexDescription.binding = 0U;
    vertexDescription.offset = static_cast<uint32_t> ( offsetof ( android_vulkan::VertexInfo, _vertex ) );
    vertexDescription.format = VK_FORMAT_R32G32B32_SFLOAT;

    VkVertexInputAttributeDescription &uvDescription = attributeDescriptions[ 1U ];
    uvDescription.location = 1U;
    uvDescription.binding = 0U;
    uvDescription.offset = static_cast<uint32_t> ( offsetof ( android_vulkan::VertexInfo, _uv ) );
    uvDescription.format = VK_FORMAT_R32G32_SFLOAT;

    VkVertexInputAttributeDescription &normalDescription = attributeDescriptions[ 2U ];
    normalDescription.location = 2U;
    normalDescription.binding = 0U;
    normalDescription.offset = static_cast<uint32_t> ( offsetof ( android_vulkan::VertexInfo, _normal ) );
    normalDescription.format = VK_FORMAT_R32G32B32_SFLOAT;

    VkVertexInputAttributeDescription &tangentDescription = attributeDescriptions[ 3U ];
    tangentDescription.location = 3U;
    tangentDescription.binding = 0U;
    tangentDescription.offset = static_cast<uint32_t> ( offsetof ( android_vulkan::VertexInfo, _tangent ) );
    tangentDescription.format = VK_FORMAT_R32G32B32_SFLOAT;

    VkVertexInputAttributeDescription &bitangentDescription = attributeDescriptions[ 4U ];
    bitangentDescription.location = 4U;
    bitangentDescription.binding = 0U;
    bitangentDescription.offset = static_cast<uint32_t> ( offsetof ( android_vulkan::VertexInfo, _bitangent ) );
    bitangentDescription.format = VK_FORMAT_R32G32B32_SFLOAT;

    VkVertexInputBindingDescription bindingDescription;
    bindingDescription.binding = 0U;
    bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
    bindingDescription.stride = sizeof ( android_vulkan::VertexInfo );

    VkPipelineVertexInputStateCreateInfo vertexInputInfo;
    vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertexInputInfo.pNext = nullptr;
    vertexInputInfo.flags = 0U;
    vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t> ( std::size ( attributeDescriptions ) );
    vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions;
    vertexInputInfo.vertexBindingDescriptionCount = 1U;
    vertexInputInfo.pVertexBindingDescriptions = &bindingDescription;

    VkPipelineDepthStencilStateCreateInfo depthStencilInfo;
    depthStencilInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    depthStencilInfo.pNext = nullptr;
    depthStencilInfo.flags = 0U;
    depthStencilInfo.stencilTestEnable = VK_FALSE;
    depthStencilInfo.depthTestEnable = VK_TRUE;
    depthStencilInfo.depthBoundsTestEnable = VK_FALSE;
    depthStencilInfo.depthCompareOp = VK_COMPARE_OP_GREATER;
    depthStencilInfo.depthWriteEnable = VK_TRUE;
    depthStencilInfo.minDepthBounds = 0.0F;
    depthStencilInfo.maxDepthBounds = 1.0F;
    depthStencilInfo.front.compareOp = VK_COMPARE_OP_ALWAYS;
    depthStencilInfo.front.compareMask = 0U;
    depthStencilInfo.front.failOp = VK_STENCIL_OP_KEEP;
    depthStencilInfo.front.passOp = VK_STENCIL_OP_KEEP;
    depthStencilInfo.front.depthFailOp = VK_STENCIL_OP_KEEP;
    depthStencilInfo.front.reference = UINT32_MAX;
    depthStencilInfo.front.writeMask = UINT32_MAX;
    depthStencilInfo.back.compareOp = VK_COMPARE_OP_ALWAYS;
    depthStencilInfo.back.compareMask = 0U;
    depthStencilInfo.back.failOp = VK_STENCIL_OP_KEEP;
    depthStencilInfo.back.passOp = VK_STENCIL_OP_KEEP;
    depthStencilInfo.back.depthFailOp = VK_STENCIL_OP_KEEP;
    depthStencilInfo.back.reference = UINT32_MAX;
    depthStencilInfo.back.writeMask = UINT32_MAX;

    VkPipelineColorBlendAttachmentState attachmentInfo;
    attachmentInfo.blendEnable = VK_FALSE;

    attachmentInfo.colorWriteMask =
        AV_VK_FLAG ( VK_COLOR_COMPONENT_R_BIT ) |
        AV_VK_FLAG ( VK_COLOR_COMPONENT_G_BIT ) |
        AV_VK_FLAG ( VK_COLOR_COMPONENT_B_BIT ) |
        AV_VK_FLAG ( VK_COLOR_COMPONENT_A_BIT );

    attachmentInfo.alphaBlendOp = VK_BLEND_OP_ADD;
    attachmentInfo.colorBlendOp = VK_BLEND_OP_ADD;
    attachmentInfo.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
    attachmentInfo.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
    attachmentInfo.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;
    attachmentInfo.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;

    VkPipelineColorBlendStateCreateInfo blendInfo;
    blendInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    blendInfo.pNext = nullptr;
    blendInfo.flags = 0U;
    blendInfo.attachmentCount = 1U;
    blendInfo.pAttachments = &attachmentInfo;
    blendInfo.logicOpEnable = VK_FALSE;
    std::memset ( blendInfo.blendConstants, 0, sizeof ( blendInfo.blendConstants ) );

    VkPipelineMultisampleStateCreateInfo multisampleInfo;
    multisampleInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisampleInfo.pNext = nullptr;
    multisampleInfo.flags = 0U;
    multisampleInfo.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
    multisampleInfo.alphaToCoverageEnable = VK_FALSE;
    multisampleInfo.alphaToOneEnable = VK_FALSE;
    multisampleInfo.sampleShadingEnable = VK_FALSE;
    multisampleInfo.pSampleMask = nullptr;

    VkPipelineRasterizationStateCreateInfo rasterizationInfo;
    rasterizationInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterizationInfo.pNext = nullptr;
    rasterizationInfo.flags = 0U;
    rasterizationInfo.depthBiasEnable = VK_FALSE;
    rasterizationInfo.depthClampEnable = VK_FALSE;
    rasterizationInfo.rasterizerDiscardEnable = VK_FALSE;
    rasterizationInfo.polygonMode = VK_POLYGON_MODE_FILL;
    rasterizationInfo.frontFace = VK_FRONT_FACE_CLOCKWISE;
    rasterizationInfo.cullMode = VK_CULL_MODE_BACK_BIT;
    rasterizationInfo.lineWidth = 1.0F;

    VkExtent2D const &surfaceSize = renderer.GetSurfaceSize ();

    VkRect2D scissor;
    scissor.extent = surfaceSize;
    std::memset ( &scissor.offset, 0, sizeof ( scissor.offset ) );

    VkViewport viewport;
    viewport.x = viewport.y = 0.0F;
    viewport.width = static_cast<float> ( surfaceSize.width );
    viewport.height = static_cast<float> ( surfaceSize.height );
    viewport.minDepth = 0.0F;
    viewport.maxDepth = 1.0F;

    VkPipelineViewportStateCreateInfo viewportInfo;
    viewportInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewportInfo.pNext = nullptr;
    viewportInfo.flags = 0U;
    viewportInfo.viewportCount = 1U;
    viewportInfo.pViewports = &viewport;
    viewportInfo.scissorCount = 1U;
    viewportInfo.pScissors = &scissor;

    VkGraphicsPipelineCreateInfo const pipelineInfo
    {
        .sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0U,
        .stageCount = static_cast<uint32_t> ( std::size ( stageInfo ) ),
        .pStages = stageInfo,
        .pVertexInputState = &vertexInputInfo,
        .pInputAssemblyState = &assemblyInfo,
        .pTessellationState = nullptr,
        .pViewportState = &viewportInfo,
        .pRasterizationState = &rasterizationInfo,
        .pMultisampleState = &multisampleInfo,
        .pDepthStencilState = &depthStencilInfo,
        .pColorBlendState = &blendInfo,
        .pDynamicState = nullptr,
        .layout = _pipelineLayout,
        .renderPass = _renderPass,
        .subpass = 0U,
        .basePipelineHandle = VK_NULL_HANDLE,
        .basePipelineIndex = -1,
    };

    VkDevice device = renderer.GetDevice ();

    bool const result = android_vulkan::Renderer::CheckVkResult (
        vkCreateGraphicsPipelines ( device, VK_NULL_HANDLE, 1U, &pipelineInfo, nullptr, &_pipeline ),
        "Game::CreatePipeline",
        "Can't create pipeline"
    );

    if ( !result ) [[unlikely]]
        return false;

    AV_SET_VULKAN_OBJECT_NAME ( device, _pipeline, VK_OBJECT_TYPE_PIPELINE, "Game::_pipeline" )
    return true;
}

void Game::DestroyPipeline ( VkDevice device ) noexcept
{
    if ( _pipeline == VK_NULL_HANDLE ) [[unlikely]]
        return;

    vkDestroyPipeline ( device, _pipeline, nullptr );
    _pipeline = VK_NULL_HANDLE;
}

void Game::DestroyPipelineLayout ( VkDevice device ) noexcept
{
    if ( _pipelineLayout == VK_NULL_HANDLE ) [[unlikely]]
        return;

    vkDestroyPipelineLayout ( device, _pipelineLayout, nullptr );
    _pipelineLayout = VK_NULL_HANDLE;
}

bool Game::CreateRenderPass ( android_vulkan::Renderer &renderer ) noexcept
{
    VkAttachmentDescription const attachmentInfo[] =
    {
        // Color
        {
            .flags = 0U,
            .format = renderer.GetSurfaceFormat (),
            .samples = VK_SAMPLE_COUNT_1_BIT,
            .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
            .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
            .stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
            .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
            .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
            .finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR
        },

        // Depth stencil
        {
            .flags = 0U,
            .format = renderer.GetDefaultDepthStencilFormat (),
            .samples = VK_SAMPLE_COUNT_1_BIT,
            .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
            .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
            .stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
            .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
            .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
            .finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL
        }
    };

    constexpr static VkAttachmentReference colorReference
    {
        .attachment = 0U,
        .layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
    };

    constexpr static VkAttachmentReference depthStencilReference
    {
        .attachment = 1U,
        .layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL
    };

    constexpr VkSubpassDescription subpassInfo
    {
        .flags = 0U,
        .pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS,
        .inputAttachmentCount = 0U,
        .pInputAttachments = nullptr,
        .colorAttachmentCount = 1U,
        .pColorAttachments = &colorReference,
        .pResolveAttachments = nullptr,
        .pDepthStencilAttachment = &depthStencilReference,
        .preserveAttachmentCount = 0U,
        .pPreserveAttachments = nullptr
    };

    constexpr VkSubpassDependency const dependencies[] = {
        {
            .srcSubpass = VK_SUBPASS_EXTERNAL,
            .dstSubpass = 0U,
            .srcStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
            .dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
            .srcAccessMask = VK_ACCESS_NONE,
            .dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
            .dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT
        },
        {
            .srcSubpass = VK_SUBPASS_EXTERNAL,
            .dstSubpass = 0U,
            .srcStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,

            .dstStageMask = AV_VK_FLAG ( VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT ) |
                AV_VK_FLAG ( VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT ),

            .srcAccessMask = VK_ACCESS_NONE,
            .dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
            .dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT
        },
    };

    VkRenderPassCreateInfo const renderPassInfo
    {
        .sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0U,
        .attachmentCount = static_cast<uint32_t> ( std::size ( attachmentInfo ) ),
        .pAttachments = attachmentInfo,
        .subpassCount = 1U,
        .pSubpasses = &subpassInfo,
        .dependencyCount = static_cast<uint32_t> ( std::size ( dependencies ) ),
        .pDependencies = dependencies
    };

    VkDevice device = renderer.GetDevice ();

    bool const result = android_vulkan::Renderer::CheckVkResult (
        vkCreateRenderPass ( device, &renderPassInfo, nullptr, &_renderPass ),
        "Game::CreateRenderPass",
        "Can't create render pass"
    );

    if ( !result ) [[unlikely]]
        return false;

    AV_SET_VULKAN_OBJECT_NAME ( device, _renderPass, VK_OBJECT_TYPE_RENDER_PASS, "Game::_renderPass" )
    return true;
}

void Game::DestroyRenderPass ( VkDevice device ) noexcept
{
    if ( _renderPass == VK_NULL_HANDLE )
        return;

    vkDestroyRenderPass ( device, _renderPass, nullptr );
    _renderPass = VK_NULL_HANDLE;
}

bool Game::CreateShaderModules ( android_vulkan::Renderer &renderer ) noexcept
{
    bool result = renderer.CreateShader ( _vertexShaderModule,
        VERTEX_SHADER,
        "Can't create vertex shader (Game::CreateShaderModules)"
    );

    if ( !result ) [[unlikely]]
        return false;

    AV_SET_VULKAN_OBJECT_NAME ( renderer.GetDevice (),
        _vertexShaderModule,
        VK_OBJECT_TYPE_SHADER_MODULE,
        VERTEX_SHADER
    )

    result = renderer.CreateShader ( _fragmentShaderModule,
        _fragmentShader,
        "Can't create fragment shader (Game::CreateShaderModules)"
    );

    if ( !result ) [[unlikely]]
        return false;

    AV_SET_VULKAN_OBJECT_NAME ( renderer.GetDevice (),
        _fragmentShaderModule,
        VK_OBJECT_TYPE_SHADER_MODULE,
        "%s",
        _fragmentShader
    )

    return true;
}

void Game::DestroyShaderModules ( VkDevice device ) noexcept
{
    if ( _fragmentShaderModule != VK_NULL_HANDLE ) [[likely]]
    {
        vkDestroyShaderModule ( device, _fragmentShaderModule, nullptr );
        _fragmentShaderModule = VK_NULL_HANDLE;
    }

    if ( _vertexShaderModule == VK_NULL_HANDLE ) [[unlikely]]
        return;

    vkDestroyShaderModule ( device, _vertexShaderModule, nullptr );
    _vertexShaderModule = VK_NULL_HANDLE;
}

bool Game::CreateUniformBuffer ( android_vulkan::Renderer &renderer ) noexcept
{
    return _transformBuffer.Init ( renderer, sizeof ( Transform ), DUAL_COMMAND_BUFFER );
}

void Game::DestroyUniformBuffer ( android_vulkan::Renderer &renderer ) noexcept
{
    _transformBuffer.FreeResources ( renderer );
}

void Game::UpdateUniformBuffer ( android_vulkan::Renderer &renderer,
    VkCommandBuffer commandBuffer,
    VkDescriptorSet ds,
    double deltaTime
) noexcept
{
    _angle += static_cast<float> ( deltaTime ) * ROTATION_SPEED;
    _transform._normalTransform.RotationY ( _angle );
    _transform._normalTransform.SetOrigin ( GXVec3 ( 0.0F, -1.0F, 3.0F ) );

    GXMat4 tmp {};
    tmp.Multiply ( _transform._normalTransform, _projectionMatrix );
    _transform._transform.Multiply ( tmp, renderer.GetPresentationEngineTransform () );

    VkDescriptorBufferInfo const bufferInfo {
        .buffer = _transformBuffer.Update ( commandBuffer, reinterpret_cast<uint8_t const*> ( &_transform ) ),
        .offset = 0U,
        .range = VK_WHOLE_SIZE
    };

    VkWriteDescriptorSet const write {
        .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
        .pNext = nullptr,
        .dstSet = ds,
        .dstBinding = 0U,
        .dstArrayElement = 0U,
        .descriptorCount = 1U,
        .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
        .pImageInfo = nullptr,
        .pBufferInfo = &bufferInfo,
        .pTexelBufferView = nullptr
    };

    vkUpdateDescriptorSets ( renderer.GetDevice (), 1U, &write, 0U, nullptr );

    VkBufferMemoryBarrier const bufferBarrier {
        .sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER,
        .pNext = nullptr,
        .srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT,
        .dstAccessMask = VK_ACCESS_UNIFORM_READ_BIT,
        .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
        .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
        .buffer = bufferInfo.buffer,
        .offset = 0U,
        .size = static_cast<VkDeviceSize> ( sizeof ( Transform ) )
    };

    vkCmdPipelineBarrier ( commandBuffer,
        VK_PIPELINE_STAGE_TRANSFER_BIT,
        VK_PIPELINE_STAGE_VERTEX_SHADER_BIT,
        0U,
        0U,
        nullptr,
        1U,
        &bufferBarrier,
        0U,
        nullptr
    );
}

} // namespace rotating_mesh
