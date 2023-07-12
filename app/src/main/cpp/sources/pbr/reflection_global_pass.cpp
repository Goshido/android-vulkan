#include <pbr/reflection_global_pass.h>
#include <trace.h>


namespace pbr {

constexpr static size_t FRAMES = 5U;
constexpr static size_t REFLECTION_PER_FRAME = 16384U;
constexpr static size_t REFLECTIONS = FRAMES * REFLECTION_PER_FRAME;

//----------------------------------------------------------------------------------------------------------------------

void ReflectionGlobalPass::Append ( TextureCubeRef &prefilter ) noexcept
{
    _prefilters.push_back ( prefilter );
}

void ReflectionGlobalPass::Execute ( VkDevice device, VkCommandBuffer commandBuffer ) noexcept
{
    size_t const count = _prefilters.size ();

    if ( !count )
        return;

    UpdateGPUData ( device, count );

    _program.Bind ( commandBuffer );

    for ( size_t i = 0U; i < count; ++i )
    {
        _program.SetDescriptorSet ( commandBuffer, _descriptorSets[ _itemReadIndex ] );
        vkCmdDraw ( commandBuffer, 4U, 1U, 0U, 0U );
        _itemReadIndex = ( _itemReadIndex + 1U ) % REFLECTIONS;
    }

    _itemBaseIndex = _itemWriteIndex;
    _itemReadIndex = _itemWriteIndex;
}

bool ReflectionGlobalPass::Init ( android_vulkan::Renderer &renderer,
    VkRenderPass renderPass,
    uint32_t subpass,
    VkExtent2D const &viewport
) noexcept
{
    return _program.Init ( renderer, renderPass, subpass, viewport ) &&
        AllocateDescriptorSets ( renderer.GetDevice () );
}

void ReflectionGlobalPass::Destroy ( VkDevice device ) noexcept
{
    if ( _descriptorPool != VK_NULL_HANDLE )
    {
        vkDestroyDescriptorPool ( device, _descriptorPool, nullptr );
        _descriptorPool = VK_NULL_HANDLE;
        AV_UNREGISTER_DESCRIPTOR_POOL ( "pbr::ReflectionGlobalPass::_descriptorPool" )
    }

    auto const clean = [] ( auto &vector ) noexcept {
        vector.clear ();
        vector.shrink_to_fit ();
    };

    clean ( _descriptorSets );
    clean ( _imageInfo );
    clean ( _writeSets );

    _program.Destroy ( device );
}

size_t ReflectionGlobalPass::GetReflectionCount () const noexcept
{
    return _prefilters.size ();
}

void ReflectionGlobalPass::Reset () noexcept
{
    AV_TRACE ( "Reflection global reset" )
    _prefilters.clear ();
}

bool ReflectionGlobalPass::AllocateDescriptorSets ( VkDevice device ) noexcept
{
    constexpr static VkDescriptorPoolSize poolSizes
    {
        .type = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,
        .descriptorCount = static_cast<uint32_t> ( REFLECTIONS )
    };

    constexpr VkDescriptorPoolCreateInfo poolInfo
    {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0U,
        .maxSets = static_cast<uint32_t> ( REFLECTIONS ),
        .poolSizeCount = 1U,
        .pPoolSizes = &poolSizes
    };

    bool result = android_vulkan::Renderer::CheckVkResult (
        vkCreateDescriptorPool ( device, &poolInfo, nullptr, &_descriptorPool ),
        "pbr::ReflectionGlobalPass::AllocateDescriptorSets",
        "Can't create descriptor pool"
    );

    if ( !result )
        return false;

    AV_REGISTER_DESCRIPTOR_POOL ( "pbr::ReflectionGlobalPass::_descriptorPool" )

    _descriptorSets.resize ( REFLECTIONS );

    std::vector<VkDescriptorSetLayout> const layouts ( REFLECTIONS,
        ReflectionGlobalDescriptorSetLayout ().GetLayout ()
    );

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

    _imageInfo.resize ( REFLECTIONS, image );

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

    _writeSets.resize ( REFLECTIONS, writeSet );

    for ( size_t i = 0U; i < REFLECTIONS; ++i )
    {
        VkWriteDescriptorSet &write = _writeSets[ i ];
        write.dstSet = _descriptorSets[ i ];
        write.pImageInfo = &_imageInfo[ i ];
    }

    // Now all what is needed to do is to init "_imageInfo::imageView".
    // Then to invoke vkUpdateDescriptorSets.

    _itemBaseIndex = 0U;
    _itemReadIndex = 0U;
    _itemWriteIndex = 0U;

    return true;
}

void ReflectionGlobalPass::UpdateGPUData ( VkDevice device, size_t count ) noexcept
{
    for ( size_t i = 0U; i < count; ++i )
    {
        _imageInfo[ _itemWriteIndex ].imageView = _prefilters[ i ]->GetImageView ();
        _itemWriteIndex = ( _itemWriteIndex + 1U ) % REFLECTIONS;
    }

    size_t const idx = _itemBaseIndex + count;
    size_t const cases[] = { 0U, idx - REFLECTIONS };
    size_t const more = cases[ static_cast<size_t> ( idx > REFLECTIONS ) ];
    size_t const available = count - more;

    VkWriteDescriptorSet const* writeSets = _writeSets.data ();
    vkUpdateDescriptorSets ( device, static_cast<uint32_t> ( available ), writeSets + _itemBaseIndex, 0U, nullptr );

    if ( more > 0U )
    {
        vkUpdateDescriptorSets ( device, static_cast<uint32_t> ( more ), writeSets, 0U, nullptr );
    }
}

} // namespace pbr
