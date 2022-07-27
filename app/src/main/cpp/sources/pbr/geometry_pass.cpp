#include <pbr/geometry_pass.h>
#include <vulkan_utils.h>


namespace pbr {

bool GeometryPass::Init ( android_vulkan::Renderer &renderer,
    VkCommandPool commandPool,
    VkExtent2D const &resolution,
    VkRenderPass renderPass,
    VkFramebuffer framebuffer,
    SamplerManager &samplerManager
) noexcept
{
    VkDevice device = renderer.GetDevice ();

    constexpr VkFenceCreateInfo fenceInfo
    {
        .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
        .pNext = nullptr,
        .flags = VK_FENCE_CREATE_SIGNALED_BIT
    };

    bool result = android_vulkan::Renderer::CheckVkResult ( vkCreateFence ( device, &fenceInfo, nullptr, &_fence ),
        "pbr::GeometryPass::Init",
        "Can't create fence"
    );

    if ( !result )
        return false;

    AV_REGISTER_FENCE ( "pbr::GeometryPass::_fence" )

    if ( !_descriptorSetLayout.Init ( device ) )
        return false;

    constexpr VkDescriptorPoolSize poolSize
    {
        .type = VK_DESCRIPTOR_TYPE_SAMPLER,
        .descriptorCount = 1U
    };

    VkDescriptorPoolCreateInfo const poolInfo
    {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0U,
        .maxSets = 1U,
        .poolSizeCount = 1U,
        .pPoolSizes = &poolSize
    };

    result = android_vulkan::Renderer::CheckVkResult (
        vkCreateDescriptorPool ( device, &poolInfo, nullptr, &_descriptorPool ),
        "pbr::GeometryPass::Init",
        "Can't create descriptor pool"
    );

    if ( !result )
        return false;

    AV_REGISTER_DESCRIPTOR_POOL ( "pbr::GeometryPass::_descriptorPool" )

    VkDescriptorSetLayout layout = _descriptorSetLayout.GetLayout ();

    VkDescriptorSetAllocateInfo const setAllocateInfo
    {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
        .pNext = nullptr,
        .descriptorPool = _descriptorPool,
        .descriptorSetCount = 1U,
        .pSetLayouts = &layout
    };

    result = android_vulkan::Renderer::CheckVkResult (
        vkAllocateDescriptorSets ( device, &setAllocateInfo, &_descriptorSet ),
        "pbr::GeometryPass::Init",
        "Can't allocate descriptor set"
    );

    if ( !result )
        return false;

    VkDescriptorImageInfo const samplerInfo
    {
        .sampler = samplerManager.GetMaterialSampler ()->GetSampler (),
        .imageView = VK_NULL_HANDLE,
        .imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
    };

    VkWriteDescriptorSet const writeSet
    {
        .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
        .pNext = nullptr,
        .dstSet = _descriptorSet,
        .dstBinding = 0U,
        .dstArrayElement = 0U,
        .descriptorCount = 1U,
        .descriptorType = VK_DESCRIPTOR_TYPE_SAMPLER,
        .pImageInfo = &samplerInfo,
        .pBufferInfo = nullptr,
        .pTexelBufferView = nullptr
    };

    vkUpdateDescriptorSets ( device, 1U, &writeSet, 0U, nullptr );

    VkCommandBufferAllocateInfo const bufferAllocateInfo
    {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
        .pNext = nullptr,
        .commandPool = commandPool,
        .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
        .commandBufferCount = 1U
    };

    result = android_vulkan::Renderer::CheckVkResult (
        vkAllocateCommandBuffers ( device, &bufferAllocateInfo, &_commandBuffer ),
        "pbr::GeometryPass::Init",
        "Can't allocate command buffers"
    );

    if ( !result )
        return false;

    constexpr static VkClearValue const clearValues[] =
    {
        // albedo
        {
            .color
            {
                .float32 = { 0.0F, 0.0F, 0.0F, 0.0F }
            }
        },

        // emission
        {
            .color
            {
                .float32 = { 0.0F, 0.0F, 0.0F, 0.0F }
            }
        },

        // normal
        {
            .color
            {
                .float32 = { 0.5F, 0.5F, 0.5F, 0.0F }
            }
        },

        // param
        {
            .color
            {
                .float32 = { 0.5F, 0.5F, 0.5F, 0.0F }
            }
        },

        // depth|stencil
        {
            .depthStencil
            {
                .depth = 0.0F,
                .stencil = 0U
            }
        }
    };

    _renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    _renderPassInfo.pNext = nullptr;
    _renderPassInfo.renderPass = renderPass;
    _renderPassInfo.framebuffer = framebuffer;
    _renderPassInfo.clearValueCount = static_cast<uint32_t> ( std::size ( clearValues ) );
    _renderPassInfo.pClearValues = clearValues;
    _renderPassInfo.renderArea.offset.x = 0;
    _renderPassInfo.renderArea.offset.y = 0;
    _renderPassInfo.renderArea.extent = resolution;

    if ( !_opaqueSubpass.Init ( renderer, commandPool, resolution, renderPass ) )
        return false;

    return _stippleSubpass.Init ( renderer, commandPool, resolution, renderPass );
}

void GeometryPass::Destroy ( VkDevice device ) noexcept
{
    _descriptorSetLayout.Destroy ( device );
    _stippleSubpass.Destroy ( device );
    _opaqueSubpass.Destroy ( device );

    if ( _descriptorPool != VK_NULL_HANDLE )
    {
        vkDestroyDescriptorPool ( device, _descriptorPool, nullptr );
        AV_UNREGISTER_DESCRIPTOR_POOL ( "pbr::GeometryPass::_descriptorPool" )
        _descriptorPool = VK_NULL_HANDLE;
    }

    if ( _fence == VK_NULL_HANDLE )
        return;

    vkDestroyFence ( device, _fence, nullptr );
    _fence = VK_NULL_HANDLE;
    AV_UNREGISTER_FENCE ( "pbr::GeometryPass::_fence" )
}

VkCommandBuffer GeometryPass::Execute ( android_vulkan::Renderer &renderer,
    GXProjectionClipPlanes const &frustum,
    GXMat4 const &view,
    GXMat4 const &viewProjection,
    DefaultTextureManager const &defaultTextureManager,
    RenderSessionStats &renderSessionStats
) noexcept
{
    if ( !BeginRenderPass () )
        return VK_NULL_HANDLE;

    bool isSamplerUsed = false;

    bool result = _opaqueSubpass.Execute ( renderer,
        _commandBuffer,
        frustum,
        view,
        viewProjection,
        defaultTextureManager,
        renderSessionStats,
        _descriptorSet,
        isSamplerUsed
    );

    if ( !result )
        return VK_NULL_HANDLE;

    result = _stippleSubpass.Execute ( renderer,
        _commandBuffer,
        view,
        viewProjection,
        defaultTextureManager,
        renderSessionStats,
        _descriptorSet,
        isSamplerUsed
    );

    if ( !result )
        return VK_NULL_HANDLE;

    vkCmdEndRenderPass ( _commandBuffer );
    return _commandBuffer;
}

VkFence GeometryPass::GetFence () const noexcept
{
    return _fence;
}

OpaqueSubpass& GeometryPass::GetOpaqueSubpass () noexcept
{
    return _opaqueSubpass;
}

StippleSubpass& GeometryPass::GetStippleSubpass () noexcept
{
    return _stippleSubpass;
}

void GeometryPass::Reset () noexcept
{
    _opaqueSubpass.Reset ();
    _stippleSubpass.Reset ();
}

bool GeometryPass::WaitReady ( android_vulkan::Renderer &renderer ) const noexcept
{
    VkDevice device = renderer.GetDevice ();

    bool const result = android_vulkan::Renderer::CheckVkResult (
        vkWaitForFences ( device, 1U, &_fence, VK_TRUE, std::numeric_limits<uint64_t>::max () ),
        "pbr::GeometryPass::WaitReady",
        "Can't wait fence"
    );

    if ( !result )
        return false;

    return android_vulkan::Renderer::CheckVkResult ( vkResetFences ( device, 1U, &_fence ),
        "pbr::GeometryPass::WaitReady",
        "Can't reset fence"
    );
}

VkSubpassDescription GeometryPass::GetSubpassDescription () noexcept
{
    constexpr static VkAttachmentReference const colorReferences[] =
    {
        // #0: albedo
        {
            .attachment = 0U,
            .layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
        },

        // #1: emission
        {
            .attachment = 1U,
            .layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
        },

        // #2: normal
        {
            .attachment = 2U,
            .layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
        },

        // #3: params
        {
            .attachment = 3U,
            .layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
        }
    };

    // depth|stencil
    constexpr static VkAttachmentReference depthStencilReference
    {
        .attachment = 4U,
        .layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL
    };

    return
    {
        .flags = 0U,
        .pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS,
        .inputAttachmentCount = 0U,
        .pInputAttachments = nullptr,
        .colorAttachmentCount = static_cast<uint32_t> ( std::size ( colorReferences ) ),
        .pColorAttachments = colorReferences,
        .pResolveAttachments = nullptr,
        .pDepthStencilAttachment = &depthStencilReference,
        .preserveAttachmentCount = 0U,
        .pPreserveAttachments = nullptr
    };
}

bool GeometryPass::BeginRenderPass () noexcept
{
    constexpr VkCommandBufferBeginInfo beginInfo
    {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
        .pNext = nullptr,
        .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
        .pInheritanceInfo = nullptr
    };

    bool const result = android_vulkan::Renderer::CheckVkResult ( vkBeginCommandBuffer ( _commandBuffer, &beginInfo ),
        "pbr::GeometryPass::BeginRenderPass",
        "Can't begin rendering command buffer"
    );

    if ( !result )
        return false;

    vkCmdBeginRenderPass ( _commandBuffer, &_renderPassInfo, VK_SUBPASS_CONTENTS_INLINE );
    return true;
}

} // namespace pbr
