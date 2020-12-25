#include <pbr/render_session.h>

GX_DISABLE_COMMON_WARNINGS

#include <algorithm>
#include <cassert>

GX_RESTORE_WARNING_STATE

#include <half_types.h>
#include <vulkan_utils.h>


namespace pbr {

constexpr static const size_t DEFAULT_TEXTURE_COUNT = 5U;

// 1 2 4 8 16 32 64 128 256 512 1024 2048 4096
constexpr static size_t MAX_SUPPORTED_MIP_COUNT = 13U;

//----------------------------------------------------------------------------------------------------------------------

class SamplerStorage final
{
    private:
        SamplerRef      _pointSampler;
        SamplerRef      _storage[ MAX_SUPPORTED_MIP_COUNT ];

    public:
        SamplerStorage () noexcept = default;

        SamplerStorage ( SamplerStorage const & ) = delete;
        SamplerStorage& operator = ( SamplerStorage const & ) = delete;

        SamplerStorage ( SamplerStorage && ) = delete;
        SamplerStorage& operator = ( SamplerStorage && ) = delete;

        ~SamplerStorage () = default;

        void FreeResources ( android_vulkan::Renderer &renderer );

        [[nodiscard]] SamplerRef GetPointSampler ( android_vulkan::Renderer &renderer );
        [[nodiscard]] SamplerRef GetSampler ( uint8_t mips, android_vulkan::Renderer &renderer );
};

void SamplerStorage::FreeResources ( android_vulkan::Renderer &renderer )
{
    if ( _pointSampler )
    {
        _pointSampler->Destroy ( renderer );
        _pointSampler = nullptr;
    }

    for ( auto &sampler : _storage )
    {
        if ( !sampler )
            continue;

        sampler->Destroy ( renderer );
        sampler = nullptr;
    }
}

SamplerRef SamplerStorage::GetPointSampler ( android_vulkan::Renderer &renderer )
{
    if ( _pointSampler )
        return _pointSampler;

    _pointSampler = std::make_shared<Sampler> ();

    VkSamplerCreateInfo info;
    info.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    info.pNext = nullptr;
    info.flags = 0U;
    info.unnormalizedCoordinates = VK_FALSE;
    info.compareEnable = VK_FALSE;
    info.compareOp = VK_COMPARE_OP_ALWAYS;
    info.borderColor = VK_BORDER_COLOR_FLOAT_TRANSPARENT_BLACK;
    info.anisotropyEnable = VK_FALSE;
    info.maxAnisotropy = 1.0F;
    info.mipmapMode = VK_SAMPLER_MIPMAP_MODE_NEAREST;
    info.minFilter = VK_FILTER_NEAREST;
    info.magFilter = VK_FILTER_NEAREST;
    info.minLod = 0.0F;
    info.maxLod = 0.0F;
    info.mipLodBias = 0.0F;
    info.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    info.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    info.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;

    if ( !_pointSampler->Init ( info, renderer ) )
        _pointSampler = nullptr;

    return _pointSampler;
}

SamplerRef SamplerStorage::GetSampler ( uint8_t mips, android_vulkan::Renderer &renderer )
{
    SamplerRef& target = _storage[ static_cast<size_t> ( mips ) ];

    if ( target )
        return target;

    target = std::make_shared<Sampler> ();

    VkSamplerCreateInfo info;
    info.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    info.pNext = nullptr;
    info.flags = 0U;
    info.unnormalizedCoordinates = VK_FALSE;
    info.compareEnable = VK_FALSE;
    info.compareOp = VK_COMPARE_OP_ALWAYS;
    info.borderColor = VK_BORDER_COLOR_FLOAT_TRANSPARENT_BLACK;
    info.anisotropyEnable = VK_FALSE;
    info.maxAnisotropy = 1.0F;
    info.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
    info.minFilter = VK_FILTER_LINEAR;
    info.magFilter = VK_FILTER_LINEAR;
    info.minLod = 0.0F;
    info.maxLod = static_cast<float> ( mips - 1U );
    info.mipLodBias = 0.0F;
    info.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    info.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    info.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;

    if ( !target->Init ( info, renderer ) )
        target = nullptr;

    return target;
}

static SamplerStorage g_SamplerStorage;

//----------------------------------------------------------------------------------------------------------------------

RenderSession::RenderSession () noexcept:
    _albedoDefault {},
    _emissionDefault {},
    _maskDefault {},
    _normalDefault {},
    _paramDefault {},
    _commandPool ( VK_NULL_HANDLE ),
    _descriptorPool ( VK_NULL_HANDLE ),
    _frustum {},
    _gBuffer {},
    _gBufferFramebuffer ( VK_NULL_HANDLE ),
    _gBufferImageBarrier {},
    _gBufferRenderPass ( VK_NULL_HANDLE ),
    _geometryPassBeginInfo {},
    _geometryPassClearValue {},
    _geometryPassFence ( VK_NULL_HANDLE ),
    _geometryPassRendering ( VK_NULL_HANDLE ),
    _geometryPassTransfer ( VK_NULL_HANDLE ),
    _isFreeTransferResources ( false ),
    _maxBatchCount ( 0U ),
    _maxUniqueCount ( 0U ),
    _opaqueCalls {},
    _pointLightCalls {},
    _pointLightShadowMaps {},
    _opaqueBatchProgram {},
    _texturePresentProgram {},
    _presentInfo {},
    _presentBeginInfo {},
    _presentClearValue {},
    _presentFramebuffers {},
    _presentRenderPass ( VK_NULL_HANDLE ),
    _presentRenderPassEndSemaphore ( VK_NULL_HANDLE ),
    _presentRenderTargetAcquiredSemaphore ( VK_NULL_HANDLE ),
    _renderSessionStats {},
    _submitInfoRender {},
    _submitInfoTransfer {},
    _uniformBufferPool {},
    _view {},
    _viewProjection {}
{
    // NOTHING
}

void RenderSession::Begin ( GXMat4 const &view, GXMat4 const &projection )
{
    _maxBatchCount = 0U;
    _view = view;
    _viewProjection.Multiply ( view, projection );
    _frustum.From ( _viewProjection );
    _opaqueCalls.clear ();
    _pointLightCalls.clear ();
}

bool RenderSession::End ( ePresentTarget /*target*/, double deltaTime, android_vulkan::Renderer &renderer )
{
    CleanupTransferResources ( renderer );

    uint32_t framebufferIndex = UINT32_MAX;

    bool result = android_vulkan::Renderer::CheckVkResult (
        vkAcquireNextImageKHR ( renderer.GetDevice (),
            renderer.GetSwapchain (),
            UINT64_MAX,
            _presentRenderTargetAcquiredSemaphore,
            VK_NULL_HANDLE,
            &framebufferIndex
        ),

        "RenderSession::End",
        "Can't get presentation image index"
    );

    if ( !result )
        return false;

    UpdatePointLightShadowMaps ();

    if ( !BeginGeometryRenderPass ( renderer ) )
        return false;

    std::vector<VkDescriptorSet> descriptorSetStorage;

    if ( !UpdateGPUData ( descriptorSetStorage, renderer ) )
        return false;

    VkDescriptorSet const* textureSets = descriptorSetStorage.data ();
    DrawOpaque ( textureSets, textureSets + ( _opaqueCalls.size () + 1U ) );

    vkCmdEndRenderPass ( _geometryPassRendering );

    android_vulkan::Texture2D& targetTexture = _gBuffer.GetAlbedo ();

    _gBufferImageBarrier.image = targetTexture.GetImage ();
    _gBufferImageBarrier.subresourceRange.levelCount = static_cast<uint32_t> ( targetTexture.GetMipLevelCount () );

    vkCmdPipelineBarrier ( _geometryPassRendering,
        VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
        VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
        0U,
        0U,
        nullptr,
        0U,
        nullptr,
        1U,
        &_gBufferImageBarrier
    );

    _presentBeginInfo.framebuffer = _presentFramebuffers[ framebufferIndex ];
    _presentBeginInfo.renderArea.extent = renderer.GetSurfaceSize ();

    vkCmdBeginRenderPass ( _geometryPassRendering, &_presentBeginInfo, VK_SUBPASS_CONTENTS_INLINE );
    _texturePresentProgram.Bind ( _geometryPassRendering );

    _texturePresentProgram.SetData ( _geometryPassRendering,
        descriptorSetStorage[ _opaqueCalls.size () ],
        renderer.GetPresentationEngineTransform ()
    );

    vkCmdDraw ( _geometryPassRendering, 4U, 1U, 0U, 0U );
    vkCmdEndRenderPass ( _geometryPassRendering );

    result = android_vulkan::Renderer::CheckVkResult ( vkEndCommandBuffer ( _geometryPassRendering ),
        "RenderSession::End",
        "Can't end command buffer"
    );

    if ( !result )
        return false;

    result = android_vulkan::Renderer::CheckVkResult (
        vkQueueSubmit ( renderer.GetQueue (), 1U, &_submitInfoRender, _geometryPassFence ),
        "RenderSession::End",
        "Can't submit geometry render command buffer"
    );

    if ( !result )
        return false;

    VkResult presentResult = VK_ERROR_DEVICE_LOST;

    _presentInfo.pResults = &presentResult;
    _presentInfo.pSwapchains = &renderer.GetSwapchain ();
    _presentInfo.pImageIndices = &framebufferIndex;

    result = android_vulkan::Renderer::CheckVkResult ( vkQueuePresentKHR ( renderer.GetQueue (), &_presentInfo ),
        "RenderSession::EndFrame",
        "Can't present frame"
    );

    if ( !result )
        return false;

    result = android_vulkan::Renderer::CheckVkResult ( presentResult,
        "RenderSession::EndFrame",
        "Present queue has been failed"
    );

    if ( !result )
        return false;

    _renderSessionStats.PrintStats ( deltaTime );
    return true;
}

bool RenderSession::Init ( android_vulkan::Renderer &renderer, VkExtent2D const &resolution )
{
    if ( !_gBuffer.Init ( resolution, renderer ) )
        return false;

    if ( !CreateSyncPrimitives ( renderer ) )
    {
        Destroy ( renderer );
        return false;
    }

    if ( !CreateGBufferRenderPass ( renderer ) )
    {
        Destroy ( renderer );
        return false;
    }

    if ( !CreateGBufferFramebuffer ( renderer ) )
    {
        Destroy ( renderer );
        return false;
    }

    if ( !CreatePresentRenderPass ( renderer ) )
    {
        Destroy ( renderer );
        return false;
    }

    if ( !CreatePresentFramebuffers ( renderer ) )
    {
        Destroy ( renderer );
        return false;
    }

    VkFenceCreateInfo fenceInfo;
    fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceInfo.pNext = nullptr;
    fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    VkDevice device = renderer.GetDevice ();

    bool result = android_vulkan::Renderer::CheckVkResult (
        vkCreateFence ( device, &fenceInfo, nullptr, &_geometryPassFence ),
        "RenderSession::Init",
        "Can't create GBuffer fence"
    );

    if ( !result )
    {
        Destroy ( renderer );
        return false;
    }

    AV_REGISTER_FENCE ( "RenderSession::_geometryPassFence" )

    if ( !_opaqueBatchProgram.Init ( renderer, _gBufferRenderPass, resolution ) )
    {
        Destroy ( renderer );
        return false;
    }

    if ( !_texturePresentProgram.Init ( renderer, _presentRenderPass, renderer.GetSurfaceSize () ) )
    {
        Destroy ( renderer );
        return false;
    }

    if ( !_uniformBufferPool.Init ( sizeof ( OpaqueProgram::InstanceData ), renderer ) )
    {
        Destroy ( renderer );
        return false;
    }

    VkCommandPoolCreateInfo info;
    info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    info.pNext = nullptr;
    info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    info.queueFamilyIndex = renderer.GetQueueFamilyIndex ();

    result = android_vulkan::Renderer::CheckVkResult ( vkCreateCommandPool ( device, &info, nullptr, &_commandPool ),
        "RenderSession::Init",
        "Can't create command pool"
    );

    if ( !result )
    {
        Destroy ( renderer );
        return false;
    }

    AV_REGISTER_COMMAND_POOL ( "RenderSession::_commandPool" )

    VkCommandBuffer commandBuffers[ DEFAULT_TEXTURE_COUNT + 2U ];
    VkCommandBufferAllocateInfo allocateInfo;
    allocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocateInfo.pNext = nullptr;
    allocateInfo.commandPool = _commandPool;
    allocateInfo.commandBufferCount = static_cast<uint32_t> ( std::size ( commandBuffers ) );
    allocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;

    result = android_vulkan::Renderer::CheckVkResult (
        vkAllocateCommandBuffers ( device, &allocateInfo, commandBuffers ),
        "RenderSession::Init",
        "Can't allocate command buffers"
    );

    if ( !result )
    {
        Destroy ( renderer );
        return false;
    }

    _geometryPassTransfer = commandBuffers[ DEFAULT_TEXTURE_COUNT ];
    _geometryPassRendering = commandBuffers[ DEFAULT_TEXTURE_COUNT + 1U ];

    auto textureLoader = [ &renderer ] ( Texture2DRef &texture,
        const uint8_t* data,
        size_t size,
        VkFormat format,
        VkCommandBuffer commandBuffer
    ) -> bool {
        texture = std::make_shared<android_vulkan::Texture2D> ();
        constexpr const VkExtent2D resolution { .width = 1U, .height = 1U };

        const bool result = texture->UploadData ( data,
            size,
            resolution,
            format,
            false,
            renderer,
            commandBuffer
        );

        if ( result )
            return true;

        texture = nullptr;
        return false;
    };

    constexpr const uint8_t albedo[ 4U ] = { 255U, 255U, 255U, 255U };

    result = textureLoader ( _albedoDefault,
        albedo,
        sizeof ( albedo ),
        VK_FORMAT_R8G8B8A8_SRGB,
        commandBuffers[ 0U ]
    );

    if ( !result )
    {
        Destroy ( renderer );
        return false;
    }

    _isFreeTransferResources = true;

    android_vulkan::Half4 const emission ( 0.0F, 0.0F, 0.0F, 0.0F );

    result = textureLoader ( _emissionDefault,
        reinterpret_cast<const uint8_t*> ( emission._data ),
        sizeof ( emission ),
        VK_FORMAT_R16G16B16A16_SFLOAT,
        commandBuffers[ 1U ]
    );

    if ( !result )
    {
        Destroy ( renderer );
        return false;
    }

    constexpr const uint8_t mask[ 4U ] = { 255U, 0U, 0U, 0U };

    result = textureLoader ( _maskDefault,
        mask,
        sizeof ( mask ),
        VK_FORMAT_R8G8B8A8_UNORM,
        commandBuffers[ 2U ]
    );

    if ( !result )
    {
        Destroy ( renderer );
        return false;
    }

    // See Table 53. Bit mappings for packed 32-bit formats of Vulkan 1.1.108 spec.
    // https://vulkan.lunarg.com/doc/view/1.1.108.0/mac/chunked_spec/chap37.html#formats-packed
    //                                        A         R              G              B
    //                                       0.0F      0.5F           0.5F           1.0F
    //                                         ><>-------------<>-------------<>-------------<
    constexpr const uint8_t normal[ 4U ] = { 0b00100000U, 0b00001000U, 0b00000011U, 0b11111111U };

    result = textureLoader ( _normalDefault,
        normal,
        sizeof ( normal ),
        VK_FORMAT_A2R10G10B10_UNORM_PACK32,
        commandBuffers[ 3U ]
    );

    if ( !result )
    {
        Destroy ( renderer );
        return false;
    }

    constexpr const uint8_t param[ 4U ] = { 128U, 128U, 128U, 128U };

    result = textureLoader ( _paramDefault,
        param,
        sizeof ( param ),
        VK_FORMAT_R8G8B8A8_UNORM,
        commandBuffers[ 4U ]
    );

    if ( result )
    {
        InitCommonStructures ();
        return true;
    }

    Destroy ( renderer );
    return false;
}

void RenderSession::Destroy ( android_vulkan::Renderer &renderer )
{
    auto freeTexture = [ &renderer ] ( Texture2DRef &texture ) {
        if ( !texture )
            return;

        texture->FreeResources ( renderer );
        texture = nullptr;
    };

    freeTexture ( _paramDefault );
    freeTexture ( _normalDefault );
    freeTexture ( _maskDefault );
    freeTexture ( _emissionDefault );
    freeTexture ( _albedoDefault );

    g_SamplerStorage.FreeResources ( renderer );
    DestroyDescriptorPool ( renderer );

    VkDevice device = renderer.GetDevice ();

    if ( _commandPool != VK_NULL_HANDLE )
    {
        vkDestroyCommandPool ( device, _commandPool, nullptr );
        _commandPool = VK_NULL_HANDLE;
        AV_UNREGISTER_COMMAND_POOL ( "RenderSession::_commandPool" )
    }

    _uniformBufferPool.Destroy ( renderer );
    _texturePresentProgram.Destroy ( renderer );
    _opaqueBatchProgram.Destroy ( renderer );

    if ( _geometryPassFence != VK_NULL_HANDLE )
    {
        vkDestroyFence ( device, _geometryPassFence, nullptr );
        _geometryPassFence = VK_NULL_HANDLE;
        AV_UNREGISTER_FENCE ( "RenderSession::_geometryPassFence" )
    }

    DestroyPresentFramebuffers ( renderer );

    if ( _presentRenderPass != VK_NULL_HANDLE )
    {
        vkDestroyRenderPass ( device, _presentRenderPass, nullptr );
        _presentRenderPass = VK_NULL_HANDLE;
        AV_UNREGISTER_RENDER_PASS ( "RenderSession::_presentRenderPass" )
    }

    if ( _gBufferFramebuffer != VK_NULL_HANDLE )
    {
        vkDestroyFramebuffer ( device, _gBufferFramebuffer, nullptr );
        _gBufferFramebuffer = VK_NULL_HANDLE;
        AV_UNREGISTER_FRAMEBUFFER ( "RenderSession::_gBufferFramebuffer" )
    }

    if ( _gBufferRenderPass != VK_NULL_HANDLE )
    {
        vkDestroyRenderPass ( renderer.GetDevice (), _gBufferRenderPass, nullptr );
        _gBufferRenderPass = VK_NULL_HANDLE;
        AV_UNREGISTER_RENDER_PASS ( "RenderSession::_gBufferRenderPass" )
    }

    DestroySyncPrimitives ( renderer );
    _gBuffer.Destroy ( renderer );
}

void RenderSession::SubmitMesh ( MeshRef &mesh,
    MaterialRef const &material,
    GXMat4 const &local,
    GXAABB const &worldBounds,
    android_vulkan::Half4 const &color0,
    android_vulkan::Half4 const &color1,
    android_vulkan::Half4 const &color2,
    android_vulkan::Half4 const &color3
)
{
    if ( material->GetMaterialType() == eMaterialType::Opaque )
    {
        _renderSessionStats.SubmitOpaque ( mesh->GetVertexCount () );
        SubmitOpaqueCall ( mesh, material, local, worldBounds, color0, color1, color2, color3 );
    }
}

void RenderSession::SubmitLight ( LightRef const &light )
{
    if ( light->GetType () == eLightType::PointLight )
    {
        SubmitPointLight ( light );
    }
}

bool RenderSession::BeginGeometryRenderPass ( android_vulkan::Renderer &renderer )
{
    VkDevice device = renderer.GetDevice ();

    bool result = android_vulkan::Renderer::CheckVkResult (
        vkWaitForFences ( device, 1U, &_geometryPassFence, VK_TRUE, UINT64_MAX ),
        "RenderSession::BeginGeometryRenderPass",
        "Can't wait for geometry pass fence"
    );

    if ( !result )
        return false;

    result = android_vulkan::Renderer::CheckVkResult ( vkResetFences ( device, 1U, &_geometryPassFence ),
        "RenderSession::BeginGeometryRenderPass",
        "Can't reset geometry pass fence"
    );

    if ( !result )
        return false;

    VkCommandBufferBeginInfo beginInfo;
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.pNext = nullptr;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    beginInfo.pInheritanceInfo = nullptr;

    result = android_vulkan::Renderer::CheckVkResult ( vkBeginCommandBuffer ( _geometryPassRendering, &beginInfo ),
        "RenderSession::End",
        "Can't begin geometry pass rendering command buffer"
    );

    if ( !result )
        return false;

    vkCmdBeginRenderPass ( _geometryPassRendering, &_geometryPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE );
    return true;
}

void RenderSession::CleanupTransferResources ( android_vulkan::Renderer &renderer )
{
    if ( !_isFreeTransferResources )
        return;

    if ( _albedoDefault )
        _albedoDefault->FreeTransferResources ( renderer );

    if ( _emissionDefault )
        _emissionDefault->FreeTransferResources ( renderer );

    if ( _maskDefault )
        _maskDefault->FreeTransferResources ( renderer );

    if ( _normalDefault )
        _normalDefault->FreeTransferResources ( renderer );

    if ( _paramDefault )
        _paramDefault->FreeTransferResources ( renderer );

    _isFreeTransferResources = false;
}

bool RenderSession::CreateGBufferFramebuffer ( android_vulkan::Renderer &renderer )
{
    const VkExtent2D& resolution = _gBuffer.GetResolution ();

    VkImageView attachments[ GBUFFER_ATTACHMENT_COUNT ] =
    {
        _gBuffer.GetAlbedo ().GetImageView (),
        _gBuffer.GetEmission ().GetImageView (),
        _gBuffer.GetNormal ().GetImageView (),
        _gBuffer.GetParams ().GetImageView (),
        _gBuffer.GetDepthStencil ().GetImageView ()
    };

    VkFramebufferCreateInfo framebufferInfo;
    framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    framebufferInfo.pNext = nullptr;
    framebufferInfo.flags = 0U;
    framebufferInfo.attachmentCount = std::size ( attachments );
    framebufferInfo.pAttachments = attachments;
    framebufferInfo.width = resolution.width;
    framebufferInfo.height = resolution.height;
    framebufferInfo.layers = 1U;
    framebufferInfo.renderPass = _gBufferRenderPass;

    const bool result = android_vulkan::Renderer::CheckVkResult (
        vkCreateFramebuffer ( renderer.GetDevice (), &framebufferInfo, nullptr, &_gBufferFramebuffer ),
        "RenderSession::CreateGBufferFramebuffer",
        "Can't create GBuffer framebuffer"
    );

    if ( !result )
        return false;

    AV_REGISTER_FRAMEBUFFER ( "RenderSession::_gBufferFramebuffer" )
    return true;
}

bool RenderSession::CreateGBufferRenderPass ( android_vulkan::Renderer &renderer )
{
    constexpr size_t const colorAttachmentCount = GBUFFER_ATTACHMENT_COUNT - 1U;

    VkAttachmentReference colorAttachmentReferences[ colorAttachmentCount ];
    VkAttachmentReference& albedoRef = colorAttachmentReferences[ 0U ];
    albedoRef.attachment = 0U;
    albedoRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkAttachmentReference& emissionRef = colorAttachmentReferences[ 1U ];
    emissionRef.attachment = 1U;
    emissionRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkAttachmentReference& normalRef = colorAttachmentReferences[ 2U ];
    normalRef.attachment = 2U;
    normalRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkAttachmentReference& paramsRef = colorAttachmentReferences[ 3U ];
    paramsRef.attachment = 3U;
    paramsRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkAttachmentReference depthAttachmentReference;
    depthAttachmentReference.attachment = 4U;
    depthAttachmentReference.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    VkSubpassDescription subpass;
    subpass.flags = 0U;
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = static_cast<uint32_t> ( colorAttachmentCount );
    subpass.pColorAttachments = colorAttachmentReferences;
    subpass.inputAttachmentCount = 0U;
    subpass.pInputAttachments = nullptr;
    subpass.pDepthStencilAttachment = &depthAttachmentReference;
    subpass.pResolveAttachments = nullptr;
    subpass.preserveAttachmentCount = 0U;
    subpass.pPreserveAttachments = nullptr;

    VkAttachmentDescription attachments[ GBUFFER_ATTACHMENT_COUNT ];
    VkAttachmentDescription& albedoAttachment = attachments[ 0U ];
    albedoAttachment.flags = 0U;
    albedoAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    albedoAttachment.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    albedoAttachment.format = _gBuffer.GetAlbedo ().GetFormat ();
    albedoAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
    albedoAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    albedoAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    albedoAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    albedoAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;

    VkAttachmentDescription& emissionAttachment = attachments[ 1U ];
    emissionAttachment.flags = 0U;
    emissionAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    emissionAttachment.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    emissionAttachment.format = _gBuffer.GetEmission ().GetFormat ();
    emissionAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
    emissionAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    emissionAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    emissionAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    emissionAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;

    VkAttachmentDescription& normalAttachment = attachments[ 2U ];
    normalAttachment.flags = 0U;
    normalAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    normalAttachment.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    normalAttachment.format = _gBuffer.GetNormal ().GetFormat ();
    normalAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
    normalAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    normalAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    normalAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    normalAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;

    VkAttachmentDescription& paramsAttachment = attachments[ 3U ];
    paramsAttachment.flags = 0U;
    paramsAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    paramsAttachment.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    paramsAttachment.format = _gBuffer.GetParams ().GetFormat ();
    paramsAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
    paramsAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    paramsAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    paramsAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    paramsAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;

    VkAttachmentDescription& depthAttachment = attachments[ 4U ];
    depthAttachment.flags = 0U;
    depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
    depthAttachment.format = _gBuffer.GetDepthStencil ().GetFormat ();
    depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
    depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_STORE;

    VkRenderPassCreateInfo renderPassInfo;
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassInfo.pNext = nullptr;
    renderPassInfo.flags = 0U;
    renderPassInfo.subpassCount = 1U;
    renderPassInfo.pSubpasses = &subpass;
    renderPassInfo.dependencyCount = 0U;
    renderPassInfo.pDependencies = nullptr;
    renderPassInfo.attachmentCount = static_cast<uint32_t> ( std::size ( attachments ) );
    renderPassInfo.pAttachments = attachments;

    bool const result = android_vulkan::Renderer::CheckVkResult (
        vkCreateRenderPass ( renderer.GetDevice (), &renderPassInfo, nullptr, &_gBufferRenderPass ),
        "RenderSession::CreateGBufferRenderPass",
        "Can't create GBuffer render pass"
    );

    if ( !result )
        return false;

    AV_REGISTER_RENDER_PASS ( "RenderSession::_gBufferRenderPass" )
    return true;
}

bool RenderSession::CreatePresentFramebuffers ( android_vulkan::Renderer &renderer )
{
    size_t const framebufferCount = renderer.GetPresentImageCount ();
    _presentFramebuffers.reserve ( framebufferCount );
    VkFramebuffer framebuffer = VK_NULL_HANDLE;

    VkExtent2D const& resolution = renderer.GetSurfaceSize ();

    VkFramebufferCreateInfo framebufferInfo;
    framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    framebufferInfo.pNext = nullptr;
    framebufferInfo.flags = 0U;
    framebufferInfo.renderPass = _presentRenderPass;
    framebufferInfo.layers = 1U;
    framebufferInfo.width = resolution.width;
    framebufferInfo.height = resolution.height;
    framebufferInfo.attachmentCount = 1U;

    VkDevice device = renderer.GetDevice ();

    for ( size_t i = 0U; i < framebufferCount; ++i )
    {
        framebufferInfo.pAttachments = &renderer.GetPresentImageView ( i );

        bool const result = android_vulkan::Renderer::CheckVkResult (
            vkCreateFramebuffer ( device, &framebufferInfo, nullptr, &framebuffer ),
            "RenderSession::CreatePresentFramebuffers",
            "Can't create a framebuffer"
        );

        if ( !result )
            return false;

        _presentFramebuffers.push_back ( framebuffer );
        AV_REGISTER_FRAMEBUFFER ( "RenderSession::_presentFramebuffers" )
    }

    return true;
}

void RenderSession::DestroyPresentFramebuffers ( android_vulkan::Renderer &renderer )
{
    VkDevice device = renderer.GetDevice ();

    for ( auto framebuffer : _presentFramebuffers )
    {
        vkDestroyFramebuffer ( device, framebuffer, nullptr );
        AV_UNREGISTER_FRAMEBUFFER ( "RenderSession::_presentFramebuffers" )
    }

    _presentFramebuffers.clear ();
}

bool RenderSession::CreatePresentRenderPass ( android_vulkan::Renderer &renderer )
{
    VkAttachmentDescription attachment;
    attachment.flags = 0U;
    attachment.format = renderer.GetSurfaceFormat ();
    attachment.samples = VK_SAMPLE_COUNT_1_BIT;
    attachment.loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    attachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    VkAttachmentReference reference;
    reference.attachment = 0U;
    reference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkSubpassDescription subpass;
    subpass.flags = 0U;
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.inputAttachmentCount = 0U;
    subpass.pInputAttachments = nullptr;
    subpass.colorAttachmentCount = 1U;
    subpass.pColorAttachments = &reference;
    subpass.pResolveAttachments = nullptr;
    subpass.pDepthStencilAttachment = nullptr;
    subpass.preserveAttachmentCount = 0U;
    subpass.pPreserveAttachments = nullptr;

    VkRenderPassCreateInfo info;
    info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    info.pNext = nullptr;
    info.flags = 0U;
    info.attachmentCount = 1U;
    info.pAttachments = &attachment;
    info.subpassCount = 1U;
    info.pSubpasses = &subpass;
    info.dependencyCount = 0U;
    info.pDependencies = nullptr;

    bool const result = android_vulkan::Renderer::CheckVkResult (
        vkCreateRenderPass ( renderer.GetDevice (), &info, nullptr, &_presentRenderPass ),
        "RenderSession::CreatePresentRenderPass",
        "Can't create render pass"
    );

    if ( !result )
        return false;

    AV_REGISTER_RENDER_PASS ( "RenderSession::_presentRenderPass" )
    return true;
}

bool RenderSession::CreateSyncPrimitives ( android_vulkan::Renderer &renderer )
{
    VkSemaphoreCreateInfo semaphoreInfo;
    semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
    semaphoreInfo.pNext = nullptr;
    semaphoreInfo.flags = 0U;

    VkDevice device = renderer.GetDevice ();

    bool result = android_vulkan::Renderer::CheckVkResult (
        vkCreateSemaphore ( device, &semaphoreInfo, nullptr, &_presentRenderPassEndSemaphore ),
        "RenderSession::CreateSyncPrimitives",
        "Can't create render pass end semaphore"
    );

    if ( !result )
        return false;

    AV_REGISTER_SEMAPHORE ( "RenderSession::_presentRenderPassEndSemaphore" )

    result = android_vulkan::Renderer::CheckVkResult (
        vkCreateSemaphore ( device, &semaphoreInfo, nullptr, &_presentRenderTargetAcquiredSemaphore ),
        "RenderSession::CreateSyncPrimitives",
        "Can't create render target acquired semaphore"
    );

    if ( !result )
        return false;

    AV_REGISTER_SEMAPHORE ( "RenderSession::_presentRenderTargetAcquiredSemaphore" )
    return true;
}

void RenderSession::DestroySyncPrimitives ( android_vulkan::Renderer &renderer )
{
    VkDevice device = renderer.GetDevice ();

    if ( _presentRenderTargetAcquiredSemaphore != VK_NULL_HANDLE )
    {
        vkDestroySemaphore ( device, _presentRenderTargetAcquiredSemaphore, nullptr );
        _presentRenderTargetAcquiredSemaphore = VK_NULL_HANDLE;
        AV_UNREGISTER_SEMAPHORE ( "RenderSession::_presentRenderTargetAcquiredSemaphore" )
    }

    if ( _presentRenderPassEndSemaphore == VK_NULL_HANDLE )
        return;

    vkDestroySemaphore ( device, _presentRenderPassEndSemaphore, nullptr );
    _presentRenderPassEndSemaphore = VK_NULL_HANDLE;
    AV_UNREGISTER_SEMAPHORE ( "RenderSession::_presentRenderPassEndSemaphore" )
}

void RenderSession::DestroyDescriptorPool ( android_vulkan::Renderer &renderer )
{
    if ( _descriptorPool == VK_NULL_HANDLE )
        return;

    vkDestroyDescriptorPool ( renderer.GetDevice (), _descriptorPool, nullptr );
    _descriptorPool = VK_NULL_HANDLE;
    AV_UNREGISTER_DESCRIPTOR_POOL ( "RenderSession::_descriptorPool" )
}

void RenderSession::DrawOpaque ( VkDescriptorSet const* textureSets, VkDescriptorSet const* instanceSets )
{
    size_t textureSetIndex = 0U;
    size_t uniformUsed = 0U;
    bool isProgramBind = false;

    constexpr VkDeviceSize const offset = 0U;

    for ( auto const &call : _opaqueCalls )
    {
        OpaqueCall const &opaqueCall = call.second;

        VkDescriptorSet textureSet = textureSets[ textureSetIndex ];
        ++textureSetIndex;

        if ( !isProgramBind )
        {
            _opaqueBatchProgram.Bind ( _geometryPassRendering );
            isProgramBind = true;
        }

        bool isUniformBind = false;

        auto instanceDrawer = [ & ] ( MeshRef const &mesh, uint32_t batches ) {
            if ( isUniformBind )
            {
                _opaqueBatchProgram.SetDescriptorSet ( _geometryPassRendering, instanceSets + uniformUsed, 1U, 1U );
            }
            else
            {
                VkDescriptorSet sets[] = { textureSet, instanceSets[ uniformUsed ] };

                _opaqueBatchProgram.SetDescriptorSet ( _geometryPassRendering,
                    sets,
                    0U,
                    static_cast<uint32_t> ( std::size ( sets ) )
                );

                isUniformBind = true;
            }

            vkCmdBindVertexBuffers ( _geometryPassRendering, 0U, 1U, &mesh->GetVertexBuffer (), &offset );
            vkCmdBindIndexBuffer ( _geometryPassRendering, mesh->GetIndexBuffer (), 0U, VK_INDEX_TYPE_UINT32 );

            vkCmdDrawIndexed ( _geometryPassRendering,
                mesh->GetVertexCount (),
                batches,
                0U,
                0U,
                0U
            );

            _renderSessionStats.RenderOpaque ( mesh->GetVertexCount (), batches );
            ++uniformUsed;
        };

        for ( auto const &[mesh, opaqueData] : opaqueCall.GetUniqueList () )
        {
            if ( !opaqueData._isVisible )
                continue;

            instanceDrawer ( mesh, 1U );
        }

        for ( auto const &item : opaqueCall.GetBatchList () )
        {
            MeshGroup const& group = item.second;
            MeshRef const& mesh = group._mesh;
            size_t instanceCount = 0U;

            for ( auto const& opaqueData : group._opaqueData )
            {
                if ( !opaqueData._isVisible )
                    continue;

                ++instanceCount;
            }

            size_t instanceIndex = 0U;
            size_t batches = 0U;

            while ( instanceIndex < instanceCount )
            {
                batches = std::min ( instanceCount - instanceIndex,
                    static_cast<size_t> ( PBR_OPAQUE_MAX_INSTANCE_COUNT )
                );

                instanceIndex += batches;

                if ( batches < PBR_OPAQUE_MAX_INSTANCE_COUNT )
                    continue;

                instanceDrawer ( mesh, static_cast<uint32_t> ( batches ) );
                batches = 0U;
            }

            if ( !batches )
                continue;

            instanceDrawer ( mesh, static_cast<uint32_t> ( batches ) );
        }
    }
}

void RenderSession::InitCommonStructures ()
{
    VkClearValue& albedoClear = _geometryPassClearValue[ 0U ];
    albedoClear.color.float32[ 0U ] = 0.0F;
    albedoClear.color.float32[ 1U ] = 0.0F;
    albedoClear.color.float32[ 2U ] = 0.0F;
    albedoClear.color.float32[ 3U ] = 0.0F;

    VkClearValue& emissionClear = _geometryPassClearValue[ 1U ];
    emissionClear.color.float32[ 1U ] = 0.0F;
    emissionClear.color.float32[ 2U ] = 0.0F;
    emissionClear.color.float32[ 3U ] = 0.0F;
    emissionClear.color.float32[ 0U ] = 0.0F;

    VkClearValue& normalClear = _geometryPassClearValue[ 2U ];
    normalClear.color.float32[ 1U ] = 0.5F;
    normalClear.color.float32[ 2U ] = 0.5F;
    normalClear.color.float32[ 3U ] = 0.5F;
    normalClear.color.float32[ 0U ] = 0.0F;

    VkClearValue& paramClear = _geometryPassClearValue[ 3U ];
    paramClear.color.float32[ 1U ] = 0.5F;
    paramClear.color.float32[ 2U ] = 0.5F;
    paramClear.color.float32[ 3U ] = 0.5F;
    paramClear.color.float32[ 0U ] = 0.0F;

    VkClearValue& depthStencilClear = _geometryPassClearValue[ 4U ];
    depthStencilClear.depthStencil.depth = 1.0F;
    depthStencilClear.depthStencil.stencil = 0U;

    _geometryPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    _geometryPassBeginInfo.pNext = nullptr;
    _geometryPassBeginInfo.renderPass = _gBufferRenderPass;
    _geometryPassBeginInfo.framebuffer = _gBufferFramebuffer;
    _geometryPassBeginInfo.clearValueCount = static_cast<uint32_t> ( std::size ( _geometryPassClearValue ) );
    _geometryPassBeginInfo.pClearValues = _geometryPassClearValue;
    _geometryPassBeginInfo.renderArea.offset.x = 0;
    _geometryPassBeginInfo.renderArea.offset.y = 0;
    _geometryPassBeginInfo.renderArea.extent = _gBuffer.GetResolution ();

    _gBufferImageBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    _gBufferImageBarrier.pNext = nullptr;
    _gBufferImageBarrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    _gBufferImageBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
    _gBufferImageBarrier.oldLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    _gBufferImageBarrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    _gBufferImageBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    _gBufferImageBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    _gBufferImageBarrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    _gBufferImageBarrier.subresourceRange.baseMipLevel = 0U;
    _gBufferImageBarrier.subresourceRange.baseArrayLayer = 0U;
    _gBufferImageBarrier.subresourceRange.layerCount = 1U;

    _presentClearValue.color.float32[ 0U ] = 0.0F;
    _presentClearValue.color.float32[ 1U ] = 0.0F;
    _presentClearValue.color.float32[ 2U ] = 0.0F;
    _presentClearValue.color.float32[ 3U ] = 0.0F;

    _presentBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    _presentBeginInfo.pNext = nullptr;
    _presentBeginInfo.renderPass = _presentRenderPass;
    _presentBeginInfo.clearValueCount = 1U;
    _presentBeginInfo.pClearValues = &_presentClearValue;
    _presentBeginInfo.renderArea.offset.x = 0;
    _presentBeginInfo.renderArea.offset.y = 0;

    constexpr static VkPipelineStageFlags const waitStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;

    _submitInfoRender.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    _submitInfoRender.pNext = nullptr;
    _submitInfoRender.waitSemaphoreCount = 1U;
    _submitInfoRender.pWaitSemaphores = &_presentRenderTargetAcquiredSemaphore;
    _submitInfoRender.pWaitDstStageMask = &waitStage;
    _submitInfoRender.commandBufferCount = 1U;
    _submitInfoRender.pCommandBuffers = &_geometryPassRendering;
    _submitInfoRender.signalSemaphoreCount = 1U;
    _submitInfoRender.pSignalSemaphores = &_presentRenderPassEndSemaphore;

    _presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    _presentInfo.pNext = nullptr;
    _presentInfo.waitSemaphoreCount = 1U;
    _presentInfo.pWaitSemaphores = &_presentRenderPassEndSemaphore;
    _presentInfo.swapchainCount = 1U;

    _submitInfoTransfer.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    _submitInfoTransfer.pNext = nullptr;
    _submitInfoTransfer.waitSemaphoreCount = 0U;
    _submitInfoTransfer.pWaitSemaphores = nullptr;
    _submitInfoTransfer.pWaitDstStageMask = nullptr;
    _submitInfoTransfer.commandBufferCount = 1U;
    _submitInfoTransfer.pCommandBuffers = &_geometryPassTransfer;
    _submitInfoTransfer.signalSemaphoreCount = 0U;
    _submitInfoTransfer.pSignalSemaphores = nullptr;
}

void RenderSession::SubmitOpaqueCall ( MeshRef &mesh,
    MaterialRef const &material,
    GXMat4 const &local,
    GXAABB const &worldBounds,
    android_vulkan::Half4 const &color0,
    android_vulkan::Half4 const &color1,
    android_vulkan::Half4 const &color2,
    android_vulkan::Half4 const &color3
)
{
    // Note it's safe to cast like that here. "NOLINT" is a clang-tidy control comment.
    auto& opaqueMaterial = *static_cast<OpaqueMaterial*> ( material.get () ); // NOLINT
    auto findResult = _opaqueCalls.find ( opaqueMaterial );

    if ( findResult != _opaqueCalls.cend () )
    {
        findResult->second.Append ( _maxBatchCount,
            _maxUniqueCount,
            mesh,
            local,
            worldBounds,
            color0,
            color1,
            color2,
            color3
        );

        return;
    }

    _opaqueCalls.insert (
        std::make_pair ( opaqueMaterial,
            OpaqueCall ( _maxBatchCount, _maxUniqueCount, mesh, local, worldBounds, color0, color1, color2, color3 )
        )
    );
}

void RenderSession::SubmitPointLight ( LightRef const &light )
{
    _renderSessionStats.SubmitPointLight ();

    // Note it's safe cast like that here. "NOLINT" is a clang-tidy control comment.
    auto const& pointLight = static_cast<PointLight const&> ( *light.get () ); // NOLINT

    if ( _frustum.IsVisible ( pointLight.GetBounds () ) )
    {
        _pointLightCalls.emplace_back ( std::make_pair ( light, ShadowCasters () ) );
    }
}

bool RenderSession::UpdateGPUData ( std::vector<VkDescriptorSet> &descriptorSetStorage,
    android_vulkan::Renderer &renderer
)
{
    // TODO no any elements have been submitted.

    size_t const opaqueCount = _opaqueCalls.size ();
    size_t const textureCount = opaqueCount * OpaqueTextureDescriptorSetLayout::TEXTURE_SLOTS + 1U;

    std::vector<VkDescriptorImageInfo> imageStorage;
    imageStorage.reserve ( textureCount );

    std::vector<VkWriteDescriptorSet> writeStorage0;
    writeStorage0.reserve ( textureCount * 2U );

    std::vector<DescriptorSetInfo> const& descriptorSetInfo = _opaqueBatchProgram.GetResourceInfo ();
    DescriptorSetInfo const& descriptorSet0 = descriptorSetInfo[ 0U ];
    size_t uniqueFeatures = descriptorSet0.size ();

    std::vector<VkDescriptorBufferInfo> uniformStorage;
    std::vector<VkWriteDescriptorSet> writeStorage1;

    VkCommandBufferBeginInfo beginInfo;
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.pNext = nullptr;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    beginInfo.pInheritanceInfo = nullptr;

    bool result = android_vulkan::Renderer::CheckVkResult ( vkBeginCommandBuffer ( _geometryPassTransfer, &beginInfo ),
        "RenderSession::UpdateGPUData",
        "Can't begin geometry pass transfer command buffer"
    );

    if ( !result )
        return false;

    // Note reserve size is a of top estimation.
    size_t const estimationUniformCount = ( _maxBatchCount + _maxUniqueCount ) * opaqueCount;
    uniformStorage.reserve ( estimationUniformCount );
    writeStorage1.reserve ( estimationUniformCount );
    ++uniqueFeatures;

    std::vector<VkDescriptorPoolSize> poolSizeStorage;
    poolSizeStorage.reserve ( uniqueFeatures );

    for ( auto const &item : descriptorSet0 )
    {
        poolSizeStorage.emplace_back (
            VkDescriptorPoolSize {
                .type = item._type,
                .descriptorCount = static_cast<uint32_t> ( item._count * opaqueCount + 1U )
            }
        );
    }

    poolSizeStorage.emplace_back (
        VkDescriptorPoolSize {
            .type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
            .descriptorCount = static_cast<uint32_t> ( estimationUniformCount )
        }
    );

    VkDescriptorPoolCreateInfo poolInfo;
    poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.pNext = nullptr;
    poolInfo.flags = 0U;
    poolInfo.maxSets = static_cast<uint32_t> ( opaqueCount + 1U + estimationUniformCount );
    poolInfo.poolSizeCount = static_cast<uint32_t> ( poolSizeStorage.size () );
    poolInfo.pPoolSizes = poolSizeStorage.data ();

    DestroyDescriptorPool ( renderer );
    VkDevice device = renderer.GetDevice ();

    result = android_vulkan::Renderer::CheckVkResult (
        vkCreateDescriptorPool ( device, &poolInfo, nullptr, &_descriptorPool ),
        "RenderSession::UpdateGPUData",
        "Can't create descriptor pool"
    );

    if ( !result )
        return false;

    AV_REGISTER_DESCRIPTOR_POOL ( "RenderSession::_descriptorPool" )

    OpaqueTextureDescriptorSetLayout const opaqueTextureLayout;
    VkDescriptorSetLayout opaqueTextureLayoutNative = opaqueTextureLayout.GetLayout ();

    std::vector<VkDescriptorSetLayout> layouts;
    layouts.reserve ( static_cast<size_t> ( poolInfo.maxSets ) );

    for ( uint32_t i = 0U; i < opaqueCount; ++i )
        layouts.push_back ( opaqueTextureLayoutNative );

    TexturePresentDescriptorSetLayout const texturePresentLayout;
    layouts.emplace_back ( texturePresentLayout.GetLayout () );

    OpaqueInstanceDescriptorSetLayout const instanceLayout;
    VkDescriptorSetLayout instanceLayoutNative = instanceLayout.GetLayout ();

    for ( uint32_t i = opaqueCount + 1U; i < poolInfo.maxSets; ++i )
        layouts.push_back ( instanceLayoutNative );

    VkDescriptorSetAllocateInfo allocateInfo;
    allocateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocateInfo.pNext = nullptr;
    allocateInfo.descriptorPool = _descriptorPool;
    allocateInfo.descriptorSetCount = poolInfo.maxSets;
    allocateInfo.pSetLayouts = layouts.data ();

    descriptorSetStorage.resize ( static_cast<size_t> ( poolInfo.maxSets ) );
    VkDescriptorSet* descriptorSets = descriptorSetStorage.data ();

    result = android_vulkan::Renderer::CheckVkResult (
        vkAllocateDescriptorSets ( device, &allocateInfo, descriptorSets ),
        "RenderSession::UpdateGPUData",
        "Can't allocate descriptor sets"
    );

    if ( !result )
        return false;

    VkWriteDescriptorSet writeInfo0;
    writeInfo0.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    writeInfo0.pNext = nullptr;
    writeInfo0.descriptorCount = 1U;
    writeInfo0.dstArrayElement = 0U;
    writeInfo0.pBufferInfo = nullptr;
    writeInfo0.pTexelBufferView = nullptr;

    auto textureBinder = [ & ] ( Texture2DRef &texture,
        Texture2DRef &defaultTexture,
        uint32_t imageBindSlot,
        uint32_t samplerBindSlot
    ) {
        Texture2DRef& t = texture ? texture : defaultTexture;
        SamplerRef sampler = g_SamplerStorage.GetSampler ( t->GetMipLevelCount (), renderer );

        writeInfo0.pImageInfo = &imageStorage.emplace_back (
            VkDescriptorImageInfo {
                .sampler = sampler->GetSampler (),
                .imageView = t->GetImageView (),
                .imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
            }
        );

        writeInfo0.dstBinding = imageBindSlot;
        writeInfo0.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
        writeStorage0.push_back ( writeInfo0 );

        writeInfo0.dstBinding = samplerBindSlot;
        writeInfo0.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLER;
        writeStorage0.push_back ( writeInfo0 );
    };

    VkWriteDescriptorSet writeInfo1;
    writeInfo1.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    writeInfo1.pNext = nullptr;
    writeInfo1.dstBinding = 0U;
    writeInfo1.dstArrayElement = 0U;
    writeInfo1.descriptorCount = 1U;
    writeInfo1.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    writeInfo1.pImageInfo = nullptr;
    writeInfo1.pTexelBufferView = nullptr;

    constexpr VkPipelineStageFlags const syncFlags = AV_VK_FLAG ( VK_PIPELINE_STAGE_VERTEX_SHADER_BIT ) |
        AV_VK_FLAG ( VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT );

    auto uniformBinder = [ & ] ( VkBuffer uniformBuffer, VkDescriptorSet descriptorSet ) {
        writeInfo1.pBufferInfo = &uniformStorage.emplace_back (
            VkDescriptorBufferInfo {
                .buffer = uniformBuffer,
                .offset = 0U,
                .range = static_cast<VkDeviceSize> ( sizeof ( OpaqueProgram::InstanceData ) )
            }
        );

        writeInfo1.dstSet = descriptorSet;
        writeInfo1.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        writeStorage1.push_back ( writeInfo1 );
    };

    size_t i = 0U;

    for ( auto const &[material, call] : _opaqueCalls )
    {
        writeInfo0.dstSet = descriptorSets[ i ];
        ++i;

        // Warning less casting.
        auto& m = const_cast<OpaqueMaterial&> (
            *static_cast<OpaqueMaterial const*> ( reinterpret_cast<void const*> ( &material ) )
        );

        textureBinder ( m.GetAlbedo (), _albedoDefault, 0U, 1U );
        textureBinder ( m.GetEmission (), _emissionDefault, 2U, 3U );
        textureBinder ( m.GetMask (), _maskDefault, 4U, 5U );
        textureBinder ( m.GetNormal (), _normalDefault, 6U, 7U );
        textureBinder ( m.GetParam (), _paramDefault, 8U, 9U );
    }

    android_vulkan::Texture2D& albedo = _gBuffer.GetAlbedo ();
    SamplerRef sampler = g_SamplerStorage.GetPointSampler ( renderer );

    writeInfo0.pImageInfo = &imageStorage.emplace_back (
        VkDescriptorImageInfo {
            .sampler = sampler->GetSampler (),
            .imageView = albedo.GetImageView (),
            .imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
        }
    );

    writeInfo0.dstSet = descriptorSets[ opaqueCount ];
    writeInfo0.dstBinding = 0U;
    writeInfo0.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
    writeStorage0.push_back ( writeInfo0 );

    writeInfo0.dstBinding = 1U;
    writeInfo0.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLER;
    writeStorage0.push_back ( writeInfo0 );

    vkUpdateDescriptorSets ( device,
        static_cast<uint32_t> ( writeStorage0.size () ),
        writeStorage0.data (),
        0U,
        nullptr
    );

    OpaqueProgram::InstanceData instanceData {};
    _uniformBufferPool.Reset ();

    size_t uniformUsed = 0U;
    size_t const maxUniforms = _uniformBufferPool.GetItemCount ();
    VkDescriptorSet const* instanceDescriptorSet = descriptorSets + opaqueCount + 1U;

    for ( auto const &call : _opaqueCalls )
    {
        OpaqueCall const& opaqueCall = call.second;

        for ( auto const &[mesh, opaqueData] : opaqueCall.GetUniqueList () )
        {
            OpaqueProgram::ObjectData& objectData = instanceData._instanceData[ 0U ];

            if ( uniformUsed >= maxUniforms )
            {
                android_vulkan::LogError (
                    "RenderSession::UpdateGPUData - Uniform pool overflow has been detected (branch 1)!"
                );

                return false;
            }

            GXMat4 const& local = opaqueData._local;
            auto& uniqueOpaqueData = const_cast<OpaqueData&> ( opaqueData );

            if ( !_frustum.IsVisible ( uniqueOpaqueData._worldBounds ) )
            {
                uniqueOpaqueData._isVisible = false;
                continue;
            }

            uniqueOpaqueData._isVisible = true;
            objectData._localView.Multiply ( local, _view );
            objectData._localViewProjection.Multiply ( local, _viewProjection );

            objectData._color0 = opaqueData._color0;
            objectData._color1 = opaqueData._color1;
            objectData._color2 = opaqueData._color2;
            objectData._color3 = opaqueData._color3;

            uniformBinder (
                _uniformBufferPool.Acquire ( _geometryPassTransfer,
                    instanceData._instanceData,
                    syncFlags,
                    renderer
                ),

                instanceDescriptorSet[ uniformUsed ]
            );

            ++uniformUsed;
        }

        for ( auto const &item : opaqueCall.GetBatchList () )
        {
            MeshGroup const& group = item.second;
            size_t instanceIndex = 0U;

            for ( auto const &opaqueData : group._opaqueData )
            {
                if ( uniformUsed >= maxUniforms )
                {
                    android_vulkan::LogError (
                        "RenderSession::UpdateGPUData - Uniform pool overflow has been detected (branch 0)!"
                    );

                    return false;
                }

                if ( instanceIndex >= PBR_OPAQUE_MAX_INSTANCE_COUNT )
                {
                    uniformBinder (
                        _uniformBufferPool.Acquire ( _geometryPassTransfer,
                            instanceData._instanceData,
                            syncFlags,
                            renderer
                        ),

                        instanceDescriptorSet[ uniformUsed ]
                    );

                    instanceIndex = 0U;
                    ++uniformUsed;
                }

                GXMat4 const& local = opaqueData._local;
                auto& batchOpaqueData = const_cast<OpaqueData&> ( opaqueData );

                if ( !_frustum.IsVisible ( batchOpaqueData._worldBounds ) )
                {
                    batchOpaqueData._isVisible = false;
                    continue;
                }

                OpaqueProgram::ObjectData& objectData = instanceData._instanceData[ instanceIndex ];
                ++instanceIndex;
                batchOpaqueData._isVisible = true;

                objectData._localView.Multiply ( local, _view );
                objectData._localViewProjection.Multiply ( local, _viewProjection );

                objectData._color0 = opaqueData._color0;
                objectData._color1 = opaqueData._color1;
                objectData._color2 = opaqueData._color2;
                objectData._color3 = opaqueData._color3;
            }

            if ( !instanceIndex )
                continue;

            uniformBinder (
                _uniformBufferPool.Acquire ( _geometryPassTransfer,
                    instanceData._instanceData,
                    syncFlags,
                    renderer
                ),

                instanceDescriptorSet[ uniformUsed ]
            );

            ++uniformUsed;
        }
    }

    vkUpdateDescriptorSets ( device,
        static_cast<uint32_t> ( writeStorage1.size () ),
        writeStorage1.data (),
        0U,
        nullptr
    );

    result = android_vulkan::Renderer::CheckVkResult ( vkEndCommandBuffer ( _geometryPassTransfer ),
        "RenderSession::UpdateGPUData",
        "Can't end transfer command buffer"
    );

    if ( !result )
        return false;

    return android_vulkan::Renderer::CheckVkResult (
        vkQueueSubmit ( renderer.GetQueue (), 1U, &_submitInfoTransfer, VK_NULL_HANDLE ),
        "RenderSession::UpdateGPUData",
        "Can't submit geometry transfer command buffer"
    );
}

void RenderSession::UpdatePointLightShadowMaps ()
{
    //for ( auto &pointLightCall : _pointLightCalls )
    //{
    //    for ( auto const& opaqueCall : _opaqueCalls )
    //    {
    //        OpaqueCall const& opaque = opaqueCall.second;
//
    //        for ( auto const &[name, meshGroup] : opaque.GetBatchList () )
    //        {
    //            for ( auto const& opaqueData : meshGroup._opaqueData )
    //            {
    //                // TODO
    //                android_vulkan::LogDebug ( "%p", &opaqueData );
    //            }
    //        }
//
    //        for ( auto const &[mesh, opaqueData] : opaque.GetUniqueList () )
    //        {
    //            // TODO
    //            android_vulkan::LogDebug ( "%p", &opaqueData );
    //        }
    //    }
//
    //    // TODO
    //    android_vulkan::LogDebug ( "%p", &pointLightCall );
    //}
}

} // namespace pbr
