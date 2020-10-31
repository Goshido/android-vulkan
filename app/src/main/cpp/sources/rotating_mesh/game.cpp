#include <rotating_mesh/game.h>

GX_DISABLE_COMMON_WARNINGS

#include <array>
#include <cassert>
#include <cmath>
#include <thread>

GX_RESTORE_WARNING_STATE

#include <vulkan_utils.h>
#include <vertex_info.h>


namespace rotating_mesh {

constexpr static const char* VERTEX_SHADER = "shaders/static-mesh-vs.spv";
constexpr static const char* VERTEX_SHADER_ENTRY_POINT = "VS";

constexpr static const char* FRAGMENT_SHADER_ENTRY_POINT = "PS";

constexpr static std::string_view const MATERIAL_1_DIFFUSE = "textures/rotating_mesh/sonic-material-1-diffuse.ktx";
constexpr static const char* MATERIAL_1_MESH = "meshes/rotating_mesh/sonic-material-1.mesh";

constexpr static std::string_view const MATERIAL_2_DIFFUSE = "textures/rotating_mesh/sonic-material-2-diffuse.ktx";
constexpr static const char* MATERIAL_2_MESH = "meshes/rotating_mesh/sonic-material-2.mesh";
constexpr static std::string_view const MATERIAL_2_NORMAL = "textures/rotating_mesh/sonic-material-2-normal.png";

constexpr static std::string_view const MATERIAL_3_DIFFUSE = "textures/rotating_mesh/sonic-material-3-diffuse.ktx";
constexpr static const char* MATERIAL_3_MESH = "meshes/rotating_mesh/sonic-material-3.mesh";
constexpr static std::string_view const MATERIAL_3_NORMAL = "textures/rotating_mesh/sonic-material-3-normal.png";

constexpr static const float ROTATION_SPEED = GX_MATH_HALF_PI;
constexpr static const float FIELD_OF_VIEW = 60.0F;
constexpr static const float Z_NEAR = 0.1F;
constexpr static const float Z_FAR = 1.0e+3F;

//----------------------------------------------------------------------------------------------------------------------

Game::Game ( const char* fragmentShader ):
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
    VkSamplerCreateInfo samplerInfo;
    samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    samplerInfo.pNext = nullptr;
    samplerInfo.flags = 0U;
    samplerInfo.unnormalizedCoordinates = VK_FALSE;
    samplerInfo.borderColor = VK_BORDER_COLOR_FLOAT_TRANSPARENT_BLACK;
    samplerInfo.mipLodBias = 0.0F;
    samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
    samplerInfo.minLod = 0.0F;
    samplerInfo.maxLod = 8.0F;
    samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.magFilter = VK_FILTER_LINEAR;
    samplerInfo.minFilter = VK_FILTER_LINEAR;
    samplerInfo.anisotropyEnable = VK_FALSE;
    samplerInfo.maxAnisotropy = 1.0F;
    samplerInfo.compareEnable = VK_FALSE;
    samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;

    VkDevice device = renderer.GetDevice ();

    bool result = renderer.CheckVkResult (
        vkCreateSampler ( device, &samplerInfo, nullptr, &_sampler09Mips ),
        "Game::CreateSamplers",
        "Can't create sampler with 9 mips"
    );

    if ( !result )
        return false;

    AV_REGISTER_SAMPLER ( "Game::_sampler09Mips" )

    samplerInfo.maxLod = 9.0F;

    result = renderer.CheckVkResult (
        vkCreateSampler ( device, &samplerInfo, nullptr, &_sampler10Mips ),
        "Game::CreateSamplers",
        "Can't create sampler with 10 mips"
    );

    if ( !result )
        return false;

    AV_REGISTER_SAMPLER ( "Game::_sampler10Mips" )

    samplerInfo.maxLod = 10.0F;

    result = renderer.CheckVkResult (
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

    result = renderer.CheckVkResult (
        vkCreateSampler ( device, &samplerInfo, nullptr, &_sampler01Mips ),
        "Game::CreateSamplers",
        "Can't create sampler with 1 mip"
    );

    if ( !result )
        return false;

    AV_REGISTER_SAMPLER ( "Game::_sampler01Mips" )
    return true;
}

void Game::DestroySamplers ( android_vulkan::Renderer &renderer )
{
    if ( _sampler11Mips != VK_NULL_HANDLE )
    {
        vkDestroySampler ( renderer.GetDevice (), _sampler11Mips, nullptr );
        _sampler11Mips = VK_NULL_HANDLE;
        AV_UNREGISTER_SAMPLER ( "Game::_sampler11Mips" )
    }

    if ( _sampler10Mips != VK_NULL_HANDLE )
    {
        vkDestroySampler ( renderer.GetDevice (), _sampler10Mips, nullptr );
        _sampler10Mips = VK_NULL_HANDLE;
        AV_UNREGISTER_SAMPLER ( "Game::_sampler10Mips" )
    }

    if ( _sampler09Mips != VK_NULL_HANDLE )
    {
        vkDestroySampler ( renderer.GetDevice (), _sampler09Mips, nullptr );
        _sampler09Mips = VK_NULL_HANDLE;
        AV_UNREGISTER_SAMPLER ( "Game::_sampler09Mips" )
    }

    if ( _sampler01Mips == VK_NULL_HANDLE )
        return;

    vkDestroySampler ( renderer.GetDevice (), _sampler01Mips, nullptr );
    _sampler01Mips = VK_NULL_HANDLE;
    AV_UNREGISTER_SAMPLER ( "Game::_sampler01Mips" )
}

void Game::DestroyTextures ( android_vulkan::Renderer &renderer )
{
    renderer.CheckVkResult ( vkQueueWaitIdle ( renderer.GetQueue () ),
        "Game::DestroyTextures",
        "Can't wait queue idle"
    );

    for ( auto& item : _drawcalls )
    {
        item._diffuse.FreeResources ( renderer );
        item._normal.FreeResources ( renderer );
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

        assert ( !"Game::CreateCommonTextures::selector - Can't select sampler" );
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

        bool result = drawcall._diffuse.UploadData ( textureFiles[ i ],
            android_vulkan::eFormat::sRGB,
            true,
            renderer,
            commandBuffers[ i ]
        );

        if ( !result )
            return false;

        drawcall._diffuseSampler = selector ( drawcall._diffuse );
    }

    Drawcall& secondMaterial = _drawcalls[ 1U ];

    bool result = secondMaterial._normal.UploadData ( MATERIAL_2_NORMAL,
        android_vulkan::eFormat::Unorm,
        true,
        renderer,
        commandBuffers[ 3U ]
    );

    if ( !result )
        return false;

    secondMaterial._normalSampler = selector ( secondMaterial._normal );

    Drawcall& thirdMaterial = _drawcalls[ 2U ];

    result = thirdMaterial._normal.UploadData ( MATERIAL_3_NORMAL,
        android_vulkan::eFormat::Unorm,
        true,
        renderer,
        commandBuffers[ 4U ]
    );

    if ( !result )
        return false;

    thirdMaterial._normalSampler = selector ( thirdMaterial._normal );

    Drawcall& firstMaterial = _drawcalls[ 0U ];
    constexpr const uint8_t defaultNormal[] = { 128U, 128U, 255U, 128U };

    result = firstMaterial._normal.UploadData ( defaultNormal,
        std::size ( defaultNormal ),
        VkExtent2D { .width = 1U, .height = 1U },
        VK_FORMAT_R8G8B8A8_UNORM,
        false,
        renderer,
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

bool Game::OnInit ( android_vulkan::Renderer &renderer )
{
    const VkExtent2D& resolution = renderer.GetViewportResolution ();

    _projectionMatrix.Perspective ( GXDegToRad ( FIELD_OF_VIEW ),
        resolution.width / static_cast<float> ( resolution.height ),
        Z_NEAR,
        Z_FAR
    );

    if ( !CreateRenderPass ( renderer ) )
    {
        OnDestroy ( renderer );
        return false;
    }

    if ( !CreateFramebuffers ( renderer ) )
    {
        OnDestroy ( renderer );
        return false;
    }

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

    if ( !CreateUniformBuffer ( renderer ) )
    {
        OnDestroy ( renderer );
        return false;
    }

    if ( !CreateSamplers ( renderer ) )
    {
        OnDestroy ( renderer );
        return false;
    }

    if ( !LoadGPUContent ( renderer ) )
    {
        OnDestroy ( renderer );
        return false;
    }

    if ( !CreateShaderModules ( renderer ) )
    {
        OnDestroy ( renderer );
        return false;
    }

    if ( !CreatePipelineLayout ( renderer ) )
    {
        OnDestroy ( renderer );
        return false;
    }

    if ( !CreatePipeline ( renderer ) )
    {
        OnDestroy ( renderer );
        return false;
    }

    if ( !CreateDescriptorSet ( renderer ) )
    {
        OnDestroy ( renderer );
        return false;
    }

    if ( !InitCommandBuffers ( renderer ) )
    {
        OnDestroy ( renderer );
        return false;
    }

    return true;
}

bool Game::OnFrame ( android_vulkan::Renderer &renderer, double deltaTime )
{
    if ( !UpdateUniformBuffer ( renderer, deltaTime ) )
        return false;

    size_t imageIndex = SIZE_MAX;

    if ( !BeginFrame ( imageIndex, renderer ) )
        return false;

    const CommandContext& commandContext = _commandBuffers[ imageIndex ];

    constexpr const VkPipelineStageFlags waitStage =
        AV_VK_FLAG ( VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT ) |
        AV_VK_FLAG ( VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT ) |
        AV_VK_FLAG ( VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT );

    VkSubmitInfo submitInfo;
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.pNext = nullptr;
    submitInfo.commandBufferCount = 1U;
    submitInfo.pCommandBuffers = &commandContext.first;
    submitInfo.waitSemaphoreCount = 1U;
    submitInfo.pWaitDstStageMask = &waitStage;
    submitInfo.pWaitSemaphores = &_renderTargetAcquiredSemaphore;
    submitInfo.signalSemaphoreCount = 1U;
    submitInfo.pSignalSemaphores = &_renderPassEndSemaphore;

    const bool result = renderer.CheckVkResult (
        vkQueueSubmit ( renderer.GetQueue (), 1U, &submitInfo, commandContext.second ),
        "Game::OnFrame",
        "Can't submit command buffer"
    );

    if ( !result )
        return false;

    return EndFrame ( static_cast<size_t> ( imageIndex ), renderer );
}

bool Game::OnDestroy ( android_vulkan::Renderer &renderer )
{
    const bool result = renderer.CheckVkResult ( vkQueueWaitIdle ( renderer.GetQueue () ),
        "Game::OnDestroy",
        "Can't wait queue idle"
    );

    if ( !result )
        return false;

    DestroyDescriptorSet ( renderer );
    DestroyPipeline ( renderer );
    DestroyPipelineLayout ( renderer );
    DestroyShaderModules ( renderer );
    DestroySamplers ( renderer );
    DestroyMeshes ( renderer );
    DestroyTextures ( renderer );
    DestroyUniformBuffer ();
    DestroyCommandPool ( renderer );
    DestroySyncPrimitives ( renderer );
    DestroyFramebuffers ( renderer );
    DestroyRenderPass ( renderer );

    return true;
}

bool Game::BeginFrame ( size_t &imageIndex, android_vulkan::Renderer &renderer )
{
    VkDevice device = renderer.GetDevice ();
    uint32_t i = UINT32_MAX;

    bool result = renderer.CheckVkResult (
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

    result = renderer.CheckVkResult ( vkWaitForFences ( device, 1U, &commandContext.second, VK_TRUE, UINT64_MAX ),
        "Game::BeginFrame",
        "Can't wait fence"
    );

    if ( !result )
        return false;

    return renderer.CheckVkResult ( vkResetFences ( device, 1U, &commandContext.second ),
        "Game::BeginFrame",
        "Can't reset fence"
    );
}

bool Game::EndFrame ( uint32_t presentationImageIndex, android_vulkan::Renderer &renderer )
{
    VkResult presentResult = VK_ERROR_DEVICE_LOST;

    VkPresentInfoKHR presentInfo;
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    presentInfo.pNext = nullptr;
    presentInfo.waitSemaphoreCount = 1U;
    presentInfo.pWaitSemaphores = &_renderPassEndSemaphore;
    presentInfo.pResults = &presentResult;
    presentInfo.swapchainCount = 1U;
    presentInfo.pSwapchains = &renderer.GetSwapchain ();
    presentInfo.pImageIndices = &presentationImageIndex;

    const bool result = renderer.CheckVkResult ( vkQueuePresentKHR ( renderer.GetQueue (), &presentInfo ),
        "Game::EndFrame",
        "Can't present frame"
    );

    if ( !result )
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
    VkDevice device = renderer.GetDevice ();

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

void Game::DestroyDescriptorSet ( android_vulkan::Renderer &renderer )
{
    if ( _descriptorPool == VK_NULL_HANDLE )
        return;

    vkDestroyDescriptorPool ( renderer.GetDevice (), _descriptorPool, nullptr );
    _descriptorPool = VK_NULL_HANDLE;
    AV_UNREGISTER_DESCRIPTOR_POOL ( "Game::_descriptorPool" )
}

void Game::DestroyMeshes ( android_vulkan::Renderer &renderer )
{
    for ( auto& item : _drawcalls )
    {
        item._mesh.FreeResources ( renderer );
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

        result = renderer.CheckVkResult ( vkCreateFramebuffer ( device, &framebufferInfo, nullptr, &framebuffer ),
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

void Game::DestroyFramebuffers ( android_vulkan::Renderer &renderer )
{
    VkDevice device = renderer.GetDevice ();

    if ( !_framebuffers.empty () )
    {
        for ( auto framebuffer : _framebuffers )
        {
            vkDestroyFramebuffer ( device, framebuffer, nullptr );
            AV_UNREGISTER_FRAMEBUFFER ( "Game::_framebuffers" )
        }

        _framebuffers.clear ();
    }

    _depthStencilRenderTarget.FreeResources ( renderer );
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

    const VkExtent2D& surfaceSize = renderer.GetSurfaceSize ();

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

    const bool result = renderer.CheckVkResult (
        vkCreateGraphicsPipelines ( renderer.GetDevice (), VK_NULL_HANDLE, 1U, &pipelineInfo, nullptr, &_pipeline ),
        "Game::CreatePipeline",
        "Can't create pipeline"
    );

    if ( !result )
        return false;

    AV_REGISTER_PIPELINE ( "Game::_pipeline" )
    return true;
}

void Game::DestroyPipeline ( android_vulkan::Renderer &renderer )
{
    if ( _pipeline == VK_NULL_HANDLE )
        return;

    vkDestroyPipeline ( renderer.GetDevice (), _pipeline, nullptr );
    _pipeline = VK_NULL_HANDLE;
    AV_UNREGISTER_PIPELINE ( "Game::_pipeline" )
}

void Game::DestroyPipelineLayout ( android_vulkan::Renderer &renderer )
{
    VkDevice device = renderer.GetDevice ();

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

    const bool result = renderer.CheckVkResult (
        vkCreateRenderPass ( renderer.GetDevice (), &renderPassInfo, nullptr, &_renderPass ),
        "Game::CreateRenderPass",
        "Can't create render pass"
    );

    if ( !result )
        return false;

    AV_REGISTER_RENDER_PASS ( "Game::_renderPass" )
    return true;
}

void Game::DestroyRenderPass ( android_vulkan::Renderer &renderer )
{
    if ( _renderPass == VK_NULL_HANDLE )
        return;

    vkDestroyRenderPass ( renderer.GetDevice (), _renderPass, nullptr );
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

void Game::DestroyShaderModules ( android_vulkan::Renderer &renderer )
{
    VkDevice device = renderer.GetDevice ();

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

    bool result = renderer.CheckVkResult ( vkCreateSemaphore ( device, &semaphoreInfo, nullptr, &_renderPassEndSemaphore ),
        "Game::CreateSyncPrimitives",
        "Can't create render pass end semaphore"
    );

    if ( !result )
        return false;

    AV_REGISTER_SEMAPHORE ( "Game::_renderPassEndSemaphore" )

    result = renderer.CheckVkResult (
        vkCreateSemaphore ( device, &semaphoreInfo, nullptr, &_renderTargetAcquiredSemaphore ),
        "Game::CreateSyncPrimitives",
        "Can't create render target acquired semaphore"
    );

    if ( !result )
        return false;

    AV_REGISTER_SEMAPHORE ( "Game::_renderTargetAcquiredSemaphore" )
    return true;
}

void Game::DestroySyncPrimitives ( android_vulkan::Renderer &renderer )
{
    VkDevice device = renderer.GetDevice ();

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
    return _transformBuffer.Update ( reinterpret_cast<const uint8_t*> ( &_transform ), sizeof ( _transform ) );
}

void Game::DestroyUniformBuffer ()
{
    _transformBuffer.FreeResources ();
}

bool Game::InitCommandBuffers ( android_vulkan::Renderer &renderer )
{
    const size_t framebufferCount = renderer.GetPresentImageCount ();

    VkCommandBufferAllocateInfo allocateInfo;
    allocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocateInfo.pNext = nullptr;
    allocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocateInfo.commandBufferCount = static_cast<uint32_t> ( framebufferCount );
    allocateInfo.commandPool = _commandPool;

    VkDevice device = renderer.GetDevice ();
    std::vector<VkCommandBuffer> commandBuffers ( framebufferCount );

    bool result = renderer.CheckVkResult ( vkAllocateCommandBuffers ( device, &allocateInfo, commandBuffers.data () ),
        "Game::InitCommandBuffers",
        "Can't allocate command buffers"
    );

    if ( !result )
        return false;

    VkCommandBufferBeginInfo bufferBeginInfo;
    bufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    bufferBeginInfo.pNext = nullptr;
    bufferBeginInfo.flags = 0U;
    bufferBeginInfo.pInheritanceInfo = nullptr;

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

    VkFenceCreateInfo fenceInfo;
    fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceInfo.pNext = nullptr;
    fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    VkFence fence = VK_NULL_HANDLE;
    _commandBuffers.reserve ( framebufferCount );

    for ( size_t i = 0U; i < framebufferCount; ++i )
    {
        result = renderer.CheckVkResult ( vkCreateFence ( device, &fenceInfo, nullptr, &fence ),
            "Game::InitCommandBuffers",
            "Can't create fence"
        );

        if ( !result )
            return false;

        AV_REGISTER_FENCE ( "Game::_commandBuffers::_fence" )

        VkCommandBuffer commandBuffer = commandBuffers[ i ];

        result = renderer.CheckVkResult ( vkBeginCommandBuffer ( commandBuffer, &bufferBeginInfo ),
            "Game::InitCommandBuffers",
            "Can't begin command buffer"
        );

        if ( !result )
            return false;

        renderPassBeginInfo.framebuffer = _framebuffers[ i ];
        vkCmdBeginRenderPass ( commandBuffer, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE );

        vkCmdBindPipeline ( commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, _pipeline );
        constexpr VkDeviceSize offset = 0U;

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

        result = renderer.CheckVkResult ( vkEndCommandBuffer ( commandBuffer ),
            "Game::InitCommandBuffers",
            "Can't end command buffer"
        );

        if ( !result )
            return false;

        _commandBuffers.emplace_back ( std::make_pair ( commandBuffer, fence ) );
    }

    return true;
}

bool Game::UpdateUniformBuffer ( android_vulkan::Renderer &renderer, double deltaTime )
{
    _angle += static_cast<float> ( deltaTime ) * ROTATION_SPEED;

    _transform._normalTransform.RotationY ( _angle );
    _transform._normalTransform.SetOrigin ( GXVec3 ( 0.0F, -1.0F, 3.0F ) );

    GXMat4 tmp1;
    tmp1.Multiply ( _transform._normalTransform, _projectionMatrix );
    _transform._transform.Multiply ( tmp1, renderer.GetPresentationEngineTransform () );

    return _transformBuffer.Update ( reinterpret_cast<const uint8_t*> ( &_transform ), sizeof ( _transform ) );
}

} // namespace rotating_mesh
