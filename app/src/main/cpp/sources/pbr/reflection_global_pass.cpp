#include <pbr/reflection_global_pass.h>

GX_DISABLE_COMMON_WARNINGS

#include <cassert>

GX_RESTORE_WARNING_STATE


namespace pbr {

// 256 128 64 32 16 8 4 2 1
// counting from 0.0F
constexpr static float const MAX_PREFILTER_LOD = 8.0F;

ReflectionGlobalPass::ReflectionGlobalPass () noexcept:
    _commandPool ( VK_NULL_HANDLE ),
    _descriptorPool ( VK_NULL_HANDLE ),
    _descriptorSets {},
    _imageInfo {},
    _prefilters {},
    _program {},
    _sampler {},
    _submitInfoTransfer {},
    _transferCommandBuffer ( VK_NULL_HANDLE ),
    _uniformInfo {},
    _uniformPool ( eUniformPoolSize::Tiny_4M ),
    _writeSets {}
{
    // NOTHING
}

void ReflectionGlobalPass::Append ( TextureCubeRef &prefilter )
{
    _prefilters.push_back ( prefilter );
}

[[maybe_unused]] bool ReflectionGlobalPass::Execute ( android_vulkan::Renderer &renderer,
    bool &isCommonSetBind,
    LightupCommonDescriptorSet &lightupCommonDescriptorSet,
    VkCommandBuffer commandBuffer,
    GXMat4 const &viewToWorld
)
{
    size_t const count = _prefilters.size ();

    if ( !count )
        return true;

    if ( !UpdateGPUData(renderer, count, viewToWorld ) )
        return false;

    if ( !isCommonSetBind )
    {
        _program.SetCommonDescriptorSet ( commandBuffer, lightupCommonDescriptorSet.GetSet () );
        isCommonSetBind = true;
    }

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
    VkCommandPoolCreateInfo const poolInfo
    {
        .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
        .pNext = nullptr,
        .flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
        .queueFamilyIndex = renderer.GetQueueFamilyIndex ()
    };

    VkDevice device = renderer.GetDevice ();

    bool result = android_vulkan::Renderer::CheckVkResult (
        vkCreateCommandPool ( device, &poolInfo, nullptr, &_commandPool ),
        "ReflectionGlobalPass::Init",
        "Can't create command pool"
    );

    if ( !result )
        return false;

    AV_REGISTER_COMMAND_POOL ( "ReflectionGlobalPass::_commandPool" )

    VkCommandBufferAllocateInfo const allocateInfo
    {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
        .pNext = nullptr,
        .commandPool = _commandPool,
        .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
        .commandBufferCount = 1U
    };

    result = android_vulkan::Renderer::CheckVkResult (
        vkAllocateCommandBuffers ( device, &allocateInfo, &_transferCommandBuffer ),
        "ReflectionGlobalPass::Init",
        "Can't allocate command buffer"
    );

    if ( !result )
    {
        Destroy ( device );
        return false;
    }

    _submitInfoTransfer.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    _submitInfoTransfer.pNext = nullptr;
    _submitInfoTransfer.waitSemaphoreCount = 0U;
    _submitInfoTransfer.pWaitSemaphores = nullptr;
    _submitInfoTransfer.pWaitDstStageMask = nullptr;
    _submitInfoTransfer.commandBufferCount = 1U;
    _submitInfoTransfer.pCommandBuffers = &_transferCommandBuffer;
    _submitInfoTransfer.signalSemaphoreCount = 0U;
    _submitInfoTransfer.pSignalSemaphores = nullptr;

    if ( !_program.Init ( renderer, renderPass, subpass, viewport ) )
    {
        Destroy ( device );
        return false;
    }

    constexpr VkSamplerCreateInfo const samplerInfo
    {
        .sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0U,
        .magFilter = VK_FILTER_LINEAR,
        .minFilter = VK_FILTER_LINEAR,
        .mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR,
        .addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
        .addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
        .addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
        .mipLodBias = 0.0F,
        .anisotropyEnable = VK_FALSE,
        .maxAnisotropy = 1.0F,
        .compareEnable = VK_FALSE,
        .compareOp = VK_COMPARE_OP_ALWAYS,
        .minLod = 0.0F,
        .maxLod = MAX_PREFILTER_LOD,
        .borderColor = VK_BORDER_COLOR_FLOAT_TRANSPARENT_BLACK,
        .unnormalizedCoordinates = VK_FALSE
    };

    if ( !_sampler.Init ( renderer, samplerInfo ) )
    {
        Destroy ( device );
        return false;
    }

    if ( !_uniformPool.Init ( renderer, sizeof ( ReflectionGlobalProgram::Transform ) ) )
    {
        Destroy ( device );
        return false;
    }

    return true;
}

void ReflectionGlobalPass::Destroy ( VkDevice device )
{
    DestroyDescriptorPool ( device );

    _descriptorSets.clear ();
    _imageInfo.clear ();
    _uniformInfo.clear ();
    _writeSets.clear ();

    _uniformPool.Destroy ( device );
    _sampler.Destroy ( device );
    _program.Destroy ( device );

    if ( _commandPool == VK_NULL_HANDLE )
        return;

    vkDestroyCommandPool ( device, _commandPool, nullptr );
    _commandPool = VK_NULL_HANDLE;
    _transferCommandBuffer = VK_NULL_HANDLE;
    AV_UNREGISTER_COMMAND_POOL ( "ReflectionGlobalPass::_commandPool" )
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
        },
        {
            .type = VK_DESCRIPTOR_TYPE_SAMPLER,
            .descriptorCount = size
        },
        {
            .type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
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
        "ReflectionGlobalPass::AllocateDescriptorSets",
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
        "ReflectionGlobalPass::AllocateDescriptorSets",
        "Can't allocate descriptor sets"
    );

    if ( !result )
        return false;

    // Initialize all immutable constant fields.

    VkDescriptorImageInfo const image
    {
        .sampler = _sampler.GetSampler (),
        .imageView = VK_NULL_HANDLE,
        .imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
    };

    _imageInfo.resize ( neededSets, image );

    constexpr VkDescriptorBufferInfo const uniform
    {
        .buffer = VK_NULL_HANDLE,
        .offset = 0U,
        .range = static_cast<VkDeviceSize> ( sizeof ( ReflectionGlobalProgram::Transform ) )
    };

    _uniformInfo.resize ( neededSets, uniform );

    constexpr VkWriteDescriptorSet const writeSet
    {
        .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
        .pNext = nullptr,
        .dstSet = VK_NULL_HANDLE,
        .dstBinding = UINT32_MAX,
        .dstArrayElement = 0U,
        .descriptorCount = 1U,
        .descriptorType = VK_DESCRIPTOR_TYPE_MAX_ENUM,
        .pImageInfo = nullptr,
        .pBufferInfo = nullptr,
        .pTexelBufferView = nullptr
    };

    _writeSets.resize ( static_cast<size_t> ( poolInfo.poolSizeCount ), writeSet );

    for ( size_t i = 0U; i < neededSets; ++i )
    {
        size_t const base = i * static_cast<size_t> ( poolInfo.poolSizeCount );
        VkWriteDescriptorSet& imageWriteSet = _writeSets[ base ];
        VkWriteDescriptorSet& samplerWriteSet = _writeSets[ base + 1U ];
        VkWriteDescriptorSet& uniformWriteSet = _writeSets[ base + 2U ];

        VkDescriptorSet descriptorSet = _descriptorSets[ i ];
        imageWriteSet.dstSet = samplerWriteSet.dstSet = uniformWriteSet.dstSet = descriptorSet;

        imageWriteSet.dstBinding = 0U;
        samplerWriteSet.dstBinding = 1U;
        uniformWriteSet.dstBinding = 2U;

        imageWriteSet.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
        samplerWriteSet.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLER;
        uniformWriteSet.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;

        imageWriteSet.pImageInfo = samplerWriteSet.pImageInfo = &_imageInfo[ i ];
        uniformWriteSet.pBufferInfo = &_uniformInfo[ i ];
    }

    // Now all what is needed to do is to init "_imageInfo::imageView" data and "_uniformInfo::buffer".
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

bool ReflectionGlobalPass::UpdateGPUData ( android_vulkan::Renderer &renderer, size_t count, GXMat4 const &viewToWorld )
{
    _uniformPool.Reset();

    if ( !AllocateDescriptorSets ( renderer, count ) )
        return false;

    constexpr VkCommandBufferBeginInfo const beginInfo
    {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
        .pNext = nullptr,
        .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
        .pInheritanceInfo = nullptr
    };

    bool result = android_vulkan::Renderer::CheckVkResult ( vkBeginCommandBuffer ( _transferCommandBuffer, &beginInfo ),
        "ReflectionGlobalPass::UpdateGPUData",
        "Can't begin command buffer"
    );

    if ( !result )
        return false;

    ReflectionGlobalProgram::Transform const transform
    {
        ._viewToWorld = viewToWorld
    };

    for ( size_t i = 0U; i < count; ++i )
    {
        VkDescriptorImageInfo& info = _imageInfo[ i ];
        info.imageView = _prefilters[ i ]->GetImageView ();

        VkDescriptorBufferInfo& buffer = _uniformInfo[ i ];

        buffer.buffer = _uniformPool.Acquire ( renderer,
            _transferCommandBuffer,
            &transform,
            VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT
        );
    }

    result = android_vulkan::Renderer::CheckVkResult ( vkEndCommandBuffer ( _transferCommandBuffer ),
        "ReflectionGlobalPass::UpdateGPUData",
        "Can't end command buffer"
    );

    if ( !result )
        return false;

    result = android_vulkan::Renderer::CheckVkResult (
        vkQueueSubmit ( renderer.GetQueue (), 1U, &_submitInfoTransfer, VK_NULL_HANDLE ),
        "ReflectionGlobalPass::UpdateGPUData",
        "Can't submit command buffer"
    );

    if ( !result )
        return false;

    vkUpdateDescriptorSets ( renderer.GetDevice (),
        static_cast<uint32_t> ( _writeSets.size() ),
        _writeSets.data (),
        0U,
        nullptr
    );

    return true;
}

} // namespace pbr
