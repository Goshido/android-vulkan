#include <vulkan_utils.h>
#include <pbr/lightup_common_descriptor_set.h>
#include <pbr/light_lightup_base_program.h>


namespace pbr {

LightupCommonDescriptorSet::LightupCommonDescriptorSet () noexcept:
    _commandPool ( VK_NULL_HANDLE ),
    _descriptorPool ( VK_NULL_HANDLE ),
    _layout {},
    _set ( VK_NULL_HANDLE ),
    _uniformBuffer {}
{
    // NOTHING
}

bool LightupCommonDescriptorSet::Init ( android_vulkan::Renderer &renderer, GBuffer &gBuffer )
{
    constexpr static VkDescriptorPoolSize const poolSizes[] =
    {
        {
            .type = VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT,
            .descriptorCount = 4U
        },
        {
            .type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
            .descriptorCount = 1U
        }
    };

    constexpr static VkDescriptorPoolCreateInfo const poolInfo
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
        vkCreateDescriptorPool ( device, &poolInfo, nullptr, &_descriptorPool ),
        "LightupCommonDescriptorSet::Init",
        "Can't create descriptor pool"
    );

    if ( !result )
        return false;

    AV_REGISTER_DESCRIPTOR_POOL ( "LightupCommonDescriptorSet::_descriptorPool" )

    if ( !_layout.Init ( renderer ) )
    {
        Destroy ( device );
        return false;
    }

    VkDescriptorSetLayout nativeLayout = _layout.GetLayout ();

    VkDescriptorSetAllocateInfo const allocateInfo
    {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
        .pNext = nullptr,
        .descriptorPool = _descriptorPool,
        .descriptorSetCount = 1U,
        .pSetLayouts = &nativeLayout
    };

    result = android_vulkan::Renderer::CheckVkResult ( vkAllocateDescriptorSets ( device, &allocateInfo, &_set ),
        "LightupCommonDescriptorSet::Init",
        "Can't allocate descriptor set"
    );

    if ( !result )
    {
        Destroy ( device );
        return false;
    }

    VkCommandPoolCreateInfo const commandPoolInfo
    {
        .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
        .pNext = nullptr,
        .flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
        .queueFamilyIndex = renderer.GetQueueFamilyIndex ()
    };

    result = android_vulkan::Renderer::CheckVkResult (
        vkCreateCommandPool ( device, &commandPoolInfo, nullptr, &_commandPool ),
        "LightupCommonDescriptorSet::Init",
        "Can't create command pool"
    );

    if ( !result )
    {
        Destroy ( device );
        return false;
    }

    AV_REGISTER_COMMAND_POOL ( "LightupCommonDescriptorSet::_commandPool" )

    if ( !_uniformBuffer.Init ( renderer, _commandPool, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT ) )
    {
        Destroy ( device );
        return false;
    }

    LightLightupBaseProgram::ViewData const dummy {};

    if ( !_uniformBuffer.Update ( renderer, reinterpret_cast<uint8_t const*> ( &dummy ), sizeof ( dummy ) ) )
    {
        Destroy ( device );
        return false;
    }

    VkDescriptorBufferInfo const buffer
    {
        .buffer = _uniformBuffer.GetBuffer (),
        .offset = 0U,
        .range = static_cast<VkDeviceSize> ( sizeof ( LightLightupBaseProgram::ViewData ) )
    };

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
            .imageView = gBuffer.GetReadOnlyDepthImageView (),
            .imageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL
        }
    };

    VkWriteDescriptorSet const writeInfo[]
    {
        {
            .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
            .pNext = nullptr,
            .dstSet = _set,
            .dstBinding = 0U,
            .dstArrayElement = 0U,
            .descriptorCount = 1U,
            .descriptorType = VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT,
            .pImageInfo = images,
            .pBufferInfo = nullptr,
            .pTexelBufferView = nullptr
        },
        {
            .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
            .pNext = nullptr,
            .dstSet = _set,
            .dstBinding = 1U,
            .dstArrayElement = 0U,
            .descriptorCount = 1U,
            .descriptorType = VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT,
            .pImageInfo = images + 1U,
            .pBufferInfo = nullptr,
            .pTexelBufferView = nullptr
        },
        {
            .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
            .pNext = nullptr,
            .dstSet = _set,
            .dstBinding = 2U,
            .dstArrayElement = 0U,
            .descriptorCount = 1U,
            .descriptorType = VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT,
            .pImageInfo = images + 2U,
            .pBufferInfo = nullptr,
            .pTexelBufferView = nullptr
        },
        {
            .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
            .pNext = nullptr,
            .dstSet = _set,
            .dstBinding = 3U,
            .dstArrayElement = 0U,
            .descriptorCount = 1U,
            .descriptorType = VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT,
            .pImageInfo = images + 3U,
            .pBufferInfo = nullptr,
            .pTexelBufferView = nullptr
        },
        {
            .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
            .pNext = nullptr,
            .dstSet = _set,
            .dstBinding = 4U,
            .dstArrayElement = 0U,
            .descriptorCount = 1U,
            .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
            .pImageInfo = nullptr,
            .pBufferInfo = &buffer,
            .pTexelBufferView = nullptr
        }
    };

    vkUpdateDescriptorSets ( device, static_cast<uint32_t> ( std::size ( writeInfo ) ), writeInfo, 0U, nullptr );
    return true;
}

void LightupCommonDescriptorSet::Destroy ( VkDevice device )
{
    _uniformBuffer.FreeResources ( device );

    if ( _commandPool != VK_NULL_HANDLE )
    {
        vkDestroyCommandPool ( device, _commandPool, nullptr );
        _commandPool = VK_NULL_HANDLE;
        AV_UNREGISTER_COMMAND_POOL ( "LightupCommonDescriptorSet::_commandPool" )
    }

    _layout.Destroy ( device );

    if ( _descriptorPool == VK_NULL_HANDLE )
        return;

    vkDestroyDescriptorPool ( device, _descriptorPool, nullptr );
    _descriptorPool = VK_NULL_HANDLE;
    AV_UNREGISTER_DESCRIPTOR_POOL ( "LightupCommonDescriptorSet::_descriptorPool" )
}

VkDescriptorSet LightupCommonDescriptorSet::GetSet () const
{
    return _set;
}

bool LightupCommonDescriptorSet::Update ( android_vulkan::Renderer &renderer,
    VkExtent2D const &resolution,
    GXMat4 const &cvvToView
)
{
    LightLightupBaseProgram::ViewData const viewData
    {
        ._toView = cvvToView,

        ._invResolutionFactor
        {
            2.0F / static_cast<float> ( resolution.width ),
            2.0F / static_cast<float> ( resolution.height )
        },

        ._padding0_0 {},
    };

    return _uniformBuffer.Update ( renderer, reinterpret_cast<uint8_t const*> ( &viewData ), sizeof ( viewData ) );
}

} // namespace pbr
