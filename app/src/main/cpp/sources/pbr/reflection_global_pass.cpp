#include <pbr/reflection_global_pass.h>

GX_DISABLE_COMMON_WARNINGS

#include <cassert>

GX_RESTORE_WARNING_STATE


namespace pbr {

ReflectionGlobalPass::ReflectionGlobalPass () noexcept:
    _descriptorPool ( VK_NULL_HANDLE ),
    _descriptorSets {},
    _imageInfo {},
    _prefilters {},
    _program {},
    _writeSets {}
{
    // NOTHING
}

void ReflectionGlobalPass::Append ( TextureCubeRef &prefilter )
{
    _prefilters.push_back ( prefilter );
}

bool ReflectionGlobalPass::Execute ( android_vulkan::Renderer &renderer,
    VkCommandBuffer commandBuffer,
    GXMat4 const &viewToWorld
)
{
    size_t const count = _prefilters.size ();

    if ( !count )
        return true;

    if ( !UpdateGPUData ( renderer, count, viewToWorld ) )
        return false;

    _program.Bind ( commandBuffer );

    for ( size_t i = 0U; i < count; ++i )
    {
        _program.SetDescriptorSet ( commandBuffer, _descriptorSets[ i ] );
        vkCmdDraw ( commandBuffer, 4U, 1U, 0U, 0U );
    }

    return true;
}

bool ReflectionGlobalPass::Init ( android_vulkan::Renderer &renderer,
    VkRenderPass renderPass,
    uint32_t subpass,
    VkExtent2D const &viewport
)
{
    if ( _program.Init ( renderer, renderPass, subpass, viewport ) )
        return true;

    Destroy ( renderer.GetDevice () );
    return false;
}

void ReflectionGlobalPass::Destroy ( VkDevice device )
{
    DestroyDescriptorPool ( device );
    _descriptorSets.clear ();
    _imageInfo.clear ();
    _writeSets.clear ();

    _program.Destroy ( device );
}

size_t ReflectionGlobalPass::GetReflectionCount () const
{
    return _prefilters.size ();
}

void ReflectionGlobalPass::Reset ()
{
    _prefilters.clear ();
}

bool ReflectionGlobalPass::AllocateDescriptorSets ( android_vulkan::Renderer &renderer, size_t neededSets )
{
    assert ( neededSets );

    if ( _descriptorSets.size () >= neededSets )
        return true;

    VkDevice device = renderer.GetDevice ();
    DestroyDescriptorPool ( device );
    auto const size = static_cast<uint32_t> ( neededSets );

    VkDescriptorPoolSize const poolSizes[] =
    {
        {
            .type = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,
            .descriptorCount = size
        }
    };

    VkDescriptorPoolCreateInfo const poolInfo
    {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0U,
        .maxSets = size,
        .poolSizeCount = static_cast<uint32_t> ( std::size ( poolSizes ) ),
        .pPoolSizes = poolSizes
    };

    bool result = android_vulkan::Renderer::CheckVkResult (
        vkCreateDescriptorPool ( device, &poolInfo, nullptr, &_descriptorPool ),
        "pbr::ReflectionGlobalPass::AllocateDescriptorSets",
        "Can't create descriptor pool"
    );

    if ( !result )
        return false;

    AV_REGISTER_DESCRIPTOR_POOL ( "ReflectionGlobalPass::_descriptorPool" )

    _descriptorSets.resize ( neededSets, VK_NULL_HANDLE );

    ReflectionGlobalDescriptorSetLayout const layout;
    std::vector<VkDescriptorSetLayout> const layouts ( neededSets, layout.GetLayout () );

    VkDescriptorSetAllocateInfo const allocateInfo
    {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
        .pNext = nullptr,
        .descriptorPool = _descriptorPool,
        .descriptorSetCount = poolInfo.maxSets,
        .pSetLayouts = layouts.data ()
    };

    result = android_vulkan::Renderer::CheckVkResult (
        vkAllocateDescriptorSets ( device, &allocateInfo, _descriptorSets.data () ),
        "pbr::ReflectionGlobalPass::AllocateDescriptorSets",
        "Can't allocate descriptor sets"
    );

    if ( !result )
        return false;

    // Initialize all immutable constant fields.

    VkDescriptorImageInfo const image
    {
        .sampler = VK_NULL_HANDLE,
        .imageView = VK_NULL_HANDLE,
        .imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
    };

    _imageInfo.resize ( neededSets, image );

    constexpr VkWriteDescriptorSet writeSet
    {
        .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
        .pNext = nullptr,
        .dstSet = VK_NULL_HANDLE,
        .dstBinding = 0U,
        .dstArrayElement = 0U,
        .descriptorCount = 1U,
        .descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,
        .pImageInfo = nullptr,
        .pBufferInfo = nullptr,
        .pTexelBufferView = nullptr
    };

    _writeSets.resize ( neededSets, writeSet );

    for ( size_t i = 0U; i < neededSets; ++i )
    {
        VkWriteDescriptorSet& write = _writeSets[ i ];
        write.dstSet = _descriptorSets[ i ];
        write.pImageInfo = &_imageInfo[ i ];
    }

    // Now all what is needed to do is to init "_imageInfo::imageView" data.
    // Then to invoke vkUpdateDescriptorSets.
    return true;
}

void ReflectionGlobalPass::DestroyDescriptorPool ( VkDevice device )
{
    if ( _descriptorPool == VK_NULL_HANDLE )
        return;

    vkDestroyDescriptorPool ( device, _descriptorPool, nullptr );
    _descriptorPool = VK_NULL_HANDLE;
    AV_UNREGISTER_DESCRIPTOR_POOL ( "ReflectionGlobalPass::_descriptorPool" )
}

bool ReflectionGlobalPass::UpdateGPUData ( android_vulkan::Renderer &renderer, size_t count, GXMat4 const &/*viewToWorld*/ )
{
    if ( !AllocateDescriptorSets ( renderer, count ) )
        return false;

    for ( size_t i = 0U; i < count; ++i )
        _imageInfo[ i ].imageView = _prefilters[ i ]->GetImageView ();

    vkUpdateDescriptorSets ( renderer.GetDevice (),
        static_cast<uint32_t> ( _writeSets.size() ),
        _writeSets.data (),
        0U,
        nullptr
    );

    return true;
}

} // namespace pbr
