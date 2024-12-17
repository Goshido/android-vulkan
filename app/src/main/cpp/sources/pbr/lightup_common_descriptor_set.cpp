#include <precompiled_headers.hpp>
#include <vulkan_utils.hpp>
#include <pbr/lightup_common_descriptor_set.hpp>
#include <pbr/light_lightup_base_program.hpp>


namespace pbr {

namespace {

constexpr char const BRDF_LUT[] = "pbr/system/brdf-lut.png";

// 256 128 64 32 16 8 4 2 1
// counting from 0.0F
constexpr float MAX_PREFILTER_LOD = 8.0F;

} // end of anonymous namespace

//----------------------------------------------------------------------------------------------------------------------

void LightupCommonDescriptorSet::Bind ( VkCommandBuffer commandBuffer, size_t commandBufferIndex ) noexcept
{
    vkCmdBindDescriptorSets ( commandBuffer,
        VK_PIPELINE_BIND_POINT_GRAPHICS,
        _pipelineLayout,
        0U,
        1U,
        &_sets[ commandBufferIndex ],
        0U,
        nullptr
    );
}

bool LightupCommonDescriptorSet::Init ( android_vulkan::Renderer &renderer,
    VkCommandPool commandPool,
    GBuffer &gBuffer
) noexcept
{
    VkDevice device = renderer.GetDevice ();

    constexpr static VkDescriptorPoolSize const poolSizes[] =
    {
        {
            .type = VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT,
            .descriptorCount = static_cast<uint32_t> ( 4U * DUAL_COMMAND_BUFFER )
        },
        {
            .type = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,
            .descriptorCount = static_cast<uint32_t> ( DUAL_COMMAND_BUFFER )
        },
        {
            .type = VK_DESCRIPTOR_TYPE_SAMPLER,
            .descriptorCount = static_cast<uint32_t> ( 2U * DUAL_COMMAND_BUFFER )
        },
        {
            .type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
            .descriptorCount = static_cast<uint32_t> ( DUAL_COMMAND_BUFFER )
        }
    };

    constexpr VkDescriptorPoolCreateInfo poolInfo
    {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0U,
        .maxSets = static_cast<uint32_t> ( DUAL_COMMAND_BUFFER ),
        .poolSizeCount = static_cast<uint32_t> ( std::size ( poolSizes ) ),
        .pPoolSizes = poolSizes
    };

    bool result = android_vulkan::Renderer::CheckVkResult (
        vkCreateDescriptorPool ( device, &poolInfo, nullptr, &_descriptorPool ),
        "pbr::LightupCommonDescriptorSet::Init",
        "Can't create descriptor pool"
    );

    if ( !result ) [[unlikely]]
        return false;

    AV_SET_VULKAN_OBJECT_NAME ( device, _descriptorPool, VK_OBJECT_TYPE_DESCRIPTOR_POOL, "Lightup common" )

    if ( !_layout.Init ( device ) ) [[unlikely]]
        return false;

    VkDescriptorSetLayout layout = _layout.GetLayout ();
    std::vector<VkDescriptorSetLayout> layouts ( DUAL_COMMAND_BUFFER, layout );

    VkDescriptorSetAllocateInfo const descriptorSetAllocateInfo
    {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
        .pNext = nullptr,
        .descriptorPool = _descriptorPool,
        .descriptorSetCount = static_cast<uint32_t> ( DUAL_COMMAND_BUFFER ),
        .pSetLayouts = layouts.data ()
    };

    result = android_vulkan::Renderer::CheckVkResult (
        vkAllocateDescriptorSets ( device, &descriptorSetAllocateInfo, _sets ),
        "pbr::LightupCommonDescriptorSet::Init",
        "Can't allocate descriptor set"
    );

    if ( !result ) [[unlikely]]
        return false;

#if defined ( AV_ENABLE_VVL ) || defined ( AV_ENABLE_RENDERDOC )

    for ( size_t i = 0U; i < DUAL_COMMAND_BUFFER; ++i )
        AV_SET_VULKAN_OBJECT_NAME ( device, _sets[ i ], VK_OBJECT_TYPE_DESCRIPTOR_SET, "Lightup common [FIF #%zu]", i )

#endif // AV_ENABLE_VVL || AV_ENABLE_RENDERDOC

    if ( !_uniforms.Init ( renderer, sizeof ( LightLightupBaseProgram::ViewData ), "Light common" ) ) [[unlikely]]
        return false;

    VkCommandBufferAllocateInfo const bufferAllocateInfo
    {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
        .pNext = nullptr,
        .commandPool = commandPool,
        .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
        .commandBufferCount = 1U
    };

    VkCommandBuffer textureCommandBuffer;

    result = android_vulkan::Renderer::CheckVkResult (
        vkAllocateCommandBuffers ( device, &bufferAllocateInfo, &textureCommandBuffer ),
        "pbr::LightupCommonDescriptorSet::Init",
        "Can't allocate command buffer"
    );

    if ( !result ) [[unlikely]]
        return false;

    AV_SET_VULKAN_OBJECT_NAME ( device, textureCommandBuffer, VK_OBJECT_TYPE_COMMAND_BUFFER, "BRDF LUT" )

    result = _brdfLUT.UploadData ( renderer,
        BRDF_LUT,
        android_vulkan::eFormat::Unorm,
        false,
        textureCommandBuffer,
        VK_NULL_HANDLE
    );

    if ( !result ) [[unlikely]]
        return false;

    AV_SET_VULKAN_OBJECT_NAME ( device, _brdfLUT.GetImage (), VK_OBJECT_TYPE_IMAGE, "BRDF LUT" )
    AV_SET_VULKAN_OBJECT_NAME ( device, _brdfLUT.GetImageView (), VK_OBJECT_TYPE_IMAGE_VIEW, "BRDF LUT" )

    constexpr VkSamplerCreateInfo brdfSamplerInfo
    {
        .sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0U,
        .magFilter = VK_FILTER_LINEAR,
        .minFilter = VK_FILTER_LINEAR,
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

    if ( !_brdfLUTSampler.Init ( device, brdfSamplerInfo, "BRDF LUT" ) ) [[unlikely]]
        return false;

    constexpr VkSamplerCreateInfo prefilterSamplerInfo
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

    if ( !_prefilterSampler.Init ( device, prefilterSamplerInfo, "Prefilter" ) ) [[unlikely]]
        return false;

    VkDescriptorImageInfo const images[] =
    {
        {
            .sampler = VK_NULL_HANDLE,
            .imageView = gBuffer.GetAlbedo ().GetImageView (),
            .imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
        },
        {
            .sampler = VK_NULL_HANDLE,
            .imageView = gBuffer.GetNormal ().GetImageView (),
            .imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
        },
        {
            .sampler = VK_NULL_HANDLE,
            .imageView = gBuffer.GetParams ().GetImageView (),
            .imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
        },
        {
            .sampler = VK_NULL_HANDLE,
            .imageView = gBuffer.GetDepthStencil ().GetImageView (),
            .imageLayout = VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_OPTIMAL
        },
        {
            .sampler = _brdfLUTSampler.GetSampler (),
            .imageView = _brdfLUT.GetImageView (),
            .imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
        },
        {
            .sampler = _prefilterSampler.GetSampler (),
            .imageView = VK_NULL_HANDLE,
            .imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
        }
    };

    VkWriteDescriptorSet write[ 7U * DUAL_COMMAND_BUFFER ];
    size_t idx = 0U;

    for ( auto &set : _sets )
    {
        VkWriteDescriptorSet &albedo = write[ idx++ ];

        albedo = VkWriteDescriptorSet
        {
            .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
            .pNext = nullptr,
            .dstSet = set,
            .dstBinding = 0U,
            .dstArrayElement = 0U,
            .descriptorCount = 1U,
            .descriptorType = VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT,
            .pImageInfo = images,
            .pBufferInfo = nullptr,
            .pTexelBufferView = nullptr
        };

        VkWriteDescriptorSet &normal = write[ idx++ ];

        normal = VkWriteDescriptorSet
        {
            .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
            .pNext = nullptr,
            .dstSet = set,
            .dstBinding = 1U,
            .dstArrayElement = 0U,
            .descriptorCount = 1U,
            .descriptorType = VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT,
            .pImageInfo = images + 1U,
            .pBufferInfo = nullptr,
            .pTexelBufferView = nullptr
        };

        VkWriteDescriptorSet &params = write[ idx++ ];

        params = VkWriteDescriptorSet
        {
            .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
            .pNext = nullptr,
            .dstSet = set,
            .dstBinding = 2U,
            .dstArrayElement = 0U,
            .descriptorCount = 1U,
            .descriptorType = VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT,
            .pImageInfo = images + 2U,
            .pBufferInfo = nullptr,
            .pTexelBufferView = nullptr
        };

        VkWriteDescriptorSet &depthStencil = write[ idx++ ];

        depthStencil = VkWriteDescriptorSet
        {
            .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
            .pNext = nullptr,
            .dstSet = set,
            .dstBinding = 3U,
            .dstArrayElement = 0U,
            .descriptorCount = 1U,
            .descriptorType = VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT,
            .pImageInfo = images + 3U,
            .pBufferInfo = nullptr,
            .pTexelBufferView = nullptr
        };

        VkWriteDescriptorSet &brdfImage = write[ idx++ ];

        brdfImage = VkWriteDescriptorSet
        {
            .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
            .pNext = nullptr,
            .dstSet = set,
            .dstBinding = 4U,
            .dstArrayElement = 0U,
            .descriptorCount = 1U,
            .descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,
            .pImageInfo = images + 4U,
            .pBufferInfo = nullptr,
            .pTexelBufferView = nullptr
        };

        VkWriteDescriptorSet &brdfSampler = write[ idx++ ];

        brdfSampler = VkWriteDescriptorSet
        {
            .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
            .pNext = nullptr,
            .dstSet = set,
            .dstBinding = 5U,
            .dstArrayElement = 0U,
            .descriptorCount = 1U,
            .descriptorType = VK_DESCRIPTOR_TYPE_SAMPLER,
            .pImageInfo = images + 4U,
            .pBufferInfo = nullptr,
            .pTexelBufferView = nullptr
        };

        VkWriteDescriptorSet &prefilterSampler = write[ idx++ ];

        prefilterSampler = VkWriteDescriptorSet
        {
            .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
            .pNext = nullptr,
            .dstSet = set,
            .dstBinding = 6U,
            .dstArrayElement = 0U,
            .descriptorCount = 1U,
            .descriptorType = VK_DESCRIPTOR_TYPE_SAMPLER,
            .pImageInfo = images + 5U,
            .pBufferInfo = nullptr,
            .pTexelBufferView = nullptr
        };
    }

    vkUpdateDescriptorSets ( device, static_cast<uint32_t> ( std::size ( write ) ), write, 0U, nullptr );

    VkPipelineLayoutCreateInfo const layoutInfo
    {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0U,
        .setLayoutCount = 1U,
        .pSetLayouts = &layout,
        .pushConstantRangeCount = 0U,
        .pPushConstantRanges = nullptr
    };

    result = android_vulkan::Renderer::CheckVkResult (
        vkCreatePipelineLayout ( device, &layoutInfo, nullptr, &_pipelineLayout ),
        "pbr::LightupCommonDescriptorSet::Init",
        "Can't create pipeline layout"
    );

    if ( !result ) [[unlikely]]
        return false;

    AV_SET_VULKAN_OBJECT_NAME ( device, _pipelineLayout, VK_OBJECT_TYPE_PIPELINE_LAYOUT, "Lightup common" )

    _bufferInfo.offset = 0U;
    _bufferInfo.range = static_cast<VkDeviceSize> ( sizeof ( LightLightupBaseProgram::ViewData ) );

    _writeInfo.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    _writeInfo.pNext = nullptr;
    _writeInfo.dstBinding = 7U;
    _writeInfo.dstArrayElement = 0U;
    _writeInfo.descriptorCount = 1U;
    _writeInfo.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    _writeInfo.pImageInfo = nullptr;
    _writeInfo.pBufferInfo = &_bufferInfo;
    _writeInfo.pTexelBufferView = nullptr;

    _barrier.sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER;
    _barrier.pNext = nullptr;
    _barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    _barrier.dstAccessMask = VK_ACCESS_UNIFORM_READ_BIT;
    _barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    _barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    _barrier.offset = 0U;
    _barrier.size = static_cast<VkDeviceSize> ( sizeof ( LightLightupBaseProgram::ViewData ) );

    return true;
}

void LightupCommonDescriptorSet::Destroy ( android_vulkan::Renderer &renderer ) noexcept
{
    VkDevice device = renderer.GetDevice ();
    _prefilterSampler.Destroy ( device );
    _brdfLUTSampler.Destroy ( device );
    _brdfLUT.FreeResources ( renderer );
    _uniforms.Destroy ( renderer );

    if ( _pipelineLayout != VK_NULL_HANDLE ) [[likely]]
    {
        vkDestroyPipelineLayout ( device, _pipelineLayout, nullptr );
        _pipelineLayout = VK_NULL_HANDLE;
    }

    _layout.Destroy ( device );

    if ( _descriptorPool == VK_NULL_HANDLE ) [[unlikely]]
        return;

    vkDestroyDescriptorPool ( device, _descriptorPool, nullptr );
    _descriptorPool = VK_NULL_HANDLE;
}

void LightupCommonDescriptorSet::OnFreeTransferResources ( android_vulkan::Renderer &renderer ) noexcept
{
    _brdfLUT.FreeTransferResources ( renderer );
}

void LightupCommonDescriptorSet::Update ( VkDevice device,
    VkCommandBuffer commandBuffer,
    size_t commandBufferIndex,
    VkExtent2D const &resolution,
    GXMat4 const &viewerLocal,
    GXMat4 const &cvvToView
) noexcept
{
    AV_VULKAN_GROUP ( commandBuffer, "Upload light data" )

    LightLightupBaseProgram::ViewData const viewData
    {
        ._cvvToView = cvvToView,
        ._viewToWorld = viewerLocal,

        ._invResolutionFactor
        {
            2.0F / static_cast<float> ( resolution.width ),
            2.0F / static_cast<float> ( resolution.height )
        },

        ._padding0_0 {}
    };

    if ( _uniforms.GetAvailableItemCount () < 1U )
        _uniforms.Reset ();

    VkBuffer buffer = _uniforms.Push ( commandBuffer, &viewData, sizeof ( viewData ) );

    _bufferInfo.buffer = buffer;
    _writeInfo.dstSet = _sets[ commandBufferIndex ];
    vkUpdateDescriptorSets ( device, 1U, &_writeInfo, 0U, nullptr );

    _barrier.buffer = buffer;

    vkCmdPipelineBarrier ( commandBuffer,
        VK_PIPELINE_STAGE_TRANSFER_BIT,
        VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
        0U,
        0U,
        nullptr,
        1U,
        &_barrier,
        0U,
        nullptr
    );
}

} // namespace pbr
