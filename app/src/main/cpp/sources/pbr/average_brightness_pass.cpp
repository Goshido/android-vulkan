#include <pbr/average_brightness_pass.hpp>
#include <pbr/spd.inc>
#include <vulkan_utils.hpp>


namespace pbr {

void AverageBrightnessPass::FreeTransferResources ( android_vulkan::Renderer &renderer,
    VkCommandPool commandPool
) noexcept
{
    VkDevice device = renderer.GetDevice ();
    vkFreeCommandBuffers ( device, commandPool, 1U, &_commandBuffer );
    _commandBuffer = VK_NULL_HANDLE;
    FreeTransferBuffer ( renderer, device );
}

bool AverageBrightnessPass::Init ( android_vulkan::Renderer &renderer, VkCommandPool commandPool ) noexcept
{
    VkDevice device = renderer.GetDevice ();

    return CreateGlobalCounter ( renderer, device, commandPool ) &&
        _layout.Init ( device ) &&
        CreateDescriptorSet ( device );
}

void AverageBrightnessPass::Destroy ( android_vulkan::Renderer &renderer ) noexcept
{
    VkDevice device = renderer.GetDevice ();

    if ( _globalCounter != VK_NULL_HANDLE )
    {
        vkDestroyBuffer ( device, _globalCounter, nullptr );
        _globalCounter = VK_NULL_HANDLE;
        AV_UNREGISTER_BUFFER ( "pbr::AverageBrightnessPass::_globalCounter" )
    }

    if ( _globalCounterMemory._memory != VK_NULL_HANDLE )
    {
        renderer.FreeMemory ( _globalCounterMemory._memory, _globalCounterMemory._offset );
        _globalCounterMemory._memory = VK_NULL_HANDLE;
        _globalCounterMemory._offset = std::numeric_limits<VkDeviceSize>::max ();
        AV_UNREGISTER_DEVICE_MEMORY ( "pbr::AverageBrightnessPass::_globalCounterMemory" )
    }

    if ( _descriptorPool != VK_NULL_HANDLE )
    {
        vkDestroyDescriptorPool ( device, _descriptorPool, nullptr );
        _descriptorPool = VK_NULL_HANDLE;
        AV_UNREGISTER_DESCRIPTOR_POOL ( "pbr::AverageBrightnessPass::_descriptorPool" )
    }

    _layout.Destroy ( device );
    FreeTransferBuffer ( renderer, device );
}

bool AverageBrightnessPass::SetTarget ( android_vulkan::Renderer &renderer,
    android_vulkan::Texture2D const &hdrImage
) noexcept
{
    // TODO do nothing if mip chain is the same.

    VkExtent2D resolution = hdrImage.GetResolution ();
    constexpr uint32_t multipleOf64 = 6U;

    _dispatch =
    {
        .width = resolution.width >> multipleOf64,
        .height = resolution.height >> multipleOf64,
        .depth = 1U
    };

    VkDevice device = renderer.GetDevice ();

    if ( !CreateMips ( renderer, device, resolution ) )
        return false;

    BindTargetToDescriptorSet ( hdrImage );
    return true;
}

VkExtent2D AverageBrightnessPass::AdjustResolution ( VkExtent2D const &desiredResolution ) noexcept
{
    auto const adjust = [] ( uint32_t size ) constexpr -> uint32_t {
        constexpr uint32_t blockSizeShift = 6U;
        constexpr uint32_t roundUP = ( 1U << blockSizeShift ) - 1U;

        return std::min ( 2048U, std::max ( 1U, ( size + roundUP ) >> blockSizeShift ) << blockSizeShift );
    };

    return VkExtent2D
    {
        .width = adjust ( desiredResolution.width ),
        .height = adjust ( desiredResolution.height )
    };
}

bool AverageBrightnessPass::CreateDescriptorSet ( VkDevice device ) noexcept
{
    constexpr static VkDescriptorPoolSize const poolSizes[] =
    {
        {
            .type = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,
            .descriptorCount = 1U
        },
        {
            .type = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,
            .descriptorCount = 2U
        },
        {
            .type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
            .descriptorCount = 1U
        }
    };

    constexpr VkDescriptorPoolCreateInfo poolInfo
    {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0U,
        .maxSets = 1U,
        .poolSizeCount = static_cast<uint32_t> ( std::size ( poolSizes ) ),
        .pPoolSizes = poolSizes
    };

    bool result = android_vulkan::Renderer::CheckVkResult (
        vkCreateDescriptorPool ( device, &poolInfo, nullptr, &_descriptorPool ),
        "pbr::AverageBrightnessPass::CreateDescriptorSet",
        "Can't create descriptor pool"
    );

    if ( !result )
        return false;

    AV_REGISTER_DESCRIPTOR_POOL ( "pbr::AverageBrightnessPass::_descriptorPool" )
    VkDescriptorSetLayout layout = _layout.GetLayout ();

    VkDescriptorSetAllocateInfo const allocateInfo
    {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
        .pNext = nullptr,
        .descriptorPool = _descriptorPool,
        .descriptorSetCount = 1U,
        .pSetLayouts = &layout
    };

    result = android_vulkan::Renderer::CheckVkResult (
        vkAllocateDescriptorSets ( device, &allocateInfo, &_descriptorSet ),
        "pbr::AverageBrightnessPass::CreateDescriptorSet",
        "Can't create descriptor set"
    );

    if ( !result )
        return false;

    VkDescriptorBufferInfo const bufferInfo
    {
        .buffer = _globalCounter,
        .offset = 0U,
        .range = static_cast<VkDeviceSize> ( sizeof ( uint32_t ) )
    };

    VkWriteDescriptorSet const write
    {
        .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
        .pNext = nullptr,
        .dstSet = _descriptorSet,
        .dstBinding = BIND_GLOBAL_ATOMIC,
        .dstArrayElement = 0U,
        .descriptorCount = 1U,
        .descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
        .pImageInfo = nullptr,
        .pBufferInfo = &bufferInfo,
        .pTexelBufferView = nullptr
    };

    vkUpdateDescriptorSets ( device, 1U, &write, 0U, nullptr );
    return true;
}

bool AverageBrightnessPass::CreateGlobalCounter ( android_vulkan::Renderer &renderer,
    VkDevice device,
    VkCommandPool commandPool
) noexcept
{
    constexpr VkBufferUsageFlags usageFlags = AV_VK_FLAG ( VK_BUFFER_USAGE_STORAGE_BUFFER_BIT ) |
        AV_VK_FLAG ( VK_BUFFER_USAGE_TRANSFER_DST_BIT );

    VkBufferCreateInfo bufferInfo
    {
        .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0U,
        .size = sizeof ( uint32_t ),
        .usage = usageFlags,
        .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
        .queueFamilyIndexCount = 0U,
        .pQueueFamilyIndices = nullptr
    };

    bool result = android_vulkan::Renderer::CheckVkResult (
        vkCreateBuffer ( device, &bufferInfo, nullptr, &_globalCounter ),
        "pbr::AverageBrightnessPass::CreateGlobalCounter",
        "Can't create global counter buffer"
    );

    if ( !result )
        return false;

    AV_REGISTER_BUFFER ( "pbr::AverageBrightnessPass::_globalCounter" )

    VkMemoryRequirements memoryRequirements {};
    vkGetBufferMemoryRequirements ( device, _globalCounter, &memoryRequirements );

    result = renderer.TryAllocateMemory ( _globalCounterMemory._memory,
        _globalCounterMemory._offset,
        memoryRequirements,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
        "Can't allocate global counter memory (pbr::AverageBrightnessPass::CreateGlobalCounter)"
    );

    if ( !result )
        return false;

    AV_REGISTER_DEVICE_MEMORY ( "pbr::AverageBrightnessPass::_globalCounterMemory" )

    result = android_vulkan::Renderer::CheckVkResult (
        vkBindBufferMemory ( device, _globalCounter, _globalCounterMemory._memory, _globalCounterMemory._offset ),
        "pbr::AverageBrightnessPass::CreateGlobalCounter",
        "Can't bind global counter memory"
    );

    if ( !result )
        return false;

    bufferInfo.usage = AV_VK_FLAG ( VK_BUFFER_USAGE_TRANSFER_SRC_BIT );

    result = android_vulkan::Renderer::CheckVkResult (
        vkCreateBuffer ( device, &bufferInfo, nullptr, &_transferBuffer ),
        "pbr::AverageBrightnessPass::CreateGlobalCounter",
        "Can't create transfer buffer"
    );

    if ( !result )
        return false;

    AV_REGISTER_BUFFER ( "pbr::AverageBrightnessPass::_transferBuffer" )

    vkGetBufferMemoryRequirements ( device, _transferBuffer, &memoryRequirements );

    constexpr VkMemoryPropertyFlags const memoryFlags = AV_VK_FLAG ( VK_MEMORY_PROPERTY_HOST_COHERENT_BIT ) |
        AV_VK_FLAG ( VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT );

    result = renderer.TryAllocateMemory ( _transferBufferMemory._memory,
        _transferBufferMemory._offset,
        memoryRequirements,
        memoryFlags,
        "Can't allocate transfer buffer memory (pbr::AverageBrightnessPass::CreateGlobalCounter)"
    );

    if ( !result )
        return false;

    AV_REGISTER_DEVICE_MEMORY ( "pbr::AverageBrightnessPass::_transferBufferMemory" )

    result = android_vulkan::Renderer::CheckVkResult (
        vkBindBufferMemory ( device, _transferBuffer, _transferBufferMemory._memory, _transferBufferMemory._offset ),
        "pbr::AverageBrightnessPass::CreateGlobalCounter",
        "Can't bind transfer memory"
    );

    if ( !result )
        return false;

    void* destination = nullptr;

    result = renderer.MapMemory ( destination,
        _transferBufferMemory._memory,
        _transferBufferMemory._offset,
        "pbr::AverageBrightnessPass::CreateGlobalCounter",
        "Can't map transfer memory"
    );

    if ( !result )
        return false;

    auto* mappedBuffer = static_cast<uint8_t*> ( destination );
    std::memset ( mappedBuffer, 0, sizeof ( uint32_t ) );
    renderer.UnmapMemory ( _transferBufferMemory._memory );

    VkCommandBufferAllocateInfo const allocateInfo
    {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
        .pNext = nullptr,
        .commandPool = commandPool,
        .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
        .commandBufferCount = 1U
    };

    result = android_vulkan::Renderer::CheckVkResult (
        vkAllocateCommandBuffers ( device, &allocateInfo, &_commandBuffer ),
        "pbr::AverageBrightnessPass::CreateGlobalCounter",
        "Can't allocate command buffer"
    );

    if ( !result )
        return false;

    constexpr VkCommandBufferBeginInfo beginInfo
    {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
        .pNext = nullptr,
        .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
        .pInheritanceInfo = nullptr
    };

    result = android_vulkan::Renderer::CheckVkResult ( vkBeginCommandBuffer ( _commandBuffer, &beginInfo ),
        "pbr::AverageBrightnessPass::CreateGlobalCounter",
        "Can't begin command buffer"
    );

    if ( !result )
        return false;

    constexpr VkBufferCopy copyInfo
    {
        .srcOffset = 0U,
        .dstOffset = 0U,
        .size = static_cast<VkDeviceSize> ( sizeof ( uint32_t ) )
    };

    vkCmdCopyBuffer ( _commandBuffer, _transferBuffer, _globalCounter, 1U, &copyInfo );

    constexpr VkAccessFlags dstAccessFlags = AV_VK_FLAG ( VK_ACCESS_SHADER_READ_BIT ) |
        AV_VK_FLAG ( VK_ACCESS_SHADER_WRITE_BIT );

    VkBufferMemoryBarrier const barrierInfo =
    {
        .sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER,
        .pNext = nullptr,
        .srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT,
        .dstAccessMask = dstAccessFlags,
        .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
        .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
        .buffer = _globalCounter,
        .offset = 0U,
        .size = copyInfo.size
    };

    vkCmdPipelineBarrier ( _commandBuffer,
        VK_PIPELINE_STAGE_TRANSFER_BIT,
        VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
        0U,
        0U,
        nullptr,
        1U,
        &barrierInfo,
        0U,
        nullptr
    );

    result = android_vulkan::Renderer::CheckVkResult ( vkEndCommandBuffer ( _commandBuffer ),
        "pbr::AverageBrightnessPass::CreateGlobalCounter",
        "Can't end command buffer"
    );

    if ( !result )
        return false;

    VkSubmitInfo const submitInfo
    {
        .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
        .pNext = nullptr,
        .waitSemaphoreCount = 0U,
        .pWaitSemaphores = nullptr,
        .pWaitDstStageMask = nullptr,
        .commandBufferCount = 1U,
        .pCommandBuffers = &_commandBuffer,
        .signalSemaphoreCount = 0U,
        .pSignalSemaphores = nullptr
    };

    return android_vulkan::Renderer::CheckVkResult (
        vkQueueSubmit ( renderer.GetQueue (), 1U, &submitInfo, VK_NULL_HANDLE ),
        "pbr::AverageBrightnessPass::CreateGlobalCounter",
        "Can't submit command"
    );
}

void AverageBrightnessPass::BindTargetToDescriptorSet ( android_vulkan::Texture2D const &/*hdrImage*/ ) noexcept
{
    // TODO
}

bool AverageBrightnessPass::CreateMips ( android_vulkan::Renderer &/*renderer*/,
    VkDevice /*device*/,
    VkExtent2D /*resolution*/
) noexcept
{
    // TODO
    return true;
}

void AverageBrightnessPass::FreeTransferBuffer ( android_vulkan::Renderer &renderer, VkDevice device ) noexcept
{
    if ( _transferBuffer != VK_NULL_HANDLE )
    {
        vkDestroyBuffer ( device, _transferBuffer, nullptr );
        _transferBuffer = VK_NULL_HANDLE;
        AV_UNREGISTER_BUFFER ( "pbr::AverageBrightnessPass::_transferBuffer" )
    }

    if ( _transferBufferMemory._memory == VK_NULL_HANDLE )
        return;

    renderer.FreeMemory ( _transferBufferMemory._memory, _transferBufferMemory._offset );
    _transferBufferMemory._memory = VK_NULL_HANDLE;
    _transferBufferMemory._offset = std::numeric_limits<VkDeviceSize>::max ();
    AV_UNREGISTER_DEVICE_MEMORY ( "pbr::AverageBrightnessPass::_transferBufferMemory" )
}

} // namespace pbr
