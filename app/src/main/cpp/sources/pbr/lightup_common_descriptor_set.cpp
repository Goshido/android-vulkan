#include <pbr/lightup_common_descriptor_set.h>
#include <vulkan_utils.h>
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

bool LightupCommonDescriptorSet::Init ( android_vulkan::Renderer &renderer )
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

    if ( _uniformBuffer.Init ( renderer, _commandPool, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT ) )
        return true;

    Destroy ( device );
    return false;
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

[[maybe_unused]] VkDescriptorSet LightupCommonDescriptorSet::GetSet () const
{
    return _set;
}

[[maybe_unused]] bool LightupCommonDescriptorSet::Update ( android_vulkan::Renderer &renderer,
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
