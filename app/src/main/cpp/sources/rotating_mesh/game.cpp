#include <rotating_mesh/game.h>

GX_DISABLE_COMMON_WARNINGS

#include <array>
#include <cassert>
#include <cmath>
#include <thread>

GX_RESTORE_WARNING_STATE

#include <vertex_info.h>
#include <vulkan_utils.h>


namespace rotating_mesh {

constexpr static char const* VERTEX_SHADER = "shaders/static-mesh-vs.spv";
constexpr static char const* VERTEX_SHADER_ENTRY_POINT = "VS";

constexpr static const char* FRAGMENT_SHADER_ENTRY_POINT = "PS";

constexpr static std::string_view const MATERIAL_1_DIFFUSE = "textures/rotating_mesh/sonic-material-1-diffuse.ktx";
constexpr static char const* MATERIAL_1_MESH = "meshes/rotating_mesh/sonic-material-1.mesh";

constexpr static std::string_view const MATERIAL_2_DIFFUSE = "textures/rotating_mesh/sonic-material-2-diffuse.ktx";
constexpr static char const* MATERIAL_2_MESH = "meshes/rotating_mesh/sonic-material-2.mesh";
constexpr static std::string_view const MATERIAL_2_NORMAL = "textures/rotating_mesh/sonic-material-2-normal.png";

constexpr static std::string_view const MATERIAL_3_DIFFUSE = "textures/rotating_mesh/sonic-material-3-diffuse.ktx";
constexpr static char const* MATERIAL_3_MESH = "meshes/rotating_mesh/sonic-material-3.mesh";
constexpr static std::string_view const MATERIAL_3_NORMAL = "textures/rotating_mesh/sonic-material-3-normal.png";

constexpr static float const ROTATION_SPEED = GX_MATH_HALF_PI;
constexpr static float const FIELD_OF_VIEW = 60.0F;
constexpr static float const Z_NEAR = 0.1F;
constexpr static float const Z_FAR = 1.0e+3F;

//----------------------------------------------------------------------------------------------------------------------

Game::Game ( char const* fragmentShader ) noexcept:
    _commandPool ( VK_NULL_HANDLE ),
    _descriptorPool ( VK_NULL_HANDLE ),
    _descriptorSetLayout ( VK_NULL_HANDLE ),
    _drawcalls {},
    _pipelineLayout ( VK_NULL_HANDLE ),
    _transformBuffer {},
    _angle ( 0.0F ),
    _depthStencilRenderTarget {},
    _fragmentShader ( fragmentShader ),
    _framebuffers {},
    _pipeline ( VK_NULL_HANDLE ),
    _renderPass ( VK_NULL_HANDLE ),
    _renderPassEndSemaphore ( VK_NULL_HANDLE ),
    _renderTargetAcquiredSemaphore ( VK_NULL_HANDLE ),
    _sampler01Mips ( VK_NULL_HANDLE ),
    _sampler09Mips ( VK_NULL_HANDLE ),
    _sampler10Mips ( VK_NULL_HANDLE ),
    _sampler11Mips ( VK_NULL_HANDLE ),
    _vertexShaderModule ( VK_NULL_HANDLE ),
    _fragmentShaderModule ( VK_NULL_HANDLE )
{
    // NOTHING
}

bool Game::CreateSamplers ( android_vulkan::Renderer &renderer )
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

void Game::DestroySamplers ( VkDevice device )
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

void Game::DestroyTextures ( VkDevice device )
{
    for ( auto& item : _drawcalls )
    {
        item._diffuse.FreeResources ( device );
        item._normal.FreeResources ( device );
        item._diffuseSampler = item._normalSampler = VK_NULL_HANDLE;
    }
}

bool Game::CreateCommonTextures ( android_vulkan::Renderer &renderer, VkCommandBuffer* commandBuffers )
{
    auto selector = [ this ] ( const android_vulkan::Texture2D &texture ) -> VkSampler {
        const uint8_t mips = texture.GetMipLevelCount ();

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
        auto& drawcall = _drawcalls[ i ];

        bool result = drawcall._diffuse.UploadData ( renderer,
            textureFiles[ i ],
            android_vulkan::eFormat::sRGB,
            true,
            commandBuffers[ i ]
        );

        if ( !result )
            return false;

        drawcall._diffuseSampler = selector ( drawcall._diffuse );
    }

    Drawcall& secondMaterial = _drawcalls[ 1U ];

    bool result = secondMaterial._normal.UploadData ( renderer,
        MATERIAL_2_NORMAL,
        android_vulkan::eFormat::Unorm,
        true,
        commandBuffers[ 3U ]
    );

    if ( !result )
        return false;

    secondMaterial._normalSampler = selector ( secondMaterial._normal );

    Drawcall& thirdMaterial = _drawcalls[ 2U ];

    result = thirdMaterial._normal.UploadData ( renderer,
        MATERIAL_3_NORMAL,
        android_vulkan::eFormat::Unorm,
        true,
        commandBuffers[ 4U ]
    );

    if ( !result )
        return false;

    thirdMaterial._normalSampler = selector ( thirdMaterial._normal );

    Drawcall& firstMaterial = _drawcalls[ 0U ];
    constexpr const uint8_t defaultNormal[] = { 128U, 128U, 255U, 128U };

    result = firstMaterial._normal.UploadData ( renderer,
        defaultNormal,
        std::size ( defaultNormal ),
        VkExtent2D { .width = 1U, .height = 1U },
        VK_FORMAT_R8G8B8A8_UNORM,
        false,
        commandBuffers[ 5U ]
    );

    if ( !result )
        return false;

    firstMaterial._normalSampler = selector ( firstMaterial._normal );
    return true;
}

bool Game::CreateMeshes ( android_vulkan::Renderer &renderer, VkCommandBuffer* commandBuffers )
{
    constexpr const char* meshFiles[ MATERIAL_COUNT ] =
        {
            MATERIAL_1_MESH,
            MATERIAL_2_MESH,
            MATERIAL_3_MESH,
        };

    for ( size_t i = 0U; i < MATERIAL_COUNT; ++i )
    {
        const bool result = _drawcalls[ i ]._mesh.LoadMesh ( meshFiles[ i ],
            VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
            renderer, commandBuffers[ i ]
        );

        if ( result )
            continue;

        return false;
    }

    return true;
}

void Game::InitDescriptorPoolSizeCommon ( VkDescriptorPoolSize* features )
{
    VkDescriptorPoolSize& ubFeature = features[ 0U ];
    ubFeature.descriptorCount = static_cast<uint32_t> ( MATERIAL_COUNT );
    ubFeature.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;

    VkDescriptorPoolSize& diffuseTextureFeature = features[ 1U ];
    diffuseTextureFeature.descriptorCount = static_cast<uint32_t> ( MATERIAL_COUNT * 2U );
    diffuseTextureFeature.type = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;

    VkDescriptorPoolSize& diffuseSamplerFeature = features[ 2U ];
    diffuseSamplerFeature.descriptorCount = static_cast<uint32_t> ( MATERIAL_COUNT * 2U );
    diffuseSamplerFeature.type = VK_DESCRIPTOR_TYPE_SAMPLER;
}

void Game::InitDescriptorSetLayoutBindingCommon ( VkDescriptorSetLayoutBinding* bindings )
{
    VkDescriptorSetLayoutBinding& ubInfo = bindings[ 0U ];
    ubInfo.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
    ubInfo.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    ubInfo.descriptorCount = 1U;
    ubInfo.binding = 0U;
    ubInfo.pImmutableSamplers = nullptr;

    VkDescriptorSetLayoutBinding& diffuseImageInfo = bindings[ 1U ];
    diffuseImageInfo.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
    diffuseImageInfo.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
    diffuseImageInfo.descriptorCount = 1U;
    diffuseImageInfo.binding = 1U;
    diffuseImageInfo.pImmutableSamplers = nullptr;

    VkDescriptorSetLayoutBinding& diffuseSamplerInfo = bindings[ 2U ];
    diffuseSamplerInfo.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
    diffuseSamplerInfo.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLER;
    diffuseSamplerInfo.descriptorCount = 1U;
    diffuseSamplerInfo.binding = 2U;
    diffuseSamplerInfo.pImmutableSamplers = nullptr;

    VkDescriptorSetLayoutBinding& normalImageInfo = bindings[ 3U ];
    normalImageInfo.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
    normalImageInfo.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
    normalImageInfo.descriptorCount = 1U;
    normalImageInfo.binding = 3U;
    normalImageInfo.pImmutableSamplers = nullptr;

    VkDescriptorSetLayoutBinding& normalSamplerInfo = bindings[ 4U ];
    normalSamplerInfo.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
    normalSamplerInfo.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLER;
    normalSamplerInfo.descriptorCount = 1U;
    normalSamplerInfo.binding = 4U;
    normalSamplerInfo.pImmutableSamplers = nullptr;
}

bool Game::IsReady ()
{
    return _pipeline != VK_NULL_HANDLE;
}

bool Game::OnFrame ( android_vulkan::Renderer &renderer, double deltaTime )
{
    if ( !UpdateUniformBuffer ( renderer, deltaTime ) )
        return false;

    size_t imageIndex = SIZE_MAX;

    if ( !BeginFrame ( renderer, imageIndex ) )
        return false;

    CommandContext const& commandContext = _commandBuffers[ imageIndex ];

    constexpr VkPipelineStageFlags const waitStage =
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

bool Game::OnInitDevice ( android_vulkan::Renderer &renderer )
{
    if ( !CreateSyncPrimitives ( renderer ) )
    {
        OnDestroyDevice ( renderer.GetDevice () );
        return false;
    }

    if ( !CreateCommandPool ( renderer ) )
    {
        OnDestroyDevice ( renderer.GetDevice () );
        return false;
    }

    if ( !CreateUniformBuffer ( renderer ) )
    {
        OnDestroyDevice ( renderer.GetDevice () );
        return false;
    }

    if ( !CreateSamplers ( renderer ) )
    {
        OnDestroyDevice ( renderer.GetDevice () );
        return false;
    }

    if ( !LoadGPUContent ( renderer ) )
    {
        OnDestroyDevice ( renderer.GetDevice () );
        return false;
    }

    if ( !CreateShaderModules ( renderer ) )
    {
        OnDestroyDevice ( renderer.GetDevice () );
        return false;
    }

    if ( !CreatePipelineLayout ( renderer ) )
    {
        OnDestroyDevice ( renderer.GetDevice () );
        return false;
    }

    if ( !CreateDescriptorSet ( renderer ) )
    {
        OnDestroyDevice ( renderer.GetDevice () );
        return false;
    }

    return true;
}

void Game::OnDestroyDevice ( VkDevice device )
{
    DestroyDescriptorSet ( device );
    DestroyPipelineLayout ( device );
    DestroyShaderModules ( device );
    DestroySamplers ( device );
    DestroyMeshes ( device );
    DestroyTextures ( device );
    DestroyUniformBuffer ( device );
    DestroyCommandPool ( device );
    DestroySyncPrimitives ( device );
}

bool Game::OnSwapchainCreated ( android_vulkan::Renderer &renderer )
{
    VkExtent2D const& resolution = renderer.GetViewportResolution ();

    _projectionMatrix.Perspective ( GXDegToRad ( FIELD_OF_VIEW ),
        resolution.width / static_cast<float> ( resolution.height ),
        Z_NEAR,
        Z_FAR
    );

    if ( !CreateRenderPass ( renderer ) )
    {
        OnSwapchainDestroyed ( renderer.GetDevice () );
        return false;
    }

    if ( !CreateFramebuffers ( renderer ) )
    {
        OnSwapchainDestroyed ( renderer.GetDevice () );
        return false;
    }

    if ( !CreatePipeline ( renderer ) )
    {
        OnSwapchainDestroyed ( renderer.GetDevice () );
        return false;
    }

    if ( !CreateCommandBuffers ( renderer ) )
    {
        OnSwapchainDestroyed ( renderer.GetDevice () );
        return false;
    }

    return true;
}

void Game::OnSwapchainDestroyed ( VkDevice device )
{
    DestroyCommandBuffers ( device );
    DestroyPipeline ( device );
    DestroyFramebuffers ( device );
    DestroyRenderPass ( device );
}

bool Game::BeginFrame ( android_vulkan::Renderer &renderer, size_t &imageIndex )
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
    const CommandContext& commandContext = _commandBuffers[ imageIndex ];

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

bool Game::EndFrame ( android_vulkan::Renderer &renderer, uint32_t presentationImageIndex )
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

    const bool result = android_vulkan::Renderer::CheckVkResult (
        vkQueuePresentKHR ( renderer.GetQueue (), &presentInfo ),
        "Game::EndFrame",
        "Can't present frame"
    );

    if ( !result )
        return false;

    return android_vulkan::Renderer::CheckVkResult ( presentResult, "Game::EndFrame", "Present queue has been failed" );
}

bool Game::CreateCommandPool ( android_vulkan::Renderer &renderer )
{
    VkCommandPoolCreateInfo const poolInfo
    {
        .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
        .pNext = nullptr,
        .flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
        .queueFamilyIndex = renderer.GetQueueFamilyIndex ()
    };

    const bool result = android_vulkan::Renderer::CheckVkResult (
        vkCreateCommandPool ( renderer.GetDevice (), &poolInfo, nullptr, &_commandPool ),
        "Game::CreateCommandPool",
        "Can't create command pool"
    );

    if ( !result )
        return false;

    AV_REGISTER_COMMAND_POOL ( "Game::_commandPool" )
    return true;
}

void Game::DestroyCommandPool ( VkDevice device )
{
    if ( !_commandBuffers.empty () )
    {
        for ( const auto& item : _commandBuffers )
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

void Game::DestroyDescriptorSet ( VkDevice device )
{
    if ( _descriptorPool == VK_NULL_HANDLE )
        return;

    vkDestroyDescriptorPool ( device, _descriptorPool, nullptr );
    _descriptorPool = VK_NULL_HANDLE;
    AV_UNREGISTER_DESCRIPTOR_POOL ( "Game::_descriptorPool" )
}

void Game::DestroyMeshes ( VkDevice device )
{
    for ( auto& item : _drawcalls )
    {
        item._mesh.FreeResources ( device );
    }
}

bool Game::CreateFramebuffers ( android_vulkan::Renderer &renderer )
{
    VkDevice device = renderer.GetDevice ();
    const VkExtent2D& resolution = renderer.GetSurfaceSize ();

    bool result = _depthStencilRenderTarget.CreateRenderTarget ( resolution,
        renderer.GetDefaultDepthStencilFormat (),
        VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
        renderer
    );

    if ( !result )
        return false;

    const size_t framebufferCount = renderer.GetPresentImageCount ();
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

void Game::DestroyFramebuffers ( VkDevice device )
{
    if ( !_framebuffers.empty () )
    {
        for ( auto framebuffer : _framebuffers )
        {
            vkDestroyFramebuffer ( device, framebuffer, nullptr );
            AV_UNREGISTER_FRAMEBUFFER ( "Game::_framebuffers" )
        }

        _framebuffers.clear ();
    }

    _depthStencilRenderTarget.FreeResources ( device );
}

bool Game::CreatePipeline ( android_vulkan::Renderer &renderer )
{
    VkPipelineShaderStageCreateInfo stageInfo[ 2U ];
    VkPipelineShaderStageCreateInfo& vertexStage = stageInfo[ 0U ];
    vertexStage.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    vertexStage.pNext = nullptr;
    vertexStage.flags = 0U;
    vertexStage.pSpecializationInfo = nullptr;
    vertexStage.stage = VK_SHADER_STAGE_VERTEX_BIT;
    vertexStage.module = _vertexShaderModule;
    vertexStage.pName = VERTEX_SHADER_ENTRY_POINT;

    VkPipelineShaderStageCreateInfo& fragmentStage = stageInfo[ 1U ];
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
    VkVertexInputAttributeDescription& vertexDescription = attributeDescriptions[ 0U ];
    vertexDescription.location = 0U;
    vertexDescription.binding = 0U;
    vertexDescription.offset = static_cast<uint32_t> ( offsetof ( android_vulkan::VertexInfo, _vertex ) );
    vertexDescription.format = VK_FORMAT_R32G32B32_SFLOAT;

    VkVertexInputAttributeDescription& uvDescription = attributeDescriptions[ 1U ];
    uvDescription.location = 1U;
    uvDescription.binding = 0U;
    uvDescription.offset = static_cast<uint32_t> ( offsetof ( android_vulkan::VertexInfo, _uv ) );
    uvDescription.format = VK_FORMAT_R32G32_SFLOAT;

    VkVertexInputAttributeDescription& normalDescription = attributeDescriptions[ 2U ];
    normalDescription.location = 2U;
    normalDescription.binding = 0U;
    normalDescription.offset = static_cast<uint32_t> ( offsetof ( android_vulkan::VertexInfo, _normal ) );
    normalDescription.format = VK_FORMAT_R32G32B32_SFLOAT;

    VkVertexInputAttributeDescription& tangentDescription = attributeDescriptions[ 3U ];
    tangentDescription.location = 3U;
    tangentDescription.binding = 0U;
    tangentDescription.offset = static_cast<uint32_t> ( offsetof ( android_vulkan::VertexInfo, _tangent ) );
    tangentDescription.format = VK_FORMAT_R32G32B32_SFLOAT;

    VkVertexInputAttributeDescription& bitangentDescription = attributeDescriptions[ 4U ];
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
    depthStencilInfo.depthCompareOp = VK_COMPARE_OP_LESS;
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
    memset ( blendInfo.blendConstants, 0, sizeof ( blendInfo.blendConstants ) );

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

    VkExtent2D const& surfaceSize = renderer.GetSurfaceSize ();

    VkRect2D scissor;
    scissor.extent = surfaceSize;
    memset ( &scissor.offset, 0, sizeof ( scissor.offset ) );

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

    VkGraphicsPipelineCreateInfo pipelineInfo;
    pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipelineInfo.pNext = nullptr;
    pipelineInfo.flags = 0U;
    pipelineInfo.subpass = 0U;
    pipelineInfo.stageCount = static_cast<uint32_t> ( std::size ( stageInfo ) );
    pipelineInfo.pStages = stageInfo;
    pipelineInfo.renderPass = _renderPass;
    pipelineInfo.pDynamicState = nullptr;
    pipelineInfo.layout = _pipelineLayout;
    pipelineInfo.basePipelineIndex = 0;
    pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;
    pipelineInfo.pInputAssemblyState = &assemblyInfo;
    pipelineInfo.pVertexInputState = &vertexInputInfo;
    pipelineInfo.pTessellationState = nullptr;
    pipelineInfo.pDepthStencilState = &depthStencilInfo;
    pipelineInfo.pRasterizationState = &rasterizationInfo;
    pipelineInfo.pViewportState = &viewportInfo;
    pipelineInfo.pColorBlendState = &blendInfo;
    pipelineInfo.pMultisampleState = &multisampleInfo;

    const bool result = android_vulkan::Renderer::CheckVkResult (
        vkCreateGraphicsPipelines ( renderer.GetDevice (), VK_NULL_HANDLE, 1U, &pipelineInfo, nullptr, &_pipeline ),
        "Game::CreatePipeline",
        "Can't create pipeline"
    );

    if ( !result )
        return false;

    AV_REGISTER_PIPELINE ( "Game::_pipeline" )
    return true;
}

void Game::DestroyPipeline ( VkDevice device )
{
    if ( _pipeline == VK_NULL_HANDLE )
        return;

    vkDestroyPipeline ( device, _pipeline, nullptr );
    _pipeline = VK_NULL_HANDLE;
    AV_UNREGISTER_PIPELINE ( "Game::_pipeline" )
}

void Game::DestroyPipelineLayout ( VkDevice device )
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

bool Game::CreateRenderPass ( android_vulkan::Renderer &renderer )
{
    VkAttachmentDescription attachmentInfo[ 2U ];
    VkAttachmentDescription& colorAttachment = attachmentInfo[ 0U ];
    colorAttachment.format = renderer.GetSurfaceFormat ();
    colorAttachment.flags = 0U;
    colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
    colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
    colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;

    VkAttachmentDescription& depthStencilAttachment = attachmentInfo[ 1U ];
    depthStencilAttachment.format = renderer.GetDefaultDepthStencilFormat ();
    depthStencilAttachment.flags = 0U;
    depthStencilAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
    depthStencilAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    depthStencilAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
    depthStencilAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    depthStencilAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    depthStencilAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    depthStencilAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;

    VkAttachmentReference colorReference;
    colorReference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    colorReference.attachment = 0U;

    VkAttachmentReference depthStencilReference;
    depthStencilReference.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
    depthStencilReference.attachment = 1U;

    VkSubpassDescription subpassInfo;
    subpassInfo.flags = 0U;
    subpassInfo.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpassInfo.colorAttachmentCount = 1U;
    subpassInfo.pColorAttachments = &colorReference;
    subpassInfo.pDepthStencilAttachment = &depthStencilReference;
    subpassInfo.preserveAttachmentCount = 0U;
    subpassInfo.pPreserveAttachments = nullptr;
    subpassInfo.inputAttachmentCount = 0U;
    subpassInfo.pInputAttachments = nullptr;
    subpassInfo.pResolveAttachments = nullptr;

    VkRenderPassCreateInfo renderPassInfo;
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassInfo.pNext = nullptr;
    renderPassInfo.flags = 0U;
    renderPassInfo.attachmentCount = static_cast<uint32_t> ( std::size ( attachmentInfo ) );
    renderPassInfo.pAttachments = attachmentInfo;
    renderPassInfo.dependencyCount = 0U;
    renderPassInfo.pDependencies = nullptr;
    renderPassInfo.subpassCount = 1U;
    renderPassInfo.pSubpasses = &subpassInfo;

    const bool result = android_vulkan::Renderer::CheckVkResult (
        vkCreateRenderPass ( renderer.GetDevice (), &renderPassInfo, nullptr, &_renderPass ),
        "Game::CreateRenderPass",
        "Can't create render pass"
    );

    if ( !result )
        return false;

    AV_REGISTER_RENDER_PASS ( "Game::_renderPass" )
    return true;
}

void Game::DestroyRenderPass ( VkDevice device )
{
    if ( _renderPass == VK_NULL_HANDLE )
        return;

    vkDestroyRenderPass ( device, _renderPass, nullptr );
    _renderPass = VK_NULL_HANDLE;
    AV_UNREGISTER_RENDER_PASS ( "Game::_renderPass" )
}

bool Game::CreateShaderModules ( android_vulkan::Renderer &renderer )
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

void Game::DestroyShaderModules ( VkDevice device )
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

bool Game::CreateSyncPrimitives ( android_vulkan::Renderer &renderer )
{
    VkDevice device = renderer.GetDevice ();

    VkSemaphoreCreateInfo semaphoreInfo;
    semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
    semaphoreInfo.pNext = nullptr;
    semaphoreInfo.flags = 0U;

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

void Game::DestroySyncPrimitives ( VkDevice device )
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

bool Game::CreateUniformBuffer ( android_vulkan::Renderer& renderer )
{
    if ( !_transformBuffer.Init ( renderer, _commandPool, VK_PIPELINE_STAGE_VERTEX_SHADER_BIT ) )
        return false;

    _transform._transform = renderer.GetPresentationEngineTransform ();

    return _transformBuffer.Update ( renderer,
        reinterpret_cast<const uint8_t*> ( &_transform ),
        sizeof ( _transform )
    );
}

void Game::DestroyUniformBuffer ( VkDevice device )
{
    _transformBuffer.FreeResources ( device );
}

bool Game::CreateCommandBuffers ( android_vulkan::Renderer &renderer )
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

    constexpr VkCommandBufferBeginInfo const bufferBeginInfo
    {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
        .pNext = nullptr,
        .flags = 0U,
        .pInheritanceInfo = nullptr
    };

    VkClearValue clearValues[ 2U ];
    VkClearValue& colorTarget = clearValues[ 0U ];
    memset ( &colorTarget.color, 0, sizeof ( colorTarget.color ) );

    VkClearValue& depthStencilTarget = clearValues[ 1U ];
    depthStencilTarget.depthStencil.depth = 1.0F;
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

    constexpr VkFenceCreateInfo const fenceInfo
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
        constexpr VkDeviceSize const offset = 0U;

        for ( auto& item : _drawcalls )
        {
            vkCmdBindDescriptorSets ( commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, _pipelineLayout, 0U, 1U,
                &item._descriptorSet, 0U, nullptr
            );

            android_vulkan::MeshGeometry& mesh = item._mesh;

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

void Game::DestroyCommandBuffers ( VkDevice device )
{
    if ( _commandBuffers.empty () )
        return;

    for ( auto const& [commandBuffer, fence] : _commandBuffers )
    {
        vkFreeCommandBuffers ( device, _commandPool, 1U, &commandBuffer );

        vkDestroyFence ( device, fence, nullptr );
        AV_UNREGISTER_FENCE ( "Game::_commandBuffers::_fence" )
    }

    _commandBuffers.clear ();
    _commandBuffers.shrink_to_fit ();
}

bool Game::UpdateUniformBuffer ( android_vulkan::Renderer &renderer, double deltaTime )
{
    _angle += static_cast<float> ( deltaTime ) * ROTATION_SPEED;

    _transform._normalTransform.RotationY ( _angle );
    _transform._normalTransform.SetOrigin ( GXVec3 ( 0.0F, -1.0F, 3.0F ) );

    GXMat4 tmp;
    tmp.Multiply ( _transform._normalTransform, _projectionMatrix );
    _transform._transform.Multiply ( tmp, renderer.GetPresentationEngineTransform () );

    return _transformBuffer.Update ( renderer,
        reinterpret_cast<const uint8_t*> ( &_transform ),
        sizeof ( _transform )
    );
}

} // namespace rotating_mesh
