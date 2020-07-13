#include <pbr/render_session.h>

GX_DISABLE_COMMON_WARNINGS

#include <cassert>

GX_RESTORE_WARNING_STATE

#include <vulkan_utils.h>


namespace pbr {

constexpr static const size_t DEFAULT_TEXTURE_COUNT = 4U;
constexpr static size_t GBUFFER_ATTACHMENT_COUNT = 5U;

// 1 2 4 8 16 32 64 128 256 512 1024 2048 4096
constexpr static size_t MAX_SUPPORTED_MIP_COUNT = 13U;

//----------------------------------------------------------------------------------------------------------------------

class SamplerStorage final
{
    private:
        SamplerRef      _storage[ MAX_SUPPORTED_MIP_COUNT ];

    public:
        SamplerStorage () = default;
        ~SamplerStorage () = default;

        SamplerStorage ( const SamplerStorage &other ) = delete;
        SamplerStorage& operator = ( const SamplerStorage &other ) = delete;

        [[maybe_unused]] void FreeResources ( android_vulkan::Renderer &renderer );

        [[maybe_unused]] [[nodiscard]] SamplerRef GetSampler ( Texture2DRef &texture,
            android_vulkan::Renderer &renderer
        );
};

void SamplerStorage::FreeResources ( android_vulkan::Renderer &renderer )
{
    for ( auto& sampler : _storage )
    {
        if ( !sampler )
            continue;

        sampler->Destroy( renderer );
        sampler = nullptr;
    }
}

SamplerRef SamplerStorage::GetSampler ( Texture2DRef &texture, android_vulkan::Renderer &renderer )
{
    const uint8_t mips = texture->GetMipLevelCount ();
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

RenderSession::RenderSession ():
    _albedoDefault {},
    _emissionDefault {},
    _normalDefault {},
    _paramDefault {},
    _commandPool ( VK_NULL_HANDLE ),
    _gBuffer {},
    _gBufferFramebuffer ( VK_NULL_HANDLE ),
    _gBufferRenderPass ( VK_NULL_HANDLE ),
    _isFreeTransferResources ( false ),
    _meshCount ( 0U ),
    _opaqueCalls {},
    _opaqueProgram {},
    _texturePresentProgram {},
    _viewProjection {}
{
    // NOTHING
}

void RenderSession::SubmitMesh ( MeshRef &mesh,
    MaterialRef &material,
    const GXMat4 &local
)
{
    ++_meshCount;

    if ( material->GetMaterialType() != eMaterialType::Opaque )
        return;

    SubmitOpaqueCall ( mesh, material, local );
}

void RenderSession::Begin ( const GXMat4 &view, const GXMat4 &projection )
{
    _meshCount = 0U;
    _viewProjection.Multiply ( view, projection );
}

void RenderSession::End ( ePresentTarget /*target*/, android_vulkan::Renderer &renderer )
{
    if ( _isFreeTransferResources )
    {
        if ( _albedoDefault )
            _albedoDefault->FreeTransferResources ( renderer );

        if ( _emissionDefault )
            _emissionDefault->FreeTransferResources ( renderer );

        if ( _normalDefault )
            _normalDefault->FreeTransferResources ( renderer );

        if ( _paramDefault )
            _paramDefault->FreeTransferResources ( renderer );

        _isFreeTransferResources = false;
    }

    // TODO

    _opaqueCalls.clear ();

    assert ( !"RenderSession::End - Implement me!" );
}

const VkExtent2D& RenderSession::GetResolution () const
{
    return _gBuffer.GetResolution ();
}

bool RenderSession::Init ( android_vulkan::Renderer &renderer, VkRenderPass presentRenderPass )
{
    VkDevice device = renderer.GetDevice ();

    if ( !_gBuffer.Init ( renderer.GetViewportResolution (), renderer ) )
        return false;

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

    if ( !_opaqueProgram.Init ( renderer, _gBufferRenderPass, _gBuffer.GetResolution () ) )
    {
        Destroy ( renderer );
        return false;
    }

    if ( !_texturePresentProgram.Init ( renderer, presentRenderPass, renderer.GetSurfaceSize () ) )
    {
        Destroy ( renderer );
        return false;
    }

    VkCommandPoolCreateInfo info;
    info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    info.pNext = nullptr;
    info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    info.queueFamilyIndex = renderer.GetQueueFamilyIndex ();

    bool result = renderer.CheckVkResult ( vkCreateCommandPool ( device, &info, nullptr, &_commandPool ),
        "RenderSession::Init",
        "Can't create command pool"
    );

    if ( !result )
    {
        Destroy ( renderer );
        return false;
    }

    AV_REGISTER_COMMAND_POOL ( "RenderSession::_commandPool" )

    VkCommandBuffer commandBuffers[ DEFAULT_TEXTURE_COUNT ];
    VkCommandBufferAllocateInfo allocateInfo;
    allocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocateInfo.pNext = nullptr;
    allocateInfo.commandPool = _commandPool;
    allocateInfo.commandBufferCount = static_cast<uint32_t> ( std::size ( commandBuffers ) );
    allocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;

    result = renderer.CheckVkResult ( vkAllocateCommandBuffers ( device, &allocateInfo, commandBuffers ),
        "RenderSession::Init",
        "Can't allocate command buffers"
    );

    if ( !result )
    {
        Destroy ( renderer );
        return false;
    }

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

    const android_vulkan::Half emission[ 4U ] =
    {
        android_vulkan::Half ( 0.0F ),
        android_vulkan::Half ( 0.0F ),
        android_vulkan::Half ( 0.0F ),
        android_vulkan::Half ( 0.0F )
    };

    result = textureLoader ( _emissionDefault,
        reinterpret_cast<const uint8_t*> ( emission ),
        sizeof ( emission ),
        VK_FORMAT_R16G16B16A16_SFLOAT,
        commandBuffers[ 1U ]
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
        commandBuffers[ 2U ]
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
        commandBuffers[ 3U ]
    );

    if ( result )
        return true;

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
    freeTexture ( _emissionDefault );
    freeTexture ( _albedoDefault );

    VkDevice device = renderer.GetDevice ();

    if ( _commandPool != VK_NULL_HANDLE )
    {
        vkDestroyCommandPool ( device, _commandPool, nullptr );
        _commandPool = VK_NULL_HANDLE;
        AV_UNREGISTER_COMMAND_POOL ( "RenderSession::_commandPool" )
    }

    _texturePresentProgram.Destroy ( renderer );
    _opaqueProgram.Destroy ( renderer );

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

    _gBuffer.Destroy ( renderer );
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

    const bool result = renderer.CheckVkResult (
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
    constexpr const size_t colorAttachmentCount = GBUFFER_ATTACHMENT_COUNT - 1U;

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

    const bool result = renderer.CheckVkResult (
        vkCreateRenderPass ( renderer.GetDevice (), &renderPassInfo, nullptr, &_gBufferRenderPass ),
        "RenderSession::CreateGBufferRenderPass",
        "Can't create GBuffer render pass"
    );

    if ( !result )
        return false;

    AV_REGISTER_RENDER_PASS ( "RenderSession::_gBufferRenderPass" )
    return true;
}

void RenderSession::SubmitOpaqueCall ( MeshRef &mesh, MaterialRef &material, const GXMat4 &local )
{
    auto& opaqueMaterial = *dynamic_cast<OpaqueMaterial*> ( material.get () );
    auto findResult = _opaqueCalls.find ( opaqueMaterial );

    if ( findResult != _opaqueCalls.cend () )
    {
        findResult->second.Append ( mesh, local );
        return;
    }

    _opaqueCalls.insert ( std::make_pair ( opaqueMaterial, OpaqueCall ( mesh, local ) ) );
}

} // namespace pbr
