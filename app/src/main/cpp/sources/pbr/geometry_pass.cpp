#include <precompiled_headers.hpp>
#include <pbr/geometry_pass.hpp>
#include <trace.hpp>
#include <vulkan_utils.hpp>


namespace pbr {

bool GeometryPass::Init ( android_vulkan::Renderer &renderer,
    VkExtent2D const &resolution,
    VkRenderPass renderPass,
    SamplerManager &samplerManager,
    DefaultTextureManager const &defaultTextureManager
) noexcept
{
    VkDevice device = renderer.GetDevice ();

    if ( !_descriptorSetLayout.Init ( device ) ) [[unlikely]]
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

    if ( !result ) [[unlikely]]
        return false;

    AV_SET_VULKAN_OBJECT_NAME ( device, _descriptorPool, VK_OBJECT_TYPE_DESCRIPTOR_POOL, "Geometry pass" )

    VkDescriptorSetAllocateInfo const setAllocateInfo
    {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
        .pNext = nullptr,
        .descriptorPool = _descriptorPool,
        .descriptorSetCount = 1U,
        .pSetLayouts = &_descriptorSetLayout.GetLayout ()
    };

    result = android_vulkan::Renderer::CheckVkResult (
        vkAllocateDescriptorSets ( device, &setAllocateInfo, &_descriptorSet ),
        "pbr::GeometryPass::Init",
        "Can't allocate descriptor set"
    );

    if ( !result ) [[unlikely]]
        return false;

    AV_SET_VULKAN_OBJECT_NAME ( device, _descriptorSet, VK_OBJECT_TYPE_DESCRIPTOR_SET, "Geometry pass sampler" )

    VkDescriptorImageInfo const samplerInfo
    {
        .sampler = samplerManager.GetMaterialSampler (),
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

    return _opaqueSubpass.Init ( renderer, resolution, renderPass ) &&
        _stippleSubpass.Init ( renderer, resolution, renderPass ) &&
        _geometryPool.Init ( renderer ) &&
        _materialPool.Init ( device, defaultTextureManager );
}

void GeometryPass::Destroy ( android_vulkan::Renderer &renderer ) noexcept
{
    VkDevice device = renderer.GetDevice ();

    _descriptorSetLayout.Destroy ( device );
    _stippleSubpass.Destroy ( device );
    _opaqueSubpass.Destroy ( device );
    _geometryPool.Destroy ( renderer );
    _materialPool.Destroy ( device );

    if ( _descriptorPool == VK_NULL_HANDLE )
        return;

    vkDestroyDescriptorPool ( device, _descriptorPool, nullptr );
    _descriptorPool = VK_NULL_HANDLE;
}

void GeometryPass::Execute ( VkCommandBuffer commandBuffer, RenderSessionStats &renderSessionStats ) noexcept
{
    AV_TRACE ( "Geometry pass: Execute" )
    AV_VULKAN_GROUP ( commandBuffer, "Geometry pass" )

    bool isSamplerUsed = false;

    _opaqueSubpass.Execute ( commandBuffer,
        _geometryPool,
        _materialPool,
        renderSessionStats,
        _descriptorSet,
        isSamplerUsed
    );

    _stippleSubpass.Execute ( commandBuffer,
        _geometryPool,
        _materialPool,
        renderSessionStats,
        _descriptorSet,
        isSamplerUsed
    );

    _geometryPool.Commit ();
    _materialPool.Commit ();
}

OpaqueSubpass &GeometryPass::GetOpaqueSubpass () noexcept
{
    return _opaqueSubpass;
}

StippleSubpass &GeometryPass::GetStippleSubpass () noexcept
{
    return _stippleSubpass;
}

void GeometryPass::Reset () noexcept
{
    _opaqueSubpass.Reset ();
    _stippleSubpass.Reset ();
}

void GeometryPass::UploadGPUData ( VkDevice device,
    VkCommandBuffer commandBuffer,
    GXProjectionClipPlanes const &frustum,
    GXMat4 const &view,
    GXMat4 const &viewProjection
) noexcept
{
    AV_TRACE ( "Geometry pass: Upload GPU data" )
    AV_VULKAN_GROUP ( commandBuffer, "Upload geometry data" )

    _opaqueSubpass.UpdateGPUData ( commandBuffer,
        _geometryPool,
        _materialPool,
        frustum,
        view,
        viewProjection
    );

    _stippleSubpass.UpdateGPUData ( commandBuffer,
        _geometryPool,
        _materialPool,
        view,
        viewProjection
    );

    _geometryPool.IssueSync ( device, commandBuffer );
    _materialPool.IssueSync ( device );
}

} // namespace pbr
