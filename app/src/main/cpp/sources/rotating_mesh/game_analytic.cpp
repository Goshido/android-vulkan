#include <precompiled_headers.hpp>
#include <rotating_mesh/bindings.inc>
#include <rotating_mesh/game_analytic.hpp>


namespace rotating_mesh {

namespace {

constexpr char const* FRAGMENT_SHADER = "shaders/blinn_phong_analytic.ps.spv";
constexpr size_t TEXTURE_COMMAND_BUFFERS = 6U;

} // end of anonymous namespace

//----------------------------------------------------------------------------------------------------------------------

GameAnalytic::GameAnalytic () noexcept:
    Game ( FRAGMENT_SHADER )
{
    // NOTHING
}

bool GameAnalytic::CreateMaterialDescriptorSetLayout ( android_vulkan::Renderer &renderer ) noexcept
{
    constexpr static VkDescriptorSetLayoutBinding const materialBindings[]
    {
        {
            .binding = BIND_DIFFUSE_TEXTURE,
            .descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,
            .descriptorCount = 1U,
            .stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT,
            .pImmutableSamplers = nullptr
        },
        {
            .binding = BIND_NORMAL_TEXTURE,
            .descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,
            .descriptorCount = 1U,
            .stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT,
            .pImmutableSamplers = nullptr
        }
    };

    constexpr VkDescriptorSetLayoutCreateInfo dsInfo
    {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0U,
        .bindingCount = static_cast<uint32_t> ( std::size ( materialBindings ) ),
        .pBindings = materialBindings
    };

    VkDevice device = renderer.GetDevice ();

    bool result = android_vulkan::Renderer::CheckVkResult (
        vkCreateDescriptorSetLayout ( device, &dsInfo, nullptr, &_materialDSLayout ),
        "GameLUT::CreateMaterialDescriptorSetLayout",
        "Can't create material descriptor set layout"
    );

    if ( !result ) [[unlikely]]
        return false;

    AV_SET_VULKAN_OBJECT_NAME ( device,
        _materialDSLayout,
        VK_OBJECT_TYPE_DESCRIPTOR_SET_LAYOUT,
        "Game::_materialDSLayout"
    )

    return true;
}

bool GameAnalytic::CreateDescriptorSet ( android_vulkan::Renderer &renderer ) noexcept
{
    size_t const materialEntries = MATERIAL_COUNT * 2U;

    VkDescriptorPoolSize const poolSizes[] = {
        {
            .type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
            .descriptorCount = DUAL_COMMAND_BUFFER
        },
        {
            .type = VK_DESCRIPTOR_TYPE_SAMPLER,
            .descriptorCount = 1U,
        },
        {
            .type = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,
            .descriptorCount = static_cast<uint32_t> ( materialEntries )
        }
    };

    VkDescriptorPoolCreateInfo const poolInfo
    {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0U,
        .maxSets = static_cast<uint32_t> ( MATERIAL_COUNT ) + DUAL_COMMAND_BUFFER + 1U,
        .poolSizeCount = static_cast<uint32_t> ( std::size ( poolSizes ) ),
        .pPoolSizes = poolSizes
    };

    VkDevice device = renderer.GetDevice ();

    bool result = android_vulkan::Renderer::CheckVkResult (
        vkCreateDescriptorPool ( device, &poolInfo, nullptr, &_descriptorPool ),
        "GameAnalytic::CreateDescriptorSet",
        "Can't create descriptor pool"
    );

    if ( !result ) [[unlikely]]
        return false;

    AV_SET_VULKAN_OBJECT_NAME ( device, _descriptorPool, VK_OBJECT_TYPE_DESCRIPTOR_POOL, "Game::_descriptorPool" )

    VkDescriptorSetAllocateInfo setAllocateInfo
    {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
        .pNext = nullptr,
        .descriptorPool = _descriptorPool,
        .descriptorSetCount = 1U,
        .pSetLayouts = &_fixedDSLayout
    };

    result = android_vulkan::Renderer::CheckVkResult ( vkAllocateDescriptorSets ( device, &setAllocateInfo, &_fixedDS ),
        "GameAnalytic::CreateDescriptorSet",
        "Can't allocate descriptor set"
    );

    if ( !result ) [[unlikely]]
        return false;

    AV_SET_VULKAN_OBJECT_NAME ( device, _fixedDS, VK_OBJECT_TYPE_DESCRIPTOR_SET, "Fixed" )

    VkDescriptorImageInfo const samplerInfo
    {
        .sampler = _sampler,
        .imageView = VK_NULL_HANDLE,
        .imageLayout = VK_IMAGE_LAYOUT_UNDEFINED
    };

    VkWriteDescriptorSet const fixedWrite
    {
        .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
        .pNext = nullptr,
        .dstSet = _fixedDS,
        .dstBinding = BIND_SAMPLER,
        .dstArrayElement = 0U,
        .descriptorCount = 1U,
        .descriptorType = VK_DESCRIPTOR_TYPE_SAMPLER,
        .pImageInfo = &samplerInfo,
        .pBufferInfo = nullptr,
        .pTexelBufferView = nullptr
    };

    vkUpdateDescriptorSets ( device, 1U, &fixedWrite, 0U, nullptr );

    std::vector<VkDescriptorSetLayout> materialLayouts ( MATERIAL_COUNT, _materialDSLayout );
    setAllocateInfo.descriptorSetCount = static_cast<uint32_t> ( MATERIAL_COUNT );
    setAllocateInfo.pSetLayouts = materialLayouts.data ();

    result = android_vulkan::Renderer::CheckVkResult (
        vkAllocateDescriptorSets ( device, &setAllocateInfo, _materialDS ),
        "GameAnalytic::CreateDescriptorSet",
        "Can't allocate material descriptor sets"
    );

    if ( !result ) [[unlikely]]
        return false;

#if defined ( ANDROID_VULKAN_ENABLE_VULKAN_VALIDATION_LAYERS ) ||       \
    defined ( ANDROID_VULKAN_ENABLE_RENDER_DOC_INTEGRATION )

    for ( size_t i = 0U; i < MATERIAL_COUNT; ++i )
        AV_SET_VULKAN_OBJECT_NAME ( device, _materialDS[ i ], VK_OBJECT_TYPE_DESCRIPTOR_SET, "Material #%zu", i )

#endif // ANDROID_VULKAN_ENABLE_VULKAN_VALIDATION_LAYERS || ANDROID_VULKAN_ENABLE_RENDER_DOC_INTEGRATION

    constexpr VkDescriptorImageInfo templateImageInfo
    {
        .sampler = VK_NULL_HANDLE,
        .imageView = VK_NULL_HANDLE,
        .imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
    };

    std::vector<VkDescriptorImageInfo> imageInfo ( materialEntries, templateImageInfo );

    constexpr VkWriteDescriptorSet writeTemplate
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

    std::vector<VkWriteDescriptorSet> writes ( materialEntries, writeTemplate );

    for ( size_t i = 0U; i < MATERIAL_COUNT; ++i )
    {
        Drawcall &drawcall = _drawcalls[ i ];

        VkDescriptorSet ds = _materialDS[ i ];
        drawcall._descriptorSet = ds;

        size_t idx = i * 2U;
        VkDescriptorImageInfo &diffuseImage = imageInfo[ idx ];
        diffuseImage.imageView = drawcall._diffuse.GetImageView ();

        VkWriteDescriptorSet &materialDiffuseWrite = writes[ idx ];
        materialDiffuseWrite.dstSet = ds;
        materialDiffuseWrite.dstBinding = BIND_DIFFUSE_TEXTURE;
        materialDiffuseWrite.pImageInfo = &diffuseImage;

        VkDescriptorImageInfo &normalImage = imageInfo[ ++idx ];
        normalImage.imageView = drawcall._normal.GetImageView ();

        VkWriteDescriptorSet &materialNormalWrite = writes[ idx ];
        materialNormalWrite.dstSet = ds;
        materialNormalWrite.dstBinding = BIND_NORMAL_TEXTURE;
        materialNormalWrite.pImageInfo = &normalImage;
    }

    vkUpdateDescriptorSets ( device, static_cast<uint32_t> ( materialEntries ), writes.data (), 0U, nullptr );

    std::vector<VkDescriptorSetLayout> onceLayouts ( DUAL_COMMAND_BUFFER, _onceDSLayout );
    setAllocateInfo.descriptorSetCount = static_cast<uint32_t> ( DUAL_COMMAND_BUFFER );
    setAllocateInfo.pSetLayouts = onceLayouts.data ();

    result =  android_vulkan::Renderer::CheckVkResult (
        vkAllocateDescriptorSets ( device, &setAllocateInfo, _onceDS ),
        "GameAnalytic::CreateDescriptorSet",
        "Can't allocate once descriptor sets"
    );

    if ( !result ) [[unlikely]]
        return false;

#if defined ( ANDROID_VULKAN_ENABLE_VULKAN_VALIDATION_LAYERS ) ||       \
    defined ( ANDROID_VULKAN_ENABLE_RENDER_DOC_INTEGRATION )

    for ( size_t i = 0U; i < DUAL_COMMAND_BUFFER; ++i )
        AV_SET_VULKAN_OBJECT_NAME ( device, _onceDS[ i ], VK_OBJECT_TYPE_DESCRIPTOR_SET, "Once [FIF #%zu]", i )

#endif // ANDROID_VULKAN_ENABLE_VULKAN_VALIDATION_LAYERS || ANDROID_VULKAN_ENABLE_RENDER_DOC_INTEGRATION

    return true;
}

bool GameAnalytic::LoadGPUContent ( android_vulkan::Renderer &renderer, VkCommandPool commandPool ) noexcept
{
    constexpr size_t commandBufferCount = TEXTURE_COMMAND_BUFFERS + MATERIAL_COUNT;
    VkCommandBuffer commandBuffers[ commandBufferCount ] = { VK_NULL_HANDLE };

    VkCommandBufferAllocateInfo const allocateInfo
    {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
        .pNext = nullptr,
        .commandPool = commandPool,
        .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
        .commandBufferCount = static_cast<uint32_t> ( commandBufferCount )
    };

    bool result = android_vulkan::Renderer::CheckVkResult (
        vkAllocateCommandBuffers ( renderer.GetDevice (), &allocateInfo, commandBuffers ),
        "GameLUT::LoadGPUContent",
        "Can't allocate command buffers"
    );

    if ( !result || !CreateCommonTextures ( renderer, commandBuffers ) ) [[unlikely]]
        return false;

    if ( !CreateMeshes ( renderer, commandBuffers + TEXTURE_COMMAND_BUFFERS ) ) [[unlikely]]
        return false;

    result = android_vulkan::Renderer::CheckVkResult ( vkQueueWaitIdle ( renderer.GetQueue () ),
        "GameLUT::LoadGPUContent",
        "Can't run upload commands"
    );

    if ( !result ) [[unlikely]]
        return false;

    for ( auto &item : _drawcalls )
    {
        item._mesh.FreeTransferResources ( renderer );
        item._diffuse.FreeTransferResources ( renderer );
        item._normal.FreeTransferResources ( renderer );
    }

    vkFreeCommandBuffers ( renderer.GetDevice (), commandPool, allocateInfo.commandBufferCount, commandBuffers );
    return true;
}

} // namespace rotating_mesh
