#include <precompiled_headers.hpp>
#include <pbr/brightness_factor.inc>
#include <platform/android/pbr/full_screen_triangle.inc>
#include <platform/android/pbr/tone_mapper.inc>
#include <platform/android/pbr/tone_mapper_pass.hpp>
#include <vulkan_utils.hpp>


// FUCK - remove namespace
namespace pbr::android {

bool ToneMapperPass::Init ( android_vulkan::Renderer &renderer ) noexcept
{
    constexpr static VkDescriptorPoolSize const poolSizes[] =
    {
        {
            .type = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,
            .descriptorCount = 1U
        },
        {
            .type = VK_DESCRIPTOR_TYPE_SAMPLER,
            .descriptorCount = 1U,
        },
        {
            .type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
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

    VkDevice device = renderer.GetDevice ();

    bool result = android_vulkan::Renderer::CheckVkResult (
        vkCreateDescriptorPool ( device, &poolInfo, nullptr, &_resourceDescriptorPool ),

        // FUCK - remove namespace
        "pbr::android::ToneMapperPass::Init",

        "Can't create descriptor pool"
    );

    if ( !result ) [[unlikely]]
        return false;

    AV_SET_VULKAN_OBJECT_NAME ( device, _resourceDescriptorPool, VK_OBJECT_TYPE_DESCRIPTOR_POOL, "Tone mapper" )

    if ( !_transformLayout.Init ( device ) || !_resourceLayout.Init ( device ) ) [[unlikely]]
        return false;

    VkDescriptorSetAllocateInfo const allocateInfo
    {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
        .pNext = nullptr,
        .descriptorPool = _resourceDescriptorPool,
        .descriptorSetCount = 1U,
        .pSetLayouts = &_resourceLayout.GetLayout ()
    };

    result = android_vulkan::Renderer::CheckVkResult (
        vkAllocateDescriptorSets ( device, &allocateInfo, _descriptorSets + SET_RESOURCE ),

        // FUCK - remove namespace
        "pbr::android::ToneMapperPass::Init",

        "Can't create descriptor set"
    );

    if ( !result ) [[unlikely]]
        return false;

    AV_SET_VULKAN_OBJECT_NAME ( device,
        _descriptorSets[ SET_RESOURCE ],
        VK_OBJECT_TYPE_DESCRIPTOR_SET,
        "Tone mapper resource"
    )

    return _transformUniformPool.Init ( renderer,
        _transformLayout,
        sizeof ( ToneMapperProgram::Transform ),
        BIND_TRANSFORM,
        "Tone mapper transform uniform"
    );
}

void ToneMapperPass::Destroy ( android_vulkan::Renderer &renderer ) noexcept
{
    VkDevice device = renderer.GetDevice ();

    if ( _resourceDescriptorPool != VK_NULL_HANDLE ) [[likely]]
        vkDestroyDescriptorPool ( device, std::exchange ( _resourceDescriptorPool, VK_NULL_HANDLE ), nullptr );

    _program.Destroy ( device );
    _transformLayout.Destroy ( device );
    _resourceLayout.Destroy ( device );
    _transformUniformPool.Destroy ( renderer );
}

void ToneMapperPass::Execute ( VkCommandBuffer commandBuffer ) noexcept
{
    AV_VULKAN_GROUP ( commandBuffer, "Tone mapping" )
    _program.Bind ( commandBuffer );
    _program.SetDescriptorSets ( commandBuffer, _descriptorSets );
    vkCmdDraw ( commandBuffer, 3U, 1U, 0U, 0U );
}

bool ToneMapperPass::SetBrightness ( android_vulkan::Renderer &renderer,
    VkRenderPass renderPass,
    uint32_t subpass,
    float brightnessBalance
) noexcept
{
    _program.Destroy ( renderer.GetDevice () );

    bool const result = _program.Init ( renderer,
        renderPass,
        subpass,
        BrightnessInfo ( brightnessBalance ),
        renderer.GetSurfaceSize ()
    );

    if ( !result ) [[unlikely]]
        return false;

    _brightnessBalance = brightnessBalance;
    return true;
}

bool ToneMapperPass::SetTarget ( android_vulkan::Renderer &renderer,
    VkRenderPass renderPass,
    uint32_t subpass,
    VkImageView hdrView,
    VkBuffer exposure,
    VkSampler clampToEdgeSampler
) noexcept
{
    VkDevice device = renderer.GetDevice ();
    _program.Destroy ( device );

    bool const result = _program.Init ( renderer,
        renderPass,
        subpass,
        BrightnessInfo ( _brightnessBalance ),
        renderer.GetSurfaceSize ()
    );

    if ( !result ) [[unlikely]]
        return false;

    VkDescriptorImageInfo const imageInfo
    {
        .sampler = clampToEdgeSampler,
        .imageView = hdrView,
        .imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
    };

    VkDescriptorBufferInfo const exposureInfo
    {
        .buffer = exposure,
        .offset = 0U,
        .range = static_cast<VkDeviceSize> ( sizeof ( float ) )
    };

    VkDescriptorSet resourceDescriptorSet = _descriptorSets[ SET_RESOURCE ];

    VkWriteDescriptorSet const writes[] =
    {
        {
            .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
            .pNext = nullptr,
            .dstSet = resourceDescriptorSet,
            .dstBinding = BIND_HDR_IMAGE,
            .dstArrayElement = 0U,
            .descriptorCount = 1U,
            .descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,
            .pImageInfo = &imageInfo,
            .pBufferInfo = nullptr,
            .pTexelBufferView = nullptr
        },
        {
            .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
            .pNext = nullptr,
            .dstSet = resourceDescriptorSet,
            .dstBinding = BIND_CLAMP_TO_EDGE_SAMPLER,
            .dstArrayElement = 0U,
            .descriptorCount = 1U,
            .descriptorType = VK_DESCRIPTOR_TYPE_SAMPLER,
            .pImageInfo = &imageInfo,
            .pBufferInfo = nullptr,
            .pTexelBufferView = nullptr
        },
        {
            .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
            .pNext = nullptr,
            .dstSet = resourceDescriptorSet,
            .dstBinding = BIND_EXPOSURE,
            .dstArrayElement = 0U,
            .descriptorCount = 1U,
            .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
            .pImageInfo = nullptr,
            .pBufferInfo = &exposureInfo,
            .pTexelBufferView = nullptr
        }
    };

    vkUpdateDescriptorSets ( device, static_cast<uint32_t> ( std::size ( writes ) ), writes, 0U, nullptr );
    _transformUpdated = true;
    return true;
}

void ToneMapperPass::UploadGPUData ( android_vulkan::Renderer &renderer, VkCommandBuffer commandBuffer ) noexcept
{
    if ( !_transformUpdated ) [[likely]]
        return;

    AV_VULKAN_GROUP ( commandBuffer, "Upload tone mapper data" )
    GXMat4 const &orientation = renderer.GetPresentationEngineTransform ();

    ToneMapperProgram::Transform const transform
    {
        ._transformRow0 = *reinterpret_cast<GXVec2 const*> ( orientation._data[ 0U ] ),
        ._padding0 {},
        ._transformRow1 = *reinterpret_cast<GXVec2 const*> ( orientation._data[ 1U ] )
    };

    _transformUniformPool.Push ( commandBuffer, &transform, sizeof ( transform ) );
    _descriptorSets[ SET_TRANSFORM ] = _transformUniformPool.Acquire ();

    _transformUniformPool.IssueSync ( renderer.GetDevice (), commandBuffer );
    _transformUniformPool.Commit ();

    _transformUpdated = false;
}

} // namespace pbr::android
