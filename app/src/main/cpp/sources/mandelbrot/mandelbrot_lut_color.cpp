#include <mandelbrot/mandelbrot_lut_color.h>

AV_DISABLE_COMMON_WARNINGS

#include <array>
#include <cassert>
#include <cmath>
#include <thread>

AV_RESTORE_WARNING_STATE

#include <vulkan_utils.h>


namespace mandelbrot {

constexpr static const char* FRAGMENT_SHADER = "shaders/mandelbrot-lut-color-ps.spv";
constexpr static const uint32_t LUT_SAMPLE_COUNT = 512U;
constexpr static const VkDeviceSize LUT_SAMPLE_SIZE = 4U;
constexpr static const VkDeviceSize LUT_SIZE = LUT_SAMPLE_COUNT * LUT_SAMPLE_SIZE;
constexpr static const size_t INIT_THREADS = 4U;

MandelbrotLUTColor::MandelbrotLUTColor ():
    MandelbrotBase ( FRAGMENT_SHADER ),
    _descriptorPool ( VK_NULL_HANDLE ),
    _descriptorSet ( VK_NULL_HANDLE ),
    _descriptorSetLayout ( VK_NULL_HANDLE ),
    _lut ( VK_NULL_HANDLE ),
    _lutDeviceMemory ( VK_NULL_HANDLE ),
    _lutView ( VK_NULL_HANDLE ),
    _sampler ( VK_NULL_HANDLE )
{
    // NOTHING
}

bool MandelbrotLUTColor::OnInit ( android_vulkan::Renderer &renderer )
{
    if ( !MandelbrotBase::OnInit ( renderer ) )
        return false;

    if ( !CreateLUT ( renderer ) )
    {
        OnDestroy ( renderer );
        return false;
    }

    if ( !CreateDescriptorSet ( renderer ) )
    {
        OnDestroy ( renderer );
        return false;
    }

    if ( CreateCommandBuffer ( renderer ) )
        return true;

    OnDestroy ( renderer );
    return false;
}

bool MandelbrotLUTColor::OnDestroy ( android_vulkan::Renderer &renderer )
{
    DestroyCommandBuffer ( renderer );
    DestroyDescriptorSet ( renderer );
    DestroyLUT ( renderer );
    return MandelbrotBase::OnDestroy ( renderer );
}

bool MandelbrotLUTColor::CreatePipelineLayout ( android_vulkan::Renderer &renderer )
{
    VkDescriptorSetLayoutBinding binding;
    binding.descriptorCount = 1U;
    binding.binding = 0U;
    binding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    binding.pImmutableSamplers = nullptr;
    binding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

    VkDescriptorSetLayoutCreateInfo descriptorSetInfo;
    descriptorSetInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    descriptorSetInfo.pNext = nullptr;
    descriptorSetInfo.flags = 0U;
    descriptorSetInfo.bindingCount = 1U;
    descriptorSetInfo.pBindings = &binding;

    const VkDevice device = renderer.GetDevice ();

    bool result = renderer.CheckVkResult (
        vkCreateDescriptorSetLayout ( renderer.GetDevice (), &descriptorSetInfo, nullptr, &_descriptorSetLayout ),
        "MandelbrotLUTColor::CreatePipelineLayout",
        "Can't create descriptor set layout"
    );

    if ( !result )
        return false;

    AV_REGISTER_DESCRIPTOR_SET_LAYOUT ( "MandelbrotLUTColor::_descriptorSetLayout" )

    VkPipelineLayoutCreateInfo pipelineLayoutInfo;
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.flags = 0U;
    pipelineLayoutInfo.pNext = nullptr;
    pipelineLayoutInfo.pushConstantRangeCount = 0U;
    pipelineLayoutInfo.pPushConstantRanges = nullptr;
    pipelineLayoutInfo.setLayoutCount = 1U;
    pipelineLayoutInfo.pSetLayouts = &_descriptorSetLayout;

    result = renderer.CheckVkResult (
        vkCreatePipelineLayout ( device, &pipelineLayoutInfo, nullptr, &_pipelineLayout ),
        "MandelbrotLUTColor::CreatePipelineLayout",
        "Can't create pipeline layout"
    );

    if ( !result )
        return false;

    AV_REGISTER_PIPELINE_LAYOUT ( "MandelbrotLUTColor::_pipelineLayout" )
    return true;
}

void MandelbrotLUTColor::DestroyPipelineLayout ( android_vulkan::Renderer &renderer )
{
    const VkDevice device = renderer.GetDevice ();

    if ( _pipelineLayout != VK_NULL_HANDLE )
    {
        vkDestroyPipelineLayout ( device, _pipelineLayout, nullptr );
        _pipelineLayout = VK_NULL_HANDLE;
        AV_UNREGISTER_PIPELINE_LAYOUT ( "MandelbrotLUTColor::_pipelineLayout" )
    }

    if ( _descriptorSetLayout == VK_NULL_HANDLE )
        return;

    vkDestroyDescriptorSetLayout ( device, _descriptorSetLayout, nullptr );
    _descriptorSetLayout = VK_NULL_HANDLE;
    AV_UNREGISTER_DESCRIPTOR_SET_LAYOUT ( "MandelbrotLUTColor::_descriptorSetLayout" )
}

bool MandelbrotLUTColor::CreateCommandBuffer ( android_vulkan::Renderer &renderer )
{
    const size_t framebufferCount = renderer.GetPresentFramebufferCount ();
    _commandBuffer.resize ( framebufferCount );

    VkCommandBufferAllocateInfo commandBufferInfo;
    commandBufferInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    commandBufferInfo.pNext = nullptr;
    commandBufferInfo.commandBufferCount = static_cast<uint32_t> ( framebufferCount );
    commandBufferInfo.commandPool = _commandPool;
    commandBufferInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;

    bool result = renderer.CheckVkResult (
        vkAllocateCommandBuffers ( renderer.GetDevice (), &commandBufferInfo, _commandBuffer.data () ),
        "MandelbrotLUTColor::CreateCommandBuffer",
        "Can't allocate command buffer"
    );

    if ( !result )
    {
        DestroyCommandBuffer ( renderer );
        return false;
    }

    VkCommandBufferBeginInfo commandBufferBeginInfo;
    commandBufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    commandBufferBeginInfo.pNext = nullptr;
    commandBufferBeginInfo.flags = 0U;
    commandBufferBeginInfo.pInheritanceInfo = nullptr;

    VkClearValue clearValues[ 2U ];
    VkClearValue& colorClearValue = clearValues[ 0U ];
    colorClearValue.color.float32[ 0U ] = 0.0F;
    colorClearValue.color.float32[ 1U ] = 0.0F;
    colorClearValue.color.float32[ 2U ] = 0.0F;
    colorClearValue.color.float32[ 3U ] = 1.0F;

    VkClearValue& depthStencilClearValue = clearValues[ 1U ];
    depthStencilClearValue.depthStencil.depth = 1.0F;
    depthStencilClearValue.depthStencil.stencil = 0U;

    VkRenderPassBeginInfo renderPassBeginInfo;
    renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassBeginInfo.pNext = nullptr;
    renderPassBeginInfo.renderPass = _renderPass;

    renderPassBeginInfo.renderArea.offset.x = 0;
    renderPassBeginInfo.renderArea.offset.y = 0;
    renderPassBeginInfo.renderArea.extent = renderer.GetSurfaceSize ();
    renderPassBeginInfo.clearValueCount = 2U;
    renderPassBeginInfo.pClearValues = clearValues;

    for ( size_t i = 0U; i < framebufferCount; ++i )
    {
        const VkCommandBuffer commandBuffer = _commandBuffer[ i ];

        result = renderer.CheckVkResult ( vkBeginCommandBuffer ( commandBuffer, &commandBufferBeginInfo ),
            "MandelbrotLUTColor::CreateCommandBuffer",
            "Can't begin command buffer"
        );

        if ( !result )
        {
            DestroyCommandBuffer ( renderer );
            return false;
        }

        renderPassBeginInfo.framebuffer = renderer.GetPresentFramebuffer ( static_cast<uint32_t> ( i ) );

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

        result = renderer.CheckVkResult ( vkEndCommandBuffer ( commandBuffer ),
            "MandelbrotLUTColor::CreateCommandBuffer",
            "Can't end command buffer"
        );
    }

    if ( result )
        return true;

    DestroyCommandBuffer ( renderer );
    return false;
}

void MandelbrotLUTColor::DestroyCommandBuffer ( android_vulkan::Renderer& /*renderer*/ )
{
    _commandBuffer.clear ();
}

bool MandelbrotLUTColor::CreateDescriptorSet (  android_vulkan::Renderer &renderer )
{
    VkDescriptorPoolSize poolSize;
    poolSize.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    poolSize.descriptorCount = 1U;

    VkDescriptorPoolCreateInfo poolInfo;
    poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.pNext = nullptr;
    poolInfo.flags = 0U;
    poolInfo.maxSets = 1U;
    poolInfo.poolSizeCount = 1U;
    poolInfo.pPoolSizes = &poolSize;

    const VkDevice device = renderer.GetDevice ();

    bool result = renderer.CheckVkResult ( vkCreateDescriptorPool ( device, &poolInfo, nullptr, &_descriptorPool ),
        "MandelbrotLUTColor::CreateDescriptorSet",
        "Can't create descriptor pool"
    );

    if ( !result )
        return false;

    AV_REGISTER_DESCRIPTOR_POOL ( "MandelbrotLUTColor::_descriptorPool" )

    VkDescriptorSetAllocateInfo allocateInfo;
    allocateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocateInfo.pNext = nullptr;
    allocateInfo.descriptorSetCount = 1U;
    allocateInfo.descriptorPool = _descriptorPool;
    allocateInfo.pSetLayouts = &_descriptorSetLayout;

    result = renderer.CheckVkResult ( vkAllocateDescriptorSets ( device, &allocateInfo, &_descriptorSet ),
        "MandelbrotLUTColor::CreateDescriptorSet",
        "Can't create descriptor set"
    );

    if ( !result )
        return false;

    VkDescriptorImageInfo imageInfo;
    imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    imageInfo.imageView = _lutView;
    imageInfo.sampler = _sampler;

    VkWriteDescriptorSet writeDescriptorSet;
    writeDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    writeDescriptorSet.pNext = nullptr;
    writeDescriptorSet.descriptorCount = 1U;
    writeDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    writeDescriptorSet.dstSet = _descriptorSet;
    writeDescriptorSet.dstBinding = 0U;
    writeDescriptorSet.pImageInfo = &imageInfo;
    writeDescriptorSet.pBufferInfo = nullptr;
    writeDescriptorSet.pTexelBufferView = nullptr;
    writeDescriptorSet.dstArrayElement = 0U;

    vkUpdateDescriptorSets ( device, 1U, &writeDescriptorSet, 0U, nullptr );
    return true;
}

void MandelbrotLUTColor::DestroyDescriptorSet (  android_vulkan::Renderer &renderer )
{
    if ( _descriptorPool == VK_NULL_HANDLE )
        return;

    vkDestroyDescriptorPool ( renderer.GetDevice (), _descriptorPool, nullptr );

    _descriptorPool = VK_NULL_HANDLE;
    AV_UNREGISTER_DESCRIPTOR_POOL ( "MandelbrotLUTColor::_descriptorPool" )

    _descriptorSet = VK_NULL_HANDLE;
}

bool MandelbrotLUTColor::CreateLUT ( android_vulkan::Renderer &renderer )
{
    VkImageCreateInfo imageInfo;
    imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    imageInfo.pNext = nullptr;
    imageInfo.flags = 0U;
    imageInfo.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
    imageInfo.format = VK_FORMAT_R8G8B8A8_UNORM;
    imageInfo.imageType = VK_IMAGE_TYPE_1D;
    imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
    imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
    imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    imageInfo.arrayLayers = 1U;
    imageInfo.mipLevels = 1U;
    imageInfo.extent.width = LUT_SAMPLE_COUNT;
    imageInfo.extent.height = 1U;
    imageInfo.extent.depth = 1U;
    imageInfo.queueFamilyIndexCount = 0U;
    imageInfo.pQueueFamilyIndices = nullptr;

    const VkDevice device = renderer.GetDevice ();

    bool result = renderer.CheckVkResult ( vkCreateImage ( device, &imageInfo, nullptr, &_lut ),
        "MandelbrotLUTColor::CreateLUT",
        "Can't create image"
    );

    if ( !result )
        return false;

    AV_REGISTER_IMAGE ( "MandelbrotLUTColor::_lut" )

    VkMemoryRequirements requirements;
    vkGetImageMemoryRequirements ( device, _lut, &requirements );

    result = renderer.TryAllocateMemory ( _lutDeviceMemory,
        requirements,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
        "Can't allocate LUT memory (MandelbrotLUTColor::CreateLUT)"
    );

    if ( !result )
    {
        DestroyLUT ( renderer );
        return false;
    }

    AV_REGISTER_DEVICE_MEMORY ( "MandelbrotLUTColor::_lutDeviceMemory" )

    result = renderer.CheckVkResult ( vkBindImageMemory ( device, _lut, _lutDeviceMemory, 0U ),
        "MandelbrotLUTColor::_lutDeviceMemory",
        "Can't bind LUT memory to the image"
    );

    if ( !result )
    {
        DestroyLUT ( renderer );
        return false;
    }

    VkImageViewCreateInfo imageViewInfo;
    imageViewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    imageViewInfo.pNext = nullptr;
    imageViewInfo.flags = 0U;
    imageViewInfo.image = _lut;
    imageViewInfo.format = VK_FORMAT_R8G8B8A8_UNORM;
    imageViewInfo.viewType = VK_IMAGE_VIEW_TYPE_1D;
    imageViewInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
    imageViewInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
    imageViewInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
    imageViewInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
    imageViewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    imageViewInfo.subresourceRange.baseArrayLayer = imageViewInfo.subresourceRange.baseMipLevel = 0U;
    imageViewInfo.subresourceRange.layerCount = imageViewInfo.subresourceRange.levelCount = 1U;

    result = renderer.CheckVkResult ( vkCreateImageView ( device, &imageViewInfo, nullptr, &_lutView ),
        "MandelbrotLUTColor::CreateLUT",
        "Can't create image view"
    );

    if ( !result )
    {
        DestroyLUT ( renderer );
        return false;
    }

    AV_REGISTER_IMAGE_VIEW ( "MandelbrotLUTColor::_lutView" )

    VkSamplerCreateInfo samplerInfo;
    samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    samplerInfo.pNext = nullptr;
    samplerInfo.flags = 0U;
    samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    samplerInfo.anisotropyEnable = VK_FALSE;
    samplerInfo.maxAnisotropy = 1.0F;
    samplerInfo.borderColor = VK_BORDER_COLOR_FLOAT_TRANSPARENT_BLACK;
    samplerInfo.compareEnable = VK_FALSE;
    samplerInfo.magFilter = VK_FILTER_NEAREST;
    samplerInfo.minFilter = VK_FILTER_NEAREST;
    samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_NEAREST;
    samplerInfo.minLod = samplerInfo.maxLod = 0.0F;
    samplerInfo.mipLodBias = 0.0F;
    samplerInfo.unnormalizedCoordinates = VK_FALSE;

    result = renderer.CheckVkResult ( vkCreateSampler ( device, &samplerInfo, nullptr, &_sampler ),
        "MandelbrotLUTColor::CreateLUT",
        "Can't create sampler"
    );

    if ( !result )
    {
        DestroyLUT ( renderer );
        return false;
    }

    AV_REGISTER_SAMPLER ( "MandelbrotLUTColor::_sampler" )

    if ( UploadLUTSamples ( renderer ) )
        return true;

    DestroyLUT ( renderer );
    return false;
}

void MandelbrotLUTColor::DestroyLUT ( android_vulkan::Renderer &renderer )
{
    const VkDevice device = renderer.GetDevice ();

    if ( _sampler != VK_NULL_HANDLE )
    {
        vkDestroySampler ( device, _sampler, nullptr );
        _sampler = VK_NULL_HANDLE;
        AV_UNREGISTER_SAMPLER ( "MandelbrotLUTColor::_sampler" )
    }

    if ( _lutView != VK_NULL_HANDLE )
    {
        vkDestroyImageView ( device, _lutView, nullptr );
        _lutView = VK_NULL_HANDLE;
        AV_UNREGISTER_IMAGE_VIEW ( "MandelbrotLUTColor::_lutView" )
    }

    if ( _lutDeviceMemory != VK_NULL_HANDLE )
    {
        vkFreeMemory ( device, _lutDeviceMemory, nullptr );
        _lutDeviceMemory = VK_NULL_HANDLE;
        AV_UNREGISTER_DEVICE_MEMORY ( "MandelbrotLUTColor::_lutDeviceMemory" )
    }

    if ( _lut == VK_NULL_HANDLE )
        return;

    vkDestroyImage ( renderer.GetDevice (), _lut, nullptr );
    _lut = VK_NULL_HANDLE;
    AV_UNREGISTER_IMAGE ( "MandelbrotLUTColor::_lut" )
}

void MandelbrotLUTColor::InitLUTSamples ( uint8_t* samples ) const
{
    constexpr const auto samplePerThread = static_cast<const size_t> ( LUT_SAMPLE_COUNT / INIT_THREADS );

    auto job = [ samples ] ( size_t startIndex ) {
        constexpr const float twoPi = 6.28318F;
        constexpr const float hueOffsetGreen = 2.09439F;
        constexpr const float hueOffsetBlue = 4.18879F;
        constexpr const float sampleToAngle = twoPi / static_cast<float> ( LUT_SAMPLE_COUNT );

        auto evaluator = [] ( float angle ) -> uint8_t {
            const float n = std::sinf ( angle ) * 0.5F + 0.5F;
            return static_cast<uint8_t> ( n * 255.0F + 0.5F );
        };

        const size_t limit = startIndex + samplePerThread;
        uint8_t* write = samples + startIndex * LUT_SAMPLE_SIZE;

        for ( size_t i = startIndex; i < limit; ++i )
        {
            const float pivot = i * sampleToAngle;

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

bool MandelbrotLUTColor::UploadLUTSamples ( android_vulkan::Renderer &renderer )
{
    const VkDevice device = renderer.GetDevice ();

    VkBufferCreateInfo bufferInfo;
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.pNext = nullptr;
    bufferInfo.flags = 0U;
    bufferInfo.size = LUT_SIZE;
    bufferInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    bufferInfo.queueFamilyIndexCount = 0U;
    bufferInfo.pQueueFamilyIndices = nullptr;

    VkBuffer transfer = VK_NULL_HANDLE;

    bool result = renderer.CheckVkResult ( vkCreateBuffer ( device, &bufferInfo, nullptr, &transfer ),
        "MandelbrotLUTColor::CreateLUT",
        "Can't create buffer"
    );

    if ( !result )
        return false;

    AV_REGISTER_BUFFER ( "MandelbrotLUTColor::UploadLUTSamples::transfer" )

    VkMemoryRequirements requirements;
    vkGetBufferMemoryRequirements ( device, transfer, &requirements );

    VkDeviceMemory transferDeviceMemory = VK_NULL_HANDLE;

    result = renderer.TryAllocateMemory ( transferDeviceMemory,
        requirements,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
        "Can't allocate transfer memory (MandelbrotLUTColor::UploadLUTSamples)"
    );

    auto freeTransferResource = [ & ] () {
        if ( transferDeviceMemory != VK_NULL_HANDLE )
        {
            vkFreeMemory ( device, transferDeviceMemory, nullptr );
            AV_UNREGISTER_DEVICE_MEMORY ( "MandelbrotLUTColor::UploadLUTSamples::transferDeviceMemory" )
        }

        vkDestroyBuffer ( device, transfer, nullptr );
        AV_UNREGISTER_BUFFER ( "MandelbrotLUTColor::UploadLUTSamples::transfer" )
    };

    if ( !result )
    {
        freeTransferResource ();
        return false;
    }

    AV_REGISTER_DEVICE_MEMORY ( "MandelbrotLUTColor::UploadLUTSamples::transferDeviceMemory" )

    result = renderer.CheckVkResult ( vkBindBufferMemory ( device, transfer, transferDeviceMemory, 0U ),
        "MandelbrotLUTColor::CreateLUT",
        "Can't bind memory to the transfer buffer"
    );

    if ( !result )
    {
        freeTransferResource ();
        return false;
    }

    void* data = nullptr;

    result = renderer.CheckVkResult ( vkMapMemory ( device, transferDeviceMemory, 0U, LUT_SIZE, 0U, &data ),
        "MandelbrotLUTColor::CreateLUT",
        "Can't map transfer memory"
    );

    if ( !result )
    {
        freeTransferResource ();
        return false;
    }

    InitLUTSamples ( static_cast<uint8_t*> ( data ) );
    vkUnmapMemory ( device, transferDeviceMemory );

    VkCommandBuffer uploadJob = VK_NULL_HANDLE;

    VkCommandBufferAllocateInfo allocateInfoInfo;
    allocateInfoInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocateInfoInfo.pNext = nullptr;
    allocateInfoInfo.commandBufferCount = 1U;
    allocateInfoInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocateInfoInfo.commandPool = _commandPool;

    result = renderer.CheckVkResult (
        vkAllocateCommandBuffers ( renderer.GetDevice (), &allocateInfoInfo, &uploadJob ),
        "MandelbrotLUTColor::UploadLUTSamples",
        "Can't create common buffer"
    );

    if ( !result )
    {
        freeTransferResource ();
        return false;
    }

    VkCommandBufferBeginInfo beginInfo;
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.pNext = nullptr;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    beginInfo.pInheritanceInfo = nullptr;

    result = renderer.CheckVkResult ( vkBeginCommandBuffer ( uploadJob, &beginInfo ),
        "MandelbrotLUTColor::UploadLUTSamples",
        "Can't begin command buffer"
    );

    if ( !result )
    {
        freeTransferResource ();
        return false;
    }

    VkImageMemoryBarrier barrier;
    barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrier.pNext = nullptr;
    barrier.image = _lut;
    barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    barrier.srcAccessMask = 0U;
    barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    barrier.subresourceRange.levelCount = barrier.subresourceRange.layerCount = 1U;
    barrier.subresourceRange.baseArrayLayer = barrier.subresourceRange.baseMipLevel = 0U;

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

    VkBufferImageCopy copyInfo;
    copyInfo.bufferOffset = 0U;
    copyInfo.bufferRowLength = LUT_SAMPLE_COUNT;
    copyInfo.bufferImageHeight = 1U;
    copyInfo.imageSubresource.baseArrayLayer = 0U;
    copyInfo.imageSubresource.layerCount = 1U;
    copyInfo.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    copyInfo.imageSubresource.mipLevel = 0U;
    copyInfo.imageOffset.x = copyInfo.imageOffset.y = copyInfo.imageOffset.z = 0U;
    copyInfo.imageExtent.width = LUT_SAMPLE_COUNT;
    copyInfo.imageExtent.height = 1U;
    copyInfo.imageExtent.depth = 1U;

    vkCmdCopyBufferToImage ( uploadJob, transfer, _lut, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1U, &copyInfo );

    barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrier.pNext = nullptr;
    barrier.image = _lut;
    barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
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

    result = renderer.CheckVkResult ( vkEndCommandBuffer ( uploadJob ),
        "MandelbrotLUTColor::UploadLUTSamples",
        "Can't finish command buffer"
    );

    if ( !result )
    {
        freeTransferResource ();
        return false;
    }

    VkSubmitInfo submitInfo;
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.pNext = nullptr;
    submitInfo.waitSemaphoreCount = 0U;
    submitInfo.pWaitSemaphores = nullptr;
    submitInfo.pWaitDstStageMask = nullptr;
    submitInfo.commandBufferCount = 1U;
    submitInfo.pCommandBuffers = &uploadJob;
    submitInfo.signalSemaphoreCount = 0U;
    submitInfo.pSignalSemaphores = nullptr;

    const VkQueue queue = renderer.GetQueue ();

    result = renderer.CheckVkResult (
        vkQueueSubmit ( queue, 1U, &submitInfo, VK_NULL_HANDLE ),
        "MandelbrotLUTColor::UploadLUTSamples",
        "Can't submit command buffer"
    );

    if ( !result )
    {
        freeTransferResource ();
        return false;
    }

    result = renderer.CheckVkResult ( vkQueueWaitIdle ( queue ),
        "MandelbrotLUTColor::UploadLUTSamples",
        "Can't commit transfer command"
    );

    freeTransferResource ();
    return result;
}

} // namespace mandelbrot
