#include <pbr/geometry_pass.h>
#include <vulkan_utils.h>


namespace pbr {

bool GeometryPass::Init ( android_vulkan::Renderer &renderer,
    VkCommandPool commandPool,
    VkExtent2D const &resolution,
    VkRenderPass renderPass,
    SamplerManager &samplerManager
) noexcept
{
    VkDevice device = renderer.GetDevice ();

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

    bool result = android_vulkan::Renderer::CheckVkResult (
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
}

bool GeometryPass::Execute ( android_vulkan::Renderer &renderer,
    VkCommandBuffer commandBuffer,
    GXProjectionClipPlanes const &frustum,
    GXMat4 const &view,
    GXMat4 const &viewProjection,
    DefaultTextureManager const &defaultTextureManager,
    RenderSessionStats &renderSessionStats
) noexcept
{
    bool isSamplerUsed = false;

    bool const result = _opaqueSubpass.Execute ( renderer,
        commandBuffer,
        frustum,
        view,
        viewProjection,
        defaultTextureManager,
        renderSessionStats,
        _descriptorSet,
        isSamplerUsed
    );

    if ( !result )
        return false;

    return _stippleSubpass.Execute ( renderer,
        commandBuffer,
        view,
        viewProjection,
        defaultTextureManager,
        renderSessionStats,
        _descriptorSet,
        isSamplerUsed
    );
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

} // namespace pbr
