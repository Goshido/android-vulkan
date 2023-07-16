#include <rotating_mesh/game.hpp>

GX_DISABLE_COMMON_WARNINGS

#include <array>
#include <cmath>
#include <thread>

GX_RESTORE_WARNING_STATE

#include <vertex_info.hpp>
#include <vulkan_utils.hpp>


namespace rotating_mesh {

constexpr static char const* VERTEX_SHADER = "shaders/static_mesh.vs.spv";
constexpr static char const* VERTEX_SHADER_ENTRY_POINT = "VS";

constexpr static char const* FRAGMENT_SHADER_ENTRY_POINT = "PS";

constexpr static std::string_view MATERIAL_1_DIFFUSE = "textures/rotating_mesh/sonic-material-1-diffuse.ktx";
constexpr static char const* MATERIAL_1_MESH = "meshes/rotating_mesh/sonic-material-1.mesh";

constexpr static std::string_view MATERIAL_2_DIFFUSE = "textures/rotating_mesh/sonic-material-2-diffuse.ktx";
constexpr static char const* MATERIAL_2_MESH = "meshes/rotating_mesh/sonic-material-2.mesh";
constexpr static std::string_view MATERIAL_2_NORMAL = "textures/rotating_mesh/sonic-material-2-normal.png";

constexpr static std::string_view MATERIAL_3_DIFFUSE = "textures/rotating_mesh/sonic-material-3-diffuse.ktx";
constexpr static char const* MATERIAL_3_MESH = "meshes/rotating_mesh/sonic-material-3.mesh";
constexpr static std::string_view MATERIAL_3_NORMAL = "textures/rotating_mesh/sonic-material-3-normal.png";

constexpr static float ROTATION_SPEED = GX_MATH_HALF_PI;
constexpr static float FIELD_OF_VIEW = 60.0F;
constexpr static float Z_NEAR = 0.1F;
constexpr static float Z_FAR = 1.0e+3F;

//----------------------------------------------------------------------------------------------------------------------

Game::Game ( char const* fragmentShader ) noexcept:
    _fragmentShader ( fragmentShader )
{
    // NOTHING
}

bool Game::CreateSamplers ( android_vulkan::Renderer &renderer ) noexcept
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
        .maxLod = 8.0F,
        .borderColor = VK_BORDER_COLOR_FLOAT_TRANSPARENT_BLACK,
        .unnormalizedCoordinates = VK_FALSE
    };

    VkDevice device = renderer.GetDevice ();

    bool result = android_vulkan::Renderer::CheckVkResult (
        vkCreateSampler ( device, &samplerInfo, nullptr, &_sampler09Mips ),
        "Game::CreateSamplers",
        "Can't create sampler with 9 mips"
    );

    if ( !result )
        return false;

    AV_REGISTER_SAMPLER ( "Game::_sampler09Mips" )

    samplerInfo.maxLod = 9.0F;

    result = android_vulkan::Renderer::CheckVkResult (
        vkCreateSampler ( device, &samplerInfo, nullptr, &_sampler10Mips ),
        "Game::CreateSamplers",
        "Can't create sampler with 10 mips"
    );

    if ( !result )
        return false;

    AV_REGISTER_SAMPLER ( "Game::_sampler10Mips" )

    samplerInfo.maxLod = 10.0F;

    result = android_vulkan::Renderer::CheckVkResult (
        vkCreateSampler ( device, &samplerInfo, nullptr, &_sampler11Mips ),
        "Game::CreateSamplers",
        "Can't create sampler with 11 mips"
    );

    if ( !result )
        return false;

    AV_REGISTER_SAMPLER ( "Game::_sampler11Mips" )

    samplerInfo.maxLod = 1.0F;
    samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_NEAREST;
    samplerInfo.magFilter = VK_FILTER_NEAREST;
    samplerInfo.minFilter = VK_FILTER_NEAREST;

    result = android_vulkan::Renderer::CheckVkResult (
        vkCreateSampler ( device, &samplerInfo, nullptr, &_sampler01Mips ),
        "Game::CreateSamplers",
        "Can't create sampler with 1 mip"
    );

    if ( !result )
        return false;

    AV_REGISTER_SAMPLER ( "Game::_sampler01Mips" )
    return true;
}

void Game::DestroySamplers ( VkDevice device ) noexcept
{
    if ( _sampler11Mips != VK_NULL_HANDLE )
    {
        vkDestroySampler ( device, _sampler11Mips, nullptr );
        _sampler11Mips = VK_NULL_HANDLE;
        AV_UNREGISTER_SAMPLER ( "Game::_sampler11Mips" )
    }

    if ( _sampler10Mips != VK_NULL_HANDLE )
    {
        vkDestroySampler ( device, _sampler10Mips, nullptr );
        _sampler10Mips = VK_NULL_HANDLE;
        AV_UNREGISTER_SAMPLER ( "Game::_sampler10Mips" )
    }

    if ( _sampler09Mips != VK_NULL_HANDLE )
    {
        vkDestroySampler ( device, _sampler09Mips, nullptr );
        _sampler09Mips = VK_NULL_HANDLE;
        AV_UNREGISTER_SAMPLER ( "Game::_sampler09Mips" )
    }

    if ( _sampler01Mips == VK_NULL_HANDLE )
        return;

    vkDestroySampler ( device, _sampler01Mips, nullptr );
    _sampler01Mips = VK_NULL_HANDLE;
    AV_UNREGISTER_SAMPLER ( "Game::_sampler01Mips" )
}

void Game::DestroyTextures ( android_vulkan::Renderer &renderer ) noexcept
{
    for ( auto &item : _drawcalls )
    {
        item._diffuse.FreeResources ( renderer );
        item._normal.FreeResources ( renderer );
        item._diffuseSampler = item._normalSampler = VK_NULL_HANDLE;
    }
}

bool Game::CreateCommonTextures ( android_vulkan::Renderer &renderer, VkCommandBuffer* commandBuffers ) noexcept
{
    auto selector = [ this ] ( android_vulkan::Texture2D const &texture ) -> VkSampler {
        uint8_t const mips = texture.GetMipLevelCount ();

        if ( mips == 1U )
            return _sampler01Mips;

        if ( mips == 9U )
            return _sampler09Mips;

        if ( mips == 10U )
            return _sampler10Mips;

        if ( mips == 11U )
            return _sampler11Mips;

        android_vulkan::LogError ( "Game::CreateCommonTextures::selector - Can't select sampler" );
        return VK_NULL_HANDLE;
    };

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
            return false;

        drawcall._diffuseSampler = selector ( drawcall._diffuse );
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

    secondMaterial._normalSampler = selector ( secondMaterial._normal );

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

    thirdMaterial._normalSampler = selector ( thirdMaterial._normal );

    Drawcall &firstMaterial = _drawcalls[ 0U ];
    constexpr uint8_t const defaultNormal[] = { 128U, 128U, 255U, 128U };

    result = firstMaterial._normal.UploadData ( renderer,
        defaultNormal,
        std::size ( defaultNormal ),
        VkExtent2D { .width = 1U, .height = 1U },
        VK_FORMAT_R8G8B8A8_UNORM,
        false,
        commandBuffers[ 5U ],
        VK_NULL_HANDLE
    );

    if ( !result )
        return false;

    firstMaterial._normalSampler = selector ( firstMaterial._normal );
    return true;
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
            VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
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

void Game::InitDescriptorPoolSizeCommon ( VkDescriptorPoolSize* features ) noexcept
{
    VkDescriptorPoolSize &ubFeature = features[ 0U ];
    ubFeature.descriptorCount = static_cast<uint32_t> ( MATERIAL_COUNT );
    ubFeature.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;

    VkDescriptorPoolSize &diffuseTextureFeature = features[ 1U ];
    diffuseTextureFeature.descriptorCount = static_cast<uint32_t> ( MATERIAL_COUNT * 2U );
    diffuseTextureFeature.type = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;

    VkDescriptorPoolSize &diffuseSamplerFeature = features[ 2U ];
    diffuseSamplerFeature.descriptorCount = static_cast<uint32_t> ( MATERIAL_COUNT * 2U );
    diffuseSamplerFeature.type = VK_DESCRIPTOR_TYPE_SAMPLER;
}

void Game::InitDescriptorSetLayoutBindingCommon ( VkDescriptorSetLayoutBinding* bindings ) noexcept
{
    VkDescriptorSetLayoutBinding &ubInfo = bindings[ 0U ];
    ubInfo.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
    ubInfo.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    ubInfo.descriptorCount = 1U;
    ubInfo.binding = 0U;
    ubInfo.pImmutableSamplers = nullptr;

    VkDescriptorSetLayoutBinding &diffuseImageInfo = bindings[ 1U ];
    diffuseImageInfo.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
    diffuseImageInfo.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
    diffuseImageInfo.descriptorCount = 1U;
    diffuseImageInfo.binding = 1U;
    diffuseImageInfo.pImmutableSamplers = nullptr;

    VkDescriptorSetLayoutBinding &diffuseSamplerInfo = bindings[ 2U ];
    diffuseSamplerInfo.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
    diffuseSamplerInfo.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLER;
    diffuseSamplerInfo.descriptorCount = 1U;
    diffuseSamplerInfo.binding = 2U;
    diffuseSamplerInfo.pImmutableSamplers = nullptr;

    VkDescriptorSetLayoutBinding &normalImageInfo = bindings[ 3U ];
    normalImageInfo.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
    normalImageInfo.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
    normalImageInfo.descriptorCount = 1U;
    normalImageInfo.binding = 3U;
    normalImageInfo.pImmutableSamplers = nullptr;

    VkDescriptorSetLayoutBinding &normalSamplerInfo = bindings[ 4U ];
    normalSamplerInfo.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
    normalSamplerInfo.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLER;
    normalSamplerInfo.descriptorCount = 1U;
    normalSamplerInfo.binding = 4U;
    normalSamplerInfo.pImmutableSamplers = nullptr;
}

bool Game::IsReady () noexcept
{
    return _pipeline != VK_NULL_HANDLE;
}

bool Game::OnFrame ( android_vulkan::Renderer &renderer, double deltaTime ) noexcept
{
    if ( !UpdateUniformBuffer ( renderer, deltaTime ) )
        return false;

    size_t imageIndex = SIZE_MAX;

    if ( !BeginFrame ( renderer, imageIndex ) )
        return false;

    CommandContext const &commandContext = _commandBuffers[ imageIndex ];

    constexpr VkPipelineStageFlags waitStage =
        AV_VK_FLAG ( VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT ) |
        AV_VK_FLAG ( VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT ) |
        AV_VK_FLAG ( VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT );

    VkSubmitInfo const submitInfo
    {
        .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
        .pNext = nullptr,
        .waitSemaphoreCount = 1U,
        .pWaitSemaphores = &_renderTargetAcquiredSemaphore,
        .pWaitDstStageMask = &waitStage,
        .commandBufferCount = 1U,
        .pCommandBuffers = &commandContext.first,
        .signalSemaphoreCount = 1U,
        .pSignalSemaphores = &_renderPassEndSemaphore
    };

    bool const result = android_vulkan::Renderer::CheckVkResult (
        vkQueueSubmit ( renderer.GetQueue (), 1U, &submitInfo, commandContext.second ),
        "Game::OnFrame",
        "Can't submit command buffer"
    );

    if ( !result )
        return false;

    return EndFrame ( renderer, static_cast<uint32_t> ( imageIndex ) );
}

bool Game::OnInitDevice ( android_vulkan::Renderer &renderer ) noexcept
{
    return CreateSyncPrimitives ( renderer ) &&
        CreateCommandPool ( renderer ) &&
        CreateUniformBuffer ( renderer ) &&
        CreateSamplers ( renderer ) &&
        LoadGPUContent ( renderer ) &&
        CreateShaderModules ( renderer ) &&
        CreatePipelineLayout ( renderer ) &&
        CreateDescriptorSet ( renderer );
}

void Game::OnDestroyDevice ( android_vulkan::Renderer &renderer ) noexcept
{
    VkDevice device = renderer.GetDevice ();

    DestroyDescriptorSet ( device );
    DestroyPipelineLayout ( device );
    DestroyShaderModules ( device );
    DestroySamplers ( device );
    DestroyMeshes ( renderer );
    DestroyTextures ( renderer );
    DestroyUniformBuffer ( renderer );
    DestroyCommandPool ( device );
    DestroySyncPrimitives ( device );
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
        CreatePipeline ( renderer ) &&
        CreateCommandBuffers ( renderer );
}

void Game::OnSwapchainDestroyed ( android_vulkan::Renderer &renderer ) noexcept
{
    VkDevice device = renderer.GetDevice ();

    DestroyCommandBuffers ( device );
    DestroyPipeline ( device );
    DestroyFramebuffers ( renderer );
    DestroyRenderPass ( device );
}

bool Game::BeginFrame ( android_vulkan::Renderer &renderer, size_t &imageIndex ) noexcept
{
    VkDevice device = renderer.GetDevice ();
    uint32_t i = UINT32_MAX;

    bool result = android_vulkan::Renderer::CheckVkResult (
        vkAcquireNextImageKHR ( device,
            renderer.GetSwapchain (),
            UINT64_MAX,
            _renderTargetAcquiredSemaphore,
            VK_NULL_HANDLE,
            &i
        ),

        "Game::BeginFrame",
        "Can't get presentation image index"
    );

    if ( !result )
        return false;

    imageIndex = static_cast<size_t> ( i );
    CommandContext const &commandContext = _commandBuffers[ imageIndex ];

    result = android_vulkan::Renderer::CheckVkResult (
        vkWaitForFences ( device, 1U, &commandContext.second, VK_TRUE, UINT64_MAX ),
        "Game::BeginFrame",
        "Can't wait fence"
    );

    if ( !result )
        return false;

    return android_vulkan::Renderer::CheckVkResult ( vkResetFences ( device, 1U, &commandContext.second ),
        "Game::BeginFrame",
        "Can't reset fence"
    );
}

bool Game::EndFrame ( android_vulkan::Renderer &renderer, uint32_t presentationImageIndex ) noexcept
{
    VkResult presentResult = VK_ERROR_DEVICE_LOST;

    VkPresentInfoKHR const presentInfo
    {
        .sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
        .pNext = nullptr,
        .waitSemaphoreCount = 1U,
        .pWaitSemaphores = &_renderPassEndSemaphore,
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

    if ( !result )
        return false;

    return android_vulkan::Renderer::CheckVkResult ( presentResult, "Game::EndFrame", "Present queue has been failed" );
}

bool Game::CreateCommandPool ( android_vulkan::Renderer &renderer ) noexcept
{
    VkCommandPoolCreateInfo const poolInfo
    {
        .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0U,
        .queueFamilyIndex = renderer.GetQueueFamilyIndex ()
    };

    bool const result = android_vulkan::Renderer::CheckVkResult (
        vkCreateCommandPool ( renderer.GetDevice (), &poolInfo, nullptr, &_commandPool ),
        "Game::CreateCommandPool",
        "Can't create command pool"
    );

    if ( !result )
        return false;

    AV_REGISTER_COMMAND_POOL ( "Game::_commandPool" )
    return true;
}

void Game::DestroyCommandPool ( VkDevice device ) noexcept
{
    if ( !_commandBuffers.empty () )
    {
        for ( auto const &item : _commandBuffers )
        {
            vkDestroyFence ( device, item.second, nullptr );
            AV_UNREGISTER_FENCE ( "Game::_commandBuffers::_fence" )
        }

        _commandBuffers.clear ();
    }

    if ( _commandPool == VK_NULL_HANDLE )
        return;

    vkDestroyCommandPool ( device, _commandPool, nullptr );
    _commandPool = VK_NULL_HANDLE;
    AV_UNREGISTER_COMMAND_POOL ( "Game::_commandPool" )
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

    if ( !result )
        return false;

    size_t const framebufferCount = renderer.GetPresentImageCount ();
    _framebuffers.reserve ( framebufferCount );
    VkFramebuffer framebuffer = VK_NULL_HANDLE;

    VkImageView attachments[ 2U ];
    attachments[ 1U ] = _depthStencilRenderTarget.GetImageView ();

    VkFramebufferCreateInfo framebufferInfo;
    framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    framebufferInfo.pNext = nullptr;
    framebufferInfo.flags = 0U;
    framebufferInfo.renderPass = _renderPass;
    framebufferInfo.layers = 1U;
    framebufferInfo.width = resolution.width;
    framebufferInfo.height = resolution.height;
    framebufferInfo.attachmentCount = static_cast<uint32_t> ( std::size ( attachments ) );
    framebufferInfo.pAttachments = attachments;

    for ( size_t i = 0U; i < framebufferCount; ++i )
    {
        attachments[ 0U ] = renderer.GetPresentImageView ( i );

        result = android_vulkan::Renderer::CheckVkResult (
            vkCreateFramebuffer ( device, &framebufferInfo, nullptr, &framebuffer ),
            "Game::CreateFramebuffers",
            "Can't create a framebuffer"
        );

        if ( !result )
            return false;

        _framebuffers.push_back ( framebuffer );
        AV_REGISTER_FRAMEBUFFER ( "Game::_framebuffers" )
    }

    return true;
}

void Game::DestroyFramebuffers ( android_vulkan::Renderer &renderer ) noexcept
{
    if ( !_framebuffers.empty () )
    {
        VkDevice device = renderer.GetDevice ();

        for ( auto framebuffer : _framebuffers )
        {
            vkDestroyFramebuffer ( device, framebuffer, nullptr );
            AV_UNREGISTER_FRAMEBUFFER ( "Game::_framebuffers" )
        }

        _framebuffers.clear ();
    }

    _depthStencilRenderTarget.FreeResources ( renderer );
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

    if ( !result )
        return false;

    AV_REGISTER_PIPELINE ( "Game::_pipeline" )
    return true;
}

void Game::DestroyPipeline ( VkDevice device ) noexcept
{
    if ( _pipeline == VK_NULL_HANDLE )
        return;

    vkDestroyPipeline ( device, _pipeline, nullptr );
    _pipeline = VK_NULL_HANDLE;
    AV_UNREGISTER_PIPELINE ( "Game::_pipeline" )
}

void Game::DestroyPipelineLayout ( VkDevice device ) noexcept
{
    if ( _descriptorSetLayout != VK_NULL_HANDLE )
    {
        vkDestroyDescriptorSetLayout ( device, _descriptorSetLayout, nullptr );
        _descriptorSetLayout = VK_NULL_HANDLE;
        AV_UNREGISTER_DESCRIPTOR_SET_LAYOUT ( "Game::_descriptorSetLayout" )
    }

    if ( _pipelineLayout == VK_NULL_HANDLE )
        return;

    vkDestroyPipelineLayout ( device, _pipelineLayout, nullptr );
    _pipelineLayout = VK_NULL_HANDLE;
    AV_UNREGISTER_PIPELINE_LAYOUT ( "Game::_pipelineLayout" )
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

    VkRenderPassCreateInfo const renderPassInfo
    {
        .sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0U,
        .attachmentCount = static_cast<uint32_t> ( std::size ( attachmentInfo ) ),
        .pAttachments = attachmentInfo,
        .subpassCount = 1U,
        .pSubpasses = &subpassInfo,
        .dependencyCount = 0U,
        .pDependencies = nullptr
    };

    bool const result = android_vulkan::Renderer::CheckVkResult (
        vkCreateRenderPass ( renderer.GetDevice (), &renderPassInfo, nullptr, &_renderPass ),
        "Game::CreateRenderPass",
        "Can't create render pass"
    );

    if ( !result )
        return false;

    AV_REGISTER_RENDER_PASS ( "Game::_renderPass" )
    return true;
}

void Game::DestroyRenderPass ( VkDevice device ) noexcept
{
    if ( _renderPass == VK_NULL_HANDLE )
        return;

    vkDestroyRenderPass ( device, _renderPass, nullptr );
    _renderPass = VK_NULL_HANDLE;
    AV_UNREGISTER_RENDER_PASS ( "Game::_renderPass" )
}

bool Game::CreateShaderModules ( android_vulkan::Renderer &renderer ) noexcept
{
    bool result = renderer.CreateShader ( _vertexShaderModule,
        VERTEX_SHADER,
        "Can't create vertex shader (Game::CreateShaderModules)"
    );

    if ( !result )
        return false;

    AV_REGISTER_SHADER_MODULE ( "Game::_vertexShaderModule" )

    result = renderer.CreateShader ( _fragmentShaderModule,
        _fragmentShader,
        "Can't create fragment shader (Game::CreateShaderModules)"
    );

    if ( !result )
        return false;

    AV_REGISTER_SHADER_MODULE ( "Game::_fragmentShaderModule" )
    return true;
}

void Game::DestroyShaderModules ( VkDevice device ) noexcept
{
    if ( _fragmentShaderModule != VK_NULL_HANDLE )
    {
        vkDestroyShaderModule ( device, _fragmentShaderModule, nullptr );
        _fragmentShaderModule = VK_NULL_HANDLE;
        AV_UNREGISTER_SHADER_MODULE ( "Game::_fragmentShaderModule" )
    }

    if ( _vertexShaderModule == VK_NULL_HANDLE )
        return;

    vkDestroyShaderModule ( device, _vertexShaderModule, nullptr );
    _vertexShaderModule = VK_NULL_HANDLE;
    AV_UNREGISTER_SHADER_MODULE ( "Game::_vertexShaderModule" )
}

bool Game::CreateSyncPrimitives ( android_vulkan::Renderer &renderer ) noexcept
{
    VkDevice device = renderer.GetDevice ();

    constexpr VkSemaphoreCreateInfo semaphoreInfo
    {
        .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0U
    };

    bool result = android_vulkan::Renderer::CheckVkResult (
        vkCreateSemaphore ( device, &semaphoreInfo, nullptr, &_renderPassEndSemaphore ),
        "Game::CreateSyncPrimitives",
        "Can't create render pass end semaphore"
    );

    if ( !result )
        return false;

    AV_REGISTER_SEMAPHORE ( "Game::_renderPassEndSemaphore" )

    result = android_vulkan::Renderer::CheckVkResult (
        vkCreateSemaphore ( device, &semaphoreInfo, nullptr, &_renderTargetAcquiredSemaphore ),
        "Game::CreateSyncPrimitives",
        "Can't create render target acquired semaphore"
    );

    if ( !result )
        return false;

    AV_REGISTER_SEMAPHORE ( "Game::_renderTargetAcquiredSemaphore" )
    return true;
}

void Game::DestroySyncPrimitives ( VkDevice device ) noexcept
{
    if ( _renderTargetAcquiredSemaphore != VK_NULL_HANDLE )
    {
        vkDestroySemaphore ( device, _renderTargetAcquiredSemaphore, nullptr );
        _renderTargetAcquiredSemaphore = VK_NULL_HANDLE;
        AV_UNREGISTER_SEMAPHORE ( "Game::_renderTargetAcquiredSemaphore" )
    }

    if ( _renderPassEndSemaphore == VK_NULL_HANDLE )
        return;

    vkDestroySemaphore ( device, _renderPassEndSemaphore, nullptr );
    _renderPassEndSemaphore = VK_NULL_HANDLE;
    AV_UNREGISTER_SEMAPHORE ( "Game::_renderPassEndSemaphore" )
}

bool Game::CreateUniformBuffer ( android_vulkan::Renderer &renderer ) noexcept
{
    if ( !_transformBuffer.Init ( renderer, _commandPool, VK_PIPELINE_STAGE_VERTEX_SHADER_BIT ) )
        return false;

    constexpr VkFenceCreateInfo fenceInfo
    {
        .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0U
    };

    bool result = android_vulkan::Renderer::CheckVkResult (
        vkCreateFence ( renderer.GetDevice (), &fenceInfo, nullptr, &_fence ),
        "rotating_mesh::Game::CreateUniformBuffer",
        "Can't create fence"
    );

    if ( !result )
        return false;

    AV_REGISTER_FENCE ( "rotating_mesh::Game::_fence" )

    _transform._transform = renderer.GetPresentationEngineTransform ();

    result = _transformBuffer.Update ( renderer,
        _fence,
        reinterpret_cast<const uint8_t*> ( &_transform ),
        sizeof ( _transform )
    );

    if ( !result )
        return false;

    VkDevice device = renderer.GetDevice ();

    result = android_vulkan::Renderer::CheckVkResult (
        vkWaitForFences ( device, 1U, &_fence, VK_TRUE, std::numeric_limits<uint64_t>::max () ),
        "pbr::Game::CreateUniformBuffer",
        "Can't wait fence"
    );

    if ( !result )
        return false;

    return android_vulkan::Renderer::CheckVkResult ( vkResetFences ( device, 1U, &_fence ),
        "pbr::Game::CreateUniformBuffer",
        "Can't reset fence"
    );
}

void Game::DestroyUniformBuffer ( android_vulkan::Renderer &renderer ) noexcept
{
    _transformBuffer.FreeResources ( renderer );

    if ( _fence == VK_NULL_HANDLE )
        return;

    vkDestroyFence ( renderer.GetDevice (), _fence, nullptr );
    _fence = VK_NULL_HANDLE;
    AV_UNREGISTER_FENCE ( "rotating_mesh::Game::_fence" )
}

bool Game::CreateCommandBuffers ( android_vulkan::Renderer &renderer ) noexcept
{
    const size_t framebufferCount = renderer.GetPresentImageCount ();

    VkCommandBufferAllocateInfo const allocateInfo
    {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
        .pNext = nullptr,
        .commandPool = _commandPool,
        .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
        .commandBufferCount = static_cast<uint32_t> ( framebufferCount )
    };

    VkDevice device = renderer.GetDevice ();
    std::vector<VkCommandBuffer> commandBuffers ( framebufferCount );

    bool result = android_vulkan::Renderer::CheckVkResult (
        vkAllocateCommandBuffers ( device, &allocateInfo, commandBuffers.data () ),
        "Game::CreateCommandBuffers",
        "Can't allocate command buffers"
    );

    if ( !result )
        return false;

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

    VkRenderPassBeginInfo renderPassBeginInfo;
    renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassBeginInfo.pNext = nullptr;
    renderPassBeginInfo.renderArea.offset.x = 0;
    renderPassBeginInfo.renderArea.offset.y = 0;
    renderPassBeginInfo.renderArea.extent = renderer.GetSurfaceSize ();
    renderPassBeginInfo.renderPass = _renderPass;
    renderPassBeginInfo.clearValueCount = static_cast<uint32_t> ( std::size ( clearValues ) );
    renderPassBeginInfo.pClearValues = clearValues;

    constexpr VkFenceCreateInfo fenceInfo
    {
        .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
        .pNext = nullptr,
        .flags = VK_FENCE_CREATE_SIGNALED_BIT
    };

    VkFence fence = VK_NULL_HANDLE;
    _commandBuffers.reserve ( framebufferCount );

    for ( size_t i = 0U; i < framebufferCount; ++i )
    {
        result = android_vulkan::Renderer::CheckVkResult ( vkCreateFence ( device, &fenceInfo, nullptr, &fence ),
            "Game::CreateCommandBuffers",
            "Can't create fence"
        );

        if ( !result )
            return false;

        AV_REGISTER_FENCE ( "Game::_commandBuffers::_fence" )

        VkCommandBuffer commandBuffer = commandBuffers[ i ];

        result = android_vulkan::Renderer::CheckVkResult ( vkBeginCommandBuffer ( commandBuffer, &bufferBeginInfo ),
            "Game::CreateCommandBuffers",
            "Can't begin command buffer"
        );

        if ( !result )
            return false;

        renderPassBeginInfo.framebuffer = _framebuffers[ i ];
        vkCmdBeginRenderPass ( commandBuffer, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE );

        vkCmdBindPipeline ( commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, _pipeline );
        constexpr VkDeviceSize offset = 0U;

        for ( auto &item : _drawcalls )
        {
            vkCmdBindDescriptorSets ( commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, _pipelineLayout, 0U, 1U,
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

        _commandBuffers.emplace_back ( std::make_pair ( commandBuffer, fence ) );
    }

    return true;
}

void Game::DestroyCommandBuffers ( VkDevice device ) noexcept
{
    if ( _commandBuffers.empty () )
        return;

    for ( auto const &[commandBuffer, fence] : _commandBuffers )
    {
        vkFreeCommandBuffers ( device, _commandPool, 1U, &commandBuffer );
        vkDestroyFence ( device, fence, nullptr );
        AV_UNREGISTER_FENCE ( "Game::_commandBuffers::_fence" )
    }

    _commandBuffers.clear ();
    _commandBuffers.shrink_to_fit ();
}

bool Game::UpdateUniformBuffer ( android_vulkan::Renderer &renderer, double deltaTime ) noexcept
{
    _angle += static_cast<float> ( deltaTime ) * ROTATION_SPEED;
    _transform._normalTransform.RotationY ( _angle );
    _transform._normalTransform.SetOrigin ( GXVec3 ( 0.0F, -1.0F, 3.0F ) );

    GXMat4 tmp {};
    tmp.Multiply ( _transform._normalTransform, _projectionMatrix );
    _transform._transform.Multiply ( tmp, renderer.GetPresentationEngineTransform () );

    bool result = _transformBuffer.Update ( renderer,
        _fence,
        reinterpret_cast<const uint8_t*> ( &_transform ),
        sizeof ( _transform )
    );

    if ( !result )
        return false;

    VkDevice device = renderer.GetDevice ();

    result = android_vulkan::Renderer::CheckVkResult (
        vkWaitForFences ( device, 1U, &_fence, VK_TRUE, std::numeric_limits<uint64_t>::max () ),
        "pbr::Game::UpdateUniformBuffer",
        "Can't wait fence"
    );

    if ( !result )
        return false;

    return android_vulkan::Renderer::CheckVkResult ( vkResetFences ( device, 1U, &_fence ),
        "pbr::Game::UpdateUniformBuffer",
        "Can't reset fence"
    );
}

} // namespace rotating_mesh
