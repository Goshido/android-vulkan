#include <pbr/point_light_lightup.h>

GX_DISABLE_COMMON_WARNINGS

#include <cassert>

GX_RESTORE_WARNING_STATE

#include <pbr/point_light_descriptor_set_layout.h>
#include <pbr/point_light_pass.h>


namespace pbr {

constexpr static float const UNITS_PER_METER = 32.0F;

PointLightLightup::PointLightLightup () noexcept:
    _transferCommandBuffer ( VK_NULL_HANDLE ),
    _commandPool ( VK_NULL_HANDLE ),
    _descriptorPool ( VK_NULL_HANDLE ),
    _descriptorSets {},
    _imageInfo {},
    _program {},
    _sampler {},
    _submitInfoTransfer {},
    _uniformInfoLightData {},
    _uniformPoolLightData ( eUniformPoolSize::Tiny_4M ),
    _writeSets {}
{
    // NOTHING
}

bool PointLightLightup::Init ( android_vulkan::Renderer &renderer,
    VkCommandPool commandPool,
    VkRenderPass renderPass,
    uint32_t subpass,
    VkExtent2D const &resolution
)
{
    _commandPool = commandPool;

    VkCommandBufferAllocateInfo const allocateInfo
    {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
        .pNext = nullptr,
        .commandPool = _commandPool,
        .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
        .commandBufferCount = 1U
    };

    VkDevice device = renderer.GetDevice ();

    bool result = android_vulkan::Renderer::CheckVkResult (
        vkAllocateCommandBuffers ( device, &allocateInfo, &_transferCommandBuffer ),
        "pbr::PointLightLightup::Init",
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

    if ( !_program.Init ( renderer, renderPass, subpass, resolution ) )
    {
        Destroy ( device );
        return false;
    }

    constexpr VkSamplerCreateInfo const samplerInfo
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
        .compareEnable = VK_TRUE,
        .compareOp = VK_COMPARE_OP_LESS,
        .minLod = 0.0F,
        .maxLod = 0.0F,
        .borderColor = VK_BORDER_COLOR_FLOAT_TRANSPARENT_BLACK,
        .unnormalizedCoordinates = VK_FALSE
    };

    if ( !_sampler.Init ( renderer, samplerInfo ) )
    {
        Destroy ( device );
        return false;
    }

    if ( !_uniformPoolLightData.Init ( renderer, sizeof ( PointLightLightupProgram::LightData ) ) )
    {
        Destroy ( device );
        return false;
    }

    return true;
}

void PointLightLightup::Destroy ( VkDevice device )
{
    DestroyDescriptorPool ( device );

    _imageInfo.clear ();
    _uniformInfoLightData.clear ();
    _descriptorSets.clear ();
    _writeSets.clear ();

    _uniformPoolLightData.Destroy ( device );
    _sampler.Destroy ( device );
    _program.Destroy ( device );

    if ( _commandPool == VK_NULL_HANDLE )
        return;

    vkFreeCommandBuffers ( device, _commandPool, 1U, &_transferCommandBuffer );
    _transferCommandBuffer = VK_NULL_HANDLE;
    _commandPool = VK_NULL_HANDLE;
}

void PointLightLightup::Lightup ( VkCommandBuffer commandBuffer,
    android_vulkan::MeshGeometry &unitCube,
    size_t lightIndex
)
{
    _program.Bind ( commandBuffer );
    _program.SetLightData ( commandBuffer, _descriptorSets[ lightIndex ] );

    vkCmdDrawIndexed ( commandBuffer, unitCube.GetVertexCount (), 1U, 0U, 0, 0U );
}

bool PointLightLightup::UpdateGPUData ( android_vulkan::Renderer &renderer,
    PointLightPass const &pointLightPass,
    GXMat4 const &viewerLocal,
    GXMat4 const &view
)
{
    _uniformPoolLightData.Reset();
    size_t const lightCount = pointLightPass.GetPointLightCount ();

    if ( !AllocateNativeDescriptorSets ( renderer, lightCount ) )
        return false;

    constexpr VkCommandBufferBeginInfo const beginInfo
    {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
        .pNext = nullptr,
        .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
        .pInheritanceInfo = nullptr
    };

    bool result = android_vulkan::Renderer::CheckVkResult ( vkBeginCommandBuffer ( _transferCommandBuffer, &beginInfo ),
        "pbr::PointLightLightup::UpdateGPUData",
        "Can't begin command buffer"
    );

    if ( !result )
        return false;

    size_t writeIndex = 0U;
    GXVec3 alpha {};
    GXVec3 betta {};

    constexpr float const gamma = 1.0F / UNITS_PER_METER;
    PointLightLightupProgram::LightData lightData {};
    lightData._sceneScaleFactor = gamma;

    // Note all shadowmap formats are same so grab first shadowmap and resolve.
    auto const [probeLight, probeShadowmap] = pointLightPass.GetPointLightInfo ( 0U );

    VkImageMemoryBarrier barrier
    {
        .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
        .pNext = nullptr,
        .srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
        .dstAccessMask = VK_ACCESS_SHADER_READ_BIT,
        .oldLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
        .newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
        .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
        .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
        .image = VK_NULL_HANDLE,

        .subresourceRange
        {
            .aspectMask = android_vulkan::Renderer::ResolveImageViewAspect ( probeShadowmap->GetFormat () ),
            .baseMipLevel = 0U,
            .levelCount = 1U,
            .baseArrayLayer = 0U,
            .layerCount = 6U
        }
    };

    for ( size_t i = 0U; i < lightCount; ++i )
    {
        auto const [light, shadowmap] = pointLightPass.GetPointLightInfo ( i );
        VkDescriptorSet set = _descriptorSets[ i ];

        VkDescriptorImageInfo& image = _imageInfo[ i ];
        image.imageView = shadowmap->GetImageView ();
        barrier.image = shadowmap->GetImage ();

        vkCmdPipelineBarrier ( _transferCommandBuffer,
            VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT,
            VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
            0U,
            0U,
            nullptr,
            0U,
            nullptr,
            1U,
            &barrier
        );

        VkWriteDescriptorSet &write0 = _writeSets[ writeIndex++ ];
        write0.dstBinding = 0U;
        write0.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;

        VkWriteDescriptorSet &write1 = _writeSets[ writeIndex++ ];
        write1.dstBinding = 1U;
        write1.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLER;

        write0.dstSet = write1.dstSet = set;
        write0.pImageInfo = write1.pImageInfo = &image;
        write0.pBufferInfo = write1.pBufferInfo = nullptr;

        lightData._lightProjection = light->GetProjection ();

        // Matrix optimization:
        // Point light has identity orientation by design. Point light changes its location only. Because of that
        // point light view matrix is negative "w" component. Additionally this method receives viewer local matrix
        // which is an inverse transform from view matrix. So "view to point light" transform simplifies to
        // one subtraction operation of point light location vector from "w" component of the viewer local matrix.

        GXMat4& viewToPointLight = lightData._viewToLight;
        viewToPointLight = viewerLocal;

        viewToPointLight.GetW ( alpha );
        GXVec3 const& location = light->GetLocation ();
        betta.Subtract ( alpha, location );
        viewToPointLight.SetW ( betta );

        lightData._hue = light->GetHue ();
        lightData._intensity = light->GetIntensity ();

        view.MultiplyAsPoint ( lightData._lightLocationView, location );
        VkDescriptorBufferInfo& buffer = _uniformInfoLightData[ i ];

        buffer.buffer = _uniformPoolLightData.Acquire ( renderer,
            _transferCommandBuffer,
            &lightData,
            VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT
        );

        VkWriteDescriptorSet &write2 = _writeSets[ writeIndex++ ];
        write2.dstSet = set;
        write2.dstBinding = 2U;
        write2.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        write2.pImageInfo = nullptr;
        write2.pBufferInfo = &buffer;
    }

    result = android_vulkan::Renderer::CheckVkResult ( vkEndCommandBuffer ( _transferCommandBuffer ),
        "pbr::PointLightLightup::UpdateGPUData",
        "Can't end command buffer"
    );

    if ( !result )
        return false;

    result = android_vulkan::Renderer::CheckVkResult (
        vkQueueSubmit ( renderer.GetQueue (), 1U, &_submitInfoTransfer, VK_NULL_HANDLE ),
        "pbr::PointLightLightup::UpdateGPUData",
        "Can't submit command buffer"
    );

    if ( !result )
        return false;

    vkUpdateDescriptorSets ( renderer.GetDevice (),
        static_cast<uint32_t> ( _writeSets.size () ),
        _writeSets.data (),
        0U,
        nullptr
    );

    return true;
}

bool PointLightLightup::AllocateNativeDescriptorSets ( android_vulkan::Renderer &renderer, size_t neededSets )
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

    bool const result = android_vulkan::Renderer::CheckVkResult (
        vkCreateDescriptorPool ( device, &poolInfo, nullptr, &_descriptorPool ),
        "pbr::PointLightLightup::AllocateNativeDescriptorSets",
        "Can't create descriptor pool"
    );

    if ( !result )
        return false;

    AV_REGISTER_DESCRIPTOR_POOL ( "PointLightLightup::_descriptorPool" )

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
        .range = static_cast<VkDeviceSize> ( sizeof ( PointLightLightupProgram::LightData ) )
    };

    _uniformInfoLightData.resize ( neededSets, uniform );

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

    _writeSets.resize ( 3U * neededSets, writeSet );
    _descriptorSets.resize ( neededSets, VK_NULL_HANDLE );

    PointLightDescriptorSetLayout const layout;
    std::vector<VkDescriptorSetLayout> layouts ( neededSets, layout.GetLayout () );

    VkDescriptorSetAllocateInfo const allocateInfo
    {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
        .pNext = nullptr,
        .descriptorPool = _descriptorPool,
        .descriptorSetCount = poolInfo.maxSets,
        .pSetLayouts = layouts.data ()
    };

    return android_vulkan::Renderer::CheckVkResult (
        vkAllocateDescriptorSets ( device, &allocateInfo, _descriptorSets.data () ),
        "pbr::PointLightLightup::AllocateNativeDescriptorSets",
        "Can't allocate descriptor sets"
    );
}

void PointLightLightup::DestroyDescriptorPool ( VkDevice device )
{
    if ( _descriptorPool == VK_NULL_HANDLE )
        return;

    vkDestroyDescriptorPool ( device, _descriptorPool, nullptr );
    _descriptorPool = VK_NULL_HANDLE;
    AV_UNREGISTER_DESCRIPTOR_POOL ( "PointLightLightup::_descriptorPool" )
}

} // namespace pbr
