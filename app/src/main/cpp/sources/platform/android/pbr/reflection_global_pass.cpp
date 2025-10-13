#include <precompiled_headers.hpp>
#include <platform/android/pbr/reflection_global_pass.hpp>
#include <trace.hpp>


namespace pbr {

namespace {

constexpr size_t FRAMES = 5U;
constexpr size_t REFLECTION_PER_FRAME = 16384U;
constexpr size_t REFLECTIONS = FRAMES * REFLECTION_PER_FRAME;

} // end of anonymous namespace

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
        vkCmdDraw ( commandBuffer, 3U, 1U, 0U, 0U );
        _itemReadIndex = ( _itemReadIndex + 1U ) % REFLECTIONS;
    }

    _itemBaseIndex = _itemWriteIndex;
    _itemReadIndex = _itemWriteIndex;
}

bool ReflectionGlobalPass::Init ( android_vulkan::Renderer &renderer,
    VkRenderPass renderPass,
    VkExtent2D const &viewport
) noexcept
{
    return _program.Init ( renderer, renderPass, viewport ) && AllocateDescriptorSets ( renderer.GetDevice () );
}

void ReflectionGlobalPass::Destroy ( VkDevice device ) noexcept
{
    if ( _descriptorPool != VK_NULL_HANDLE ) [[unlikely]]
    {
        vkDestroyDescriptorPool ( device, _descriptorPool, nullptr );
        _descriptorPool = VK_NULL_HANDLE;
    }

    constexpr auto clean = [] ( auto &vector ) noexcept {
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

    if ( !result ) [[unlikely]]
        return false;

    AV_SET_VULKAN_OBJECT_NAME ( device, _descriptorPool, VK_OBJECT_TYPE_DESCRIPTOR_POOL, "Reflection global" )

    _descriptorSets.resize ( REFLECTIONS );
    VkDescriptorSet* descriptorSets = _descriptorSets.data ();

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
        vkAllocateDescriptorSets ( device, &allocateInfo, descriptorSets ),
        "pbr::ReflectionGlobalPass::AllocateDescriptorSets",
        "Can't allocate descriptor sets"
    );

    if ( !result ) [[unlikely]]
        return false;

#if defined ( AV_ENABLE_VVL ) || defined ( AV_ENABLE_RENDERDOC )

    for ( size_t i = 0U; i < REFLECTIONS; ++i )
    {
        AV_SET_VULKAN_OBJECT_NAME ( device,
            descriptorSets[ i ],
            VK_OBJECT_TYPE_DESCRIPTOR_SET,
            "Reflection global #%zu",
            i
        )
    }

#endif // AV_ENABLE_VVL || AV_ENABLE_RENDERDOC

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
        write.dstSet = descriptorSets[ i ];
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

    if ( more > 0U ) [[unlikely]]
    {
        vkUpdateDescriptorSets ( device, static_cast<uint32_t> ( more ), writeSets, 0U, nullptr );
    }
}

} // namespace pbr
