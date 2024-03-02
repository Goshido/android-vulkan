#include <mandelbrot/mandelbrot_lut_color.hpp>

GX_DISABLE_COMMON_WARNINGS

#include <array>
#include <cmath>
#include <thread>

GX_RESTORE_WARNING_STATE

#include <vulkan_utils.hpp>


namespace mandelbrot {

namespace {

constexpr char const* FRAGMENT_SHADER = "shaders/mandelbrot_lut_color.ps.spv";
constexpr uint32_t LUT_SAMPLE_COUNT = 512U;
constexpr VkDeviceSize LUT_SAMPLE_SIZE = 4U;
constexpr VkDeviceSize LUT_SIZE = LUT_SAMPLE_COUNT * LUT_SAMPLE_SIZE;
constexpr size_t INIT_THREADS = 4U;

} // end of anonymous namespace

//----------------------------------------------------------------------------------------------------------------------

MandelbrotLUTColor::MandelbrotLUTColor () noexcept:
    MandelbrotBase ( FRAGMENT_SHADER )
{
    // NOTHING
}

bool MandelbrotLUTColor::OnInitDevice ( android_vulkan::Renderer &renderer ) noexcept
{
    return MandelbrotBase::OnInitDevice ( renderer ) && CreateLUT ( renderer ) && CreateDescriptorSet ( renderer );
}

void MandelbrotLUTColor::OnDestroyDevice ( android_vulkan::Renderer &renderer ) noexcept
{
    DestroyDescriptorSet ( renderer.GetDevice () );
    DestroyLUT ( renderer );
    MandelbrotBase::OnDestroyDevice ( renderer );
}

bool MandelbrotLUTColor::CreatePipelineLayout ( VkDevice device ) noexcept
{
    constexpr VkDescriptorSetLayoutBinding const binding[]
    {
        {
            .binding = 0U,
            .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
            .descriptorCount = 1U,
            .stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT,
            .pImmutableSamplers = nullptr
        }
    };

    VkDescriptorSetLayoutCreateInfo const descriptorSetInfo
    {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0U,
        .bindingCount = static_cast<uint32_t> ( std::size ( binding ) ),
        .pBindings = binding
    };

    bool result = android_vulkan::Renderer::CheckVkResult (
        vkCreateDescriptorSetLayout ( device, &descriptorSetInfo, nullptr, &_descriptorSetLayout ),
        "MandelbrotLUTColor::CreatePipelineLayout",
        "Can't create descriptor set layout"
    );

    if ( !result )
        return false;

    AV_SET_VULKAN_OBJECT_NAME ( device,
        _descriptorSetLayout,
        VK_OBJECT_TYPE_DESCRIPTOR_SET_LAYOUT,
        "MandelbrotLUTColor::_descriptorSetLayout"
    )

    VkPipelineLayoutCreateInfo const pipelineLayoutInfo
    {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0U,
        .setLayoutCount = 1U,
        .pSetLayouts = &_descriptorSetLayout,
        .pushConstantRangeCount = 0U,
        .pPushConstantRanges = nullptr
    };

    result = android_vulkan::Renderer::CheckVkResult (
        vkCreatePipelineLayout ( device, &pipelineLayoutInfo, nullptr, &_pipelineLayout ),
        "MandelbrotLUTColor::CreatePipelineLayout",
        "Can't create pipeline layout"
    );

    if ( !result )
        return false;

    AV_SET_VULKAN_OBJECT_NAME ( device,
        _pipelineLayout,
        VK_OBJECT_TYPE_PIPELINE_LAYOUT,
        "MandelbrotLUTColor::_pipelineLayout"
    )

    return true;
}

void MandelbrotLUTColor::DestroyPipelineLayout ( VkDevice device ) noexcept
{
    if ( _pipelineLayout != VK_NULL_HANDLE )
    {
        vkDestroyPipelineLayout ( device, _pipelineLayout, nullptr );
        _pipelineLayout = VK_NULL_HANDLE;
    }

    if ( _descriptorSetLayout == VK_NULL_HANDLE )
        return;

    vkDestroyDescriptorSetLayout ( device, _descriptorSetLayout, nullptr );
    _descriptorSetLayout = VK_NULL_HANDLE;
}

bool MandelbrotLUTColor::RecordCommandBuffer ( android_vulkan::Renderer &renderer,
    VkCommandBuffer commandBuffer,
    VkFramebuffer framebuffer
) noexcept
{
    constexpr VkCommandBufferBeginInfo commandBufferBeginInfo
    {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
        .pNext = nullptr,
        .flags = 0U,
        .pInheritanceInfo = nullptr
    };

    constexpr VkClearValue colorClearValue
    {
        .color
        {
            .float32 = { 0.0F, 0.0F, 0.0F, 1.0F }
        }
    };

    VkRenderPassBeginInfo renderPassBeginInfo
    {
        .sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
        .pNext = nullptr,
        .renderPass = _renderPass,
        .framebuffer = VK_NULL_HANDLE,

        .renderArea
        {
            .offset
            {
                .x = 0,
                .y = 0
            },

            .extent = renderer.GetSurfaceSize ()
        },

        .clearValueCount = 1U,
        .pClearValues = &colorClearValue,
    };

    bool result = android_vulkan::Renderer::CheckVkResult (
        vkBeginCommandBuffer ( commandBuffer, &commandBufferBeginInfo ),
        "MandelbrotLUTColor::RecordCommandBuffer",
        "Can't begin command buffer"
    );

    if ( !result )
        return false;

    renderPassBeginInfo.framebuffer = framebuffer;

    vkCmdBeginRenderPass ( commandBuffer, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE );
    vkCmdBindPipeline ( commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, _pipeline );

    vkCmdBindDescriptorSets ( commandBuffer,
        VK_PIPELINE_BIND_POINT_GRAPHICS,
        _pipelineLayout,
        0U,
        1U,
        &_descriptorSet,
        0U,
        nullptr
    );

    vkCmdDraw ( commandBuffer, 4U, 1U, 0U, 0U );
    vkCmdEndRenderPass ( commandBuffer );

    result = android_vulkan::Renderer::CheckVkResult ( vkEndCommandBuffer ( commandBuffer ),
        "MandelbrotLUTColor::CreateCommandBuffer",
        "Can't end command buffer"
    );

    return result;
}

bool MandelbrotLUTColor::CreateDescriptorSet (  android_vulkan::Renderer &renderer ) noexcept
{
    constexpr static VkDescriptorPoolSize const poolSize[]
    {
        {
            .type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
            .descriptorCount = 1U
        }
    };

    constexpr VkDescriptorPoolCreateInfo poolInfo
    {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0U,
        .maxSets = 1U,
        .poolSizeCount = static_cast<uint32_t> ( std::size ( poolSize ) ),
        .pPoolSizes = poolSize
    };

    VkDevice device = renderer.GetDevice ();

    bool result = android_vulkan::Renderer::CheckVkResult (
        vkCreateDescriptorPool ( device, &poolInfo, nullptr, &_descriptorPool ),
        "MandelbrotLUTColor::CreateDescriptorSet",
        "Can't create descriptor pool"
    );

    if ( !result )
        return false;

    AV_REGISTER_DESCRIPTOR_POOL ( "MandelbrotLUTColor::_descriptorPool" )

    VkDescriptorSetAllocateInfo const allocateInfo
    {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
        .pNext = nullptr,
        .descriptorPool = _descriptorPool,
        .descriptorSetCount = 1U,
        .pSetLayouts = &_descriptorSetLayout
    };

    result = android_vulkan::Renderer::CheckVkResult (
        vkAllocateDescriptorSets ( device, &allocateInfo, &_descriptorSet ),
        "MandelbrotLUTColor::CreateDescriptorSet",
        "Can't create descriptor set"
    );

    if ( !result )
        return false;

    VkDescriptorImageInfo const imageInfo
    {
        .sampler = _sampler,
        .imageView = _lutView,
        .imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
    };

    VkWriteDescriptorSet const writeDescriptorSet
    {
        .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
        .pNext = nullptr,
        .dstSet = _descriptorSet,
        .dstBinding = 0U,
        .dstArrayElement = 0U,
        .descriptorCount = 1U,
        .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
        .pImageInfo = &imageInfo,
        .pBufferInfo = nullptr,
        .pTexelBufferView = nullptr
    };

    vkUpdateDescriptorSets ( device, 1U, &writeDescriptorSet, 0U, nullptr );
    return true;
}

void MandelbrotLUTColor::DestroyDescriptorSet ( VkDevice device ) noexcept
{
    if ( _descriptorPool == VK_NULL_HANDLE )
        return;

    vkDestroyDescriptorPool ( device, _descriptorPool, nullptr );

    _descriptorPool = VK_NULL_HANDLE;
    AV_UNREGISTER_DESCRIPTOR_POOL ( "MandelbrotLUTColor::_descriptorPool" )

    _descriptorSet = VK_NULL_HANDLE;
}

bool MandelbrotLUTColor::CreateLUT ( android_vulkan::Renderer &renderer ) noexcept
{
    constexpr VkImageCreateFlags flags = AV_VK_FLAG ( VK_IMAGE_USAGE_TRANSFER_DST_BIT ) |
        AV_VK_FLAG ( VK_IMAGE_USAGE_SAMPLED_BIT );

    constexpr VkImageCreateInfo imageInfo
    {
        .sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0U,
        .imageType = VK_IMAGE_TYPE_1D,
        .format = VK_FORMAT_R8G8B8A8_UNORM,

        .extent
        {
            .width = LUT_SAMPLE_COUNT,
            .height = 1U,
            .depth = 1U,
        },

        .mipLevels = 1U,
        .arrayLayers = 1U,
        .samples = VK_SAMPLE_COUNT_1_BIT,
        .tiling = VK_IMAGE_TILING_OPTIMAL,
        .usage = flags,
        .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
        .queueFamilyIndexCount = 0U,
        .pQueueFamilyIndices = nullptr,
        .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED
    };

    VkDevice device = renderer.GetDevice ();

    bool result = android_vulkan::Renderer::CheckVkResult ( vkCreateImage ( device, &imageInfo, nullptr, &_lut ),
        "MandelbrotLUTColor::CreateLUT",
        "Can't create image"
    );

    if ( !result )
        return false;

    AV_REGISTER_IMAGE ( "MandelbrotLUTColor::_lut" )

    VkMemoryRequirements requirements;
    vkGetImageMemoryRequirements ( device, _lut, &requirements );

    result = renderer.TryAllocateMemory ( _lutMemory,
        _lutOffset,
        requirements,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
        "Can't allocate LUT memory (MandelbrotLUTColor::CreateLUT)"
    );

    if ( !result )
        return false;

    AV_REGISTER_DEVICE_MEMORY ( "MandelbrotLUTColor::_lutMemory" )

    result = android_vulkan::Renderer::CheckVkResult ( vkBindImageMemory ( device, _lut, _lutMemory, _lutOffset ),
        "MandelbrotLUTColor::_lutDeviceMemory",
        "Can't bind LUT memory to the image"
    );

    if ( !result )
        return false;

    VkImageViewCreateInfo const imageViewInfo
    {
        .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0U,
        .image = _lut,
        .viewType = VK_IMAGE_VIEW_TYPE_1D,
        .format = VK_FORMAT_R8G8B8A8_UNORM,

        .components
        {
            .r = VK_COMPONENT_SWIZZLE_IDENTITY,
            .g = VK_COMPONENT_SWIZZLE_IDENTITY,
            .b = VK_COMPONENT_SWIZZLE_IDENTITY,
            .a = VK_COMPONENT_SWIZZLE_IDENTITY
        },

        .subresourceRange
        {
            .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
            .baseMipLevel = 0U,
            .levelCount = 1U,
            .baseArrayLayer = 0U,
            .layerCount = 1U
        }
    };

    result = android_vulkan::Renderer::CheckVkResult ( vkCreateImageView ( device, &imageViewInfo, nullptr, &_lutView ),
        "MandelbrotLUTColor::CreateLUT",
        "Can't create image view"
    );

    if ( !result )
        return false;

    AV_REGISTER_IMAGE_VIEW ( "MandelbrotLUTColor::_lutView" )

    constexpr VkSamplerCreateInfo samplerInfo
    {
        .sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0U,
        .magFilter = VK_FILTER_NEAREST,
        .minFilter = VK_FILTER_NEAREST,
        .mipmapMode = VK_SAMPLER_MIPMAP_MODE_NEAREST,
        .addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
        .addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
        .addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
        .mipLodBias = 0.0F,
        .anisotropyEnable = VK_FALSE,
        .maxAnisotropy = 1.0F,
        .compareEnable = VK_FALSE,
        .compareOp = VK_COMPARE_OP_ALWAYS,
        .minLod = 0.0F,
        .maxLod = 0.0F,
        .borderColor = VK_BORDER_COLOR_FLOAT_TRANSPARENT_BLACK,
        .unnormalizedCoordinates = VK_FALSE
    };

    result = android_vulkan::Renderer::CheckVkResult ( vkCreateSampler ( device, &samplerInfo, nullptr, &_sampler ),
        "MandelbrotLUTColor::CreateLUT",
        "Can't create sampler"
    );

    if ( !result )
        return false;

    AV_SET_VULKAN_OBJECT_NAME ( device, _sampler, VK_OBJECT_TYPE_SAMPLER, "MandelbrotLUTColor::_sampler" )

    if ( UploadLUTSamples ( renderer ) )
        return true;

    DestroyLUT ( renderer );
    return false;
}

void MandelbrotLUTColor::DestroyLUT ( android_vulkan::Renderer &renderer  ) noexcept
{
    VkDevice device = renderer.GetDevice ();

    if ( _sampler != VK_NULL_HANDLE )
    {
        vkDestroySampler ( device, _sampler, nullptr );
        _sampler = VK_NULL_HANDLE;
    }

    if ( _lutView != VK_NULL_HANDLE )
    {
        vkDestroyImageView ( device, _lutView, nullptr );
        _lutView = VK_NULL_HANDLE;
        AV_UNREGISTER_IMAGE_VIEW ( "MandelbrotLUTColor::_lutView" )
    }

    if ( _lut != VK_NULL_HANDLE )
    {
        vkDestroyImage ( device, _lut, nullptr );
        _lut = VK_NULL_HANDLE;
        AV_UNREGISTER_IMAGE ( "MandelbrotLUTColor::_lut" )
    }

    if ( _lutMemory == VK_NULL_HANDLE )
        return;

    renderer.FreeMemory ( _lutMemory, _lutOffset );
    _lutMemory = VK_NULL_HANDLE;
    _lutOffset = std::numeric_limits<VkDeviceSize>::max ();
    AV_UNREGISTER_DEVICE_MEMORY ( "MandelbrotLUTColor::_lutMemory" )
}

bool MandelbrotLUTColor::UploadLUTSamples ( android_vulkan::Renderer &renderer ) noexcept
{
    VkDevice device = renderer.GetDevice ();

    constexpr VkBufferCreateInfo bufferInfo
    {
        .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0U,
        .size = LUT_SIZE,
        .usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
        .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
        .queueFamilyIndexCount = 0U,
        .pQueueFamilyIndices = nullptr
    };

    VkBuffer transfer = VK_NULL_HANDLE;

    bool result = android_vulkan::Renderer::CheckVkResult ( vkCreateBuffer ( device, &bufferInfo, nullptr, &transfer ),
        "MandelbrotLUTColor::CreateLUT",
        "Can't create buffer"
    );

    if ( !result )
        return false;

    AV_SET_VULKAN_OBJECT_NAME ( device,
        transfer,
        VK_OBJECT_TYPE_BUFFER,
        "MandelbrotLUTColor::UploadLUTSamples::transfer"
    )

    VkMemoryRequirements requirements;
    vkGetBufferMemoryRequirements ( device, transfer, &requirements );

    VkDeviceMemory transferMemory = VK_NULL_HANDLE;
    VkDeviceSize transferOffset = std::numeric_limits<VkDeviceSize>::max ();

    result = renderer.TryAllocateMemory ( transferMemory,
        transferOffset,
        requirements,
        AV_VK_FLAG ( VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT ) | AV_VK_FLAG ( VK_MEMORY_PROPERTY_HOST_COHERENT_BIT ),
        "Can't allocate transfer memory (MandelbrotLUTColor::UploadLUTSamples)"
    );


    auto const freeTransferResource = [ & ] () noexcept {
        if ( transfer != VK_NULL_HANDLE )
            vkDestroyBuffer ( device, transfer, nullptr );

        if ( transferMemory != VK_NULL_HANDLE )
        {
            renderer.FreeMemory ( transferMemory, transferOffset );
            AV_UNREGISTER_DEVICE_MEMORY ( "MandelbrotLUTColor::UploadLUTSamples::transferMemory" )
        }
    };

    if ( !result )
    {
        freeTransferResource ();
        return false;
    }

    AV_REGISTER_DEVICE_MEMORY ( "MandelbrotLUTColor::UploadLUTSamples::transferMemory" )

    result = android_vulkan::Renderer::CheckVkResult (
        vkBindBufferMemory ( device, transfer, transferMemory, transferOffset ),
        "MandelbrotLUTColor::CreateLUT",
        "Can't bind memory to the transfer buffer"
    );

    if ( !result )
    {
        freeTransferResource ();
        return false;
    }

    void* data = nullptr;

    result = renderer.MapMemory ( data,
        transferMemory,
        transferOffset,
        "MandelbrotLUTColor::CreateLUT",
        "Can't map transfer memory"
    );

    if ( !result )
    {
        freeTransferResource ();
        return false;
    }

    InitLUTSamples ( static_cast<uint8_t*> ( data ) );
    renderer.UnmapMemory ( transferMemory );

    VkCommandBuffer uploadJob = VK_NULL_HANDLE;

    VkCommandBufferAllocateInfo const allocateInfoInfo
    {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
        .pNext = nullptr,
        .commandPool = _commandInfo[ 0U ]._pool,
        .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
        .commandBufferCount = 1U
    };

    result = android_vulkan::Renderer::CheckVkResult (
        vkAllocateCommandBuffers ( renderer.GetDevice (), &allocateInfoInfo, &uploadJob ),
        "MandelbrotLUTColor::UploadLUTSamples",
        "Can't create common buffer"
    );

    if ( !result )
    {
        freeTransferResource ();
        return false;
    }

    constexpr VkCommandBufferBeginInfo beginInfo
    {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
        .pNext = nullptr,
        .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
        .pInheritanceInfo = nullptr
    };

    result = android_vulkan::Renderer::CheckVkResult ( vkBeginCommandBuffer ( uploadJob, &beginInfo ),
        "MandelbrotLUTColor::UploadLUTSamples",
        "Can't begin command buffer"
    );

    if ( !result )
    {
        freeTransferResource ();
        return false;
    }

    VkImageMemoryBarrier barrier
    {
        .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
        .pNext = nullptr,

        .srcAccessMask = 0U,
        .dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT,
        .oldLayout = VK_IMAGE_LAYOUT_UNDEFINED,
        .newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
        .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
        .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
        .image = _lut,

        .subresourceRange =
        {
            .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
            .baseMipLevel = 0U,
            .levelCount = 1U,
            .baseArrayLayer = 0U,
            .layerCount = 1U
        }
    };

    vkCmdPipelineBarrier ( uploadJob,
        VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
        VK_PIPELINE_STAGE_TRANSFER_BIT,
        0U,
        0U,
        nullptr,
        0U,
        nullptr,
        1U,
        &barrier
    );

    constexpr VkBufferImageCopy copyInfo
    {
        .bufferOffset = 0U,
        .bufferRowLength = LUT_SAMPLE_COUNT,
        .bufferImageHeight = 1U,

        .imageSubresource
        {
            .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
            .mipLevel = 0U,
            .baseArrayLayer = 0U,
            .layerCount = 1U
        },

        .imageOffset
        {
            .x = 0U,
            .y = 0U,
            .z = 0U
        },

        .imageExtent
        {
            .width = LUT_SAMPLE_COUNT,
            .height = 1U,
            .depth = 1U
        }
    };

    vkCmdCopyBufferToImage ( uploadJob, transfer, _lut, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1U, &copyInfo );

    barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrier.pNext = nullptr;
    barrier.image = _lut;
    barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    barrier.subresourceRange.levelCount = barrier.subresourceRange.layerCount = 1U;
    barrier.subresourceRange.baseArrayLayer = barrier.subresourceRange.baseMipLevel = 0U;

    vkCmdPipelineBarrier ( uploadJob,
        VK_PIPELINE_STAGE_TRANSFER_BIT,
        VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
        0U,
        0U,
        nullptr,
        0U,
        nullptr,
        1U,
        &barrier
    );

    result = android_vulkan::Renderer::CheckVkResult ( vkEndCommandBuffer ( uploadJob ),
        "MandelbrotLUTColor::UploadLUTSamples",
        "Can't finish command buffer"
    );

    if ( !result )
    {
        freeTransferResource ();
        return false;
    }

    VkSubmitInfo const submitInfo
    {
        .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
        .pNext = nullptr,
        .waitSemaphoreCount = 0U,
        .pWaitSemaphores = nullptr,
        .pWaitDstStageMask = nullptr,
        .commandBufferCount = 1U,
        .pCommandBuffers = &uploadJob,
        .signalSemaphoreCount = 0U,
        .pSignalSemaphores = nullptr
    };

    VkQueue queue = renderer.GetQueue ();

    result = android_vulkan::Renderer::CheckVkResult (
        vkQueueSubmit ( queue, 1U, &submitInfo, VK_NULL_HANDLE ),
        "MandelbrotLUTColor::UploadLUTSamples",
        "Can't submit command buffer"
    );

    if ( !result )
    {
        freeTransferResource ();
        return false;
    }

    result = android_vulkan::Renderer::CheckVkResult ( vkQueueWaitIdle ( queue ),
        "MandelbrotLUTColor::UploadLUTSamples",
        "Can't commit transfer command"
    );

    freeTransferResource ();
    return result;
}

void MandelbrotLUTColor::InitLUTSamples ( uint8_t* samples ) noexcept
{
    constexpr auto samplePerThread = static_cast<size_t> ( LUT_SAMPLE_COUNT / INIT_THREADS );

    auto job = [ samples ] ( size_t startIndex ) {
        constexpr float twoPi = 6.28318F;
        constexpr float hueOffsetGreen = 2.09439F;
        constexpr float hueOffsetBlue = 4.18879F;
        constexpr float sampleToAngle = twoPi / static_cast<float> ( LUT_SAMPLE_COUNT );

        auto evaluator = [] ( float angle ) -> uint8_t {
            float const n = std::sin ( angle ) * 0.5F + 0.5F;
            return static_cast<uint8_t> ( std::lround ( n * 255.0F ) );
        };

        size_t const limit = startIndex + samplePerThread;
        uint8_t* write = samples + startIndex * LUT_SAMPLE_SIZE;

        for ( size_t i = startIndex; i < limit; ++i )
        {
            float const pivot = static_cast<float> ( i ) * sampleToAngle;

            write[ 0U ] = evaluator ( pivot );
            write[ 1U ] = evaluator ( pivot + hueOffsetGreen );
            write[ 2U ] = evaluator ( pivot + hueOffsetBlue );
            write[ 3U ] = 0xFFU;

            write += LUT_SAMPLE_SIZE;
        }
    };

    std::array<std::thread, INIT_THREADS> threads;

    for ( size_t i = 0U; i < INIT_THREADS; ++i )
        threads[ i ] = std::thread ( job, i * samplePerThread );

    for ( size_t i = 0U; i < INIT_THREADS; ++i )
    {
        threads[ i ].join ();
    }
}

} // namespace mandelbrot
