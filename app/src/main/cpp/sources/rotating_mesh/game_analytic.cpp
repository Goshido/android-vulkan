#include <rotating_mesh/game_analytic.hpp>


namespace rotating_mesh {

constexpr static char const* FRAGMENT_SHADER = "shaders/blinn_phong_analytic.ps.spv";
constexpr static size_t TEXTURE_COMMAND_BUFFERS = 6U;

//----------------------------------------------------------------------------------------------------------------------

GameAnalytic::GameAnalytic () noexcept:
    Game ( FRAGMENT_SHADER )
{
    // NOTHING
}

bool GameAnalytic::CreateDescriptorSet ( android_vulkan::Renderer &renderer ) noexcept
{
    VkDevice device = renderer.GetDevice ();
    constexpr size_t uniqueFeatureCount = 3U;
    constexpr size_t featureCount = 5U;

    VkDescriptorPoolSize features[ uniqueFeatureCount ];
    InitDescriptorPoolSizeCommon ( features );

    VkDescriptorPoolCreateInfo const poolInfo
    {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0U,
        .maxSets = static_cast<uint32_t> ( MATERIAL_COUNT ),
        .poolSizeCount = static_cast<uint32_t> ( std::size ( features ) ),
        .pPoolSizes = features
    };

    bool result = android_vulkan::Renderer::CheckVkResult (
        vkCreateDescriptorPool ( device, &poolInfo, nullptr, &_descriptorPool ),
        "GameAnalytic::CreateDescriptorSet",
        "Can't create descriptor pool"
    );

    if ( !result )
        return false;

    AV_REGISTER_DESCRIPTOR_POOL ( "Game::_descriptorPool" )

    VkDescriptorSetLayout layouts[ MATERIAL_COUNT ];
    VkDescriptorSet sets[ MATERIAL_COUNT ];

    for ( auto &item : layouts )
        item = _descriptorSetLayout;

    VkDescriptorSetAllocateInfo const setAllocateInfo
    {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
        .pNext = nullptr,
        .descriptorPool = _descriptorPool,
        .descriptorSetCount = poolInfo.maxSets,
        .pSetLayouts = layouts
    };

    result = android_vulkan::Renderer::CheckVkResult ( vkAllocateDescriptorSets ( device, &setAllocateInfo, sets ),
        "GameAnalytic::CreateDescriptorSet",
        "Can't allocate descriptor set"
    );

    if ( !result )
        return false;

    constexpr size_t writeSetCount = featureCount * MATERIAL_COUNT;
    VkWriteDescriptorSet writeSets[ writeSetCount ];
    VkDescriptorImageInfo diffuseInfo[ MATERIAL_COUNT ];
    VkDescriptorImageInfo normalInfo[ MATERIAL_COUNT ];

    VkDescriptorBufferInfo bufferInfo;
    bufferInfo.buffer = _transformBuffer.GetBuffer ();
    bufferInfo.range = _transformBuffer.GetSize ();
    bufferInfo.offset = 0U;

    for ( size_t i = 0U; i < MATERIAL_COUNT; ++i )
    {
        Drawcall &drawcall = _drawcalls[ i ];
        drawcall._descriptorSet = sets[ i ];

        VkDescriptorImageInfo &diffuseImage = diffuseInfo[ i ];
        diffuseImage.sampler = drawcall._diffuseSampler;
        diffuseImage.imageView = drawcall._diffuse.GetImageView ();
        diffuseImage.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

        VkDescriptorImageInfo &normalImage = normalInfo[ i ];
        normalImage.sampler = drawcall._normalSampler;
        normalImage.imageView = drawcall._normal.GetImageView ();
        normalImage.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

        size_t const pivotIndex = i * featureCount;

        VkWriteDescriptorSet &ubWriteSet = writeSets[ pivotIndex ];
        ubWriteSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        ubWriteSet.pNext = nullptr;
        ubWriteSet.dstSet = drawcall._descriptorSet;
        ubWriteSet.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        ubWriteSet.dstBinding = 0U;
        ubWriteSet.dstArrayElement = 0U;
        ubWriteSet.descriptorCount = 1U;
        ubWriteSet.pBufferInfo = &bufferInfo;
        ubWriteSet.pImageInfo = nullptr;
        ubWriteSet.pTexelBufferView = nullptr;

        VkWriteDescriptorSet &diffuseImageWriteSet = writeSets[ pivotIndex + 1U ];
        diffuseImageWriteSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        diffuseImageWriteSet.pNext = nullptr;
        diffuseImageWriteSet.dstSet = drawcall._descriptorSet;
        diffuseImageWriteSet.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
        diffuseImageWriteSet.dstBinding = 1U;
        diffuseImageWriteSet.dstArrayElement = 0U;
        diffuseImageWriteSet.descriptorCount = 1U;
        diffuseImageWriteSet.pBufferInfo = nullptr;
        diffuseImageWriteSet.pImageInfo = &diffuseImage;
        diffuseImageWriteSet.pTexelBufferView = nullptr;

        VkWriteDescriptorSet &diffuseSamplerWriteSet = writeSets[ pivotIndex + 2U ];
        diffuseSamplerWriteSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        diffuseSamplerWriteSet.pNext = nullptr;
        diffuseSamplerWriteSet.dstSet = drawcall._descriptorSet;
        diffuseSamplerWriteSet.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLER;
        diffuseSamplerWriteSet.dstBinding = 2U;
        diffuseSamplerWriteSet.dstArrayElement = 0U;
        diffuseSamplerWriteSet.descriptorCount = 1U;
        diffuseSamplerWriteSet.pBufferInfo = nullptr;
        diffuseSamplerWriteSet.pImageInfo = &diffuseImage;
        diffuseSamplerWriteSet.pTexelBufferView = nullptr;

        VkWriteDescriptorSet &normalImageWriteSet = writeSets[ pivotIndex + 3U ];
        normalImageWriteSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        normalImageWriteSet.pNext = nullptr;
        normalImageWriteSet.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
        normalImageWriteSet.dstSet = drawcall._descriptorSet;
        normalImageWriteSet.dstBinding = 3U;
        normalImageWriteSet.dstArrayElement = 0U;
        normalImageWriteSet.descriptorCount = 1U;
        normalImageWriteSet.pBufferInfo = nullptr;
        normalImageWriteSet.pImageInfo = &normalImage;
        normalImageWriteSet.pTexelBufferView = nullptr;

        VkWriteDescriptorSet &normalSamplerWriteSet = writeSets[ pivotIndex + 4U ];
        normalSamplerWriteSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        normalSamplerWriteSet.pNext = nullptr;
        normalSamplerWriteSet.dstSet = drawcall._descriptorSet;
        normalSamplerWriteSet.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLER;
        normalSamplerWriteSet.dstBinding = 4U;
        normalSamplerWriteSet.dstArrayElement = 0U;
        normalSamplerWriteSet.descriptorCount = 1U;
        normalSamplerWriteSet.pBufferInfo = nullptr;
        normalSamplerWriteSet.pImageInfo = &normalImage;
        normalSamplerWriteSet.pTexelBufferView = nullptr;
    }

    vkUpdateDescriptorSets ( device, static_cast<uint32_t> ( writeSetCount ), writeSets, 0U, nullptr );
    return true;
}

bool GameAnalytic::CreatePipelineLayout ( android_vulkan::Renderer &renderer ) noexcept
{
    VkDescriptorSetLayoutBinding bindings[ 5U ];
    InitDescriptorSetLayoutBindingCommon ( bindings );

    VkDescriptorSetLayoutCreateInfo const descriptorSetInfo
    {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0U,
        .bindingCount = static_cast<uint32_t> ( std::size ( bindings ) ),
        .pBindings = bindings
    };

    VkDevice device = renderer.GetDevice ();

    bool result = android_vulkan::Renderer::CheckVkResult (
        vkCreateDescriptorSetLayout ( device, &descriptorSetInfo, nullptr, &_descriptorSetLayout ),
        "GameAnalytic::CreatePipelineLayout",
        "Can't create descriptor set layout"
    );

    if ( !result )
        return false;

    AV_REGISTER_DESCRIPTOR_SET_LAYOUT ( "Game::_descriptorSetLayout" )

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
        "GameAnalytic::CreatePipelineLayout",
        "Can't create pipeline layout"
    );

    if ( !result )
        return false;

    AV_REGISTER_PIPELINE_LAYOUT ( "Game::_pipelineLayout" )
    return true;
}

bool GameAnalytic::LoadGPUContent ( android_vulkan::Renderer &renderer ) noexcept
{
    constexpr size_t commandBufferCount = TEXTURE_COMMAND_BUFFERS + MATERIAL_COUNT;
    VkCommandBuffer commandBuffers[ commandBufferCount ] = { VK_NULL_HANDLE };

    VkCommandBufferAllocateInfo const allocateInfo
    {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
        .pNext = nullptr,
        .commandPool = _commandPool,
        .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
        .commandBufferCount = static_cast<uint32_t> ( commandBufferCount )
    };

    bool result = android_vulkan::Renderer::CheckVkResult (
        vkAllocateCommandBuffers ( renderer.GetDevice (), &allocateInfo, commandBuffers ),
        "GameLUT::LoadGPUContent",
        "Can't allocate command buffers"
    );

    if ( !result || !CreateCommonTextures ( renderer, commandBuffers ) )
        return false;

    if ( !CreateMeshes ( renderer, commandBuffers + TEXTURE_COMMAND_BUFFERS ) )
        return false;

    result = android_vulkan::Renderer::CheckVkResult ( vkQueueWaitIdle ( renderer.GetQueue () ),
        "GameLUT::LoadGPUContent",
        "Can't run upload commands"
    );

    if ( !result )
        return false;

    for ( auto &item : _drawcalls )
    {
        item._mesh.FreeTransferResources ( renderer );
        item._diffuse.FreeTransferResources ( renderer );
        item._normal.FreeTransferResources ( renderer );
    }

    vkFreeCommandBuffers ( renderer.GetDevice (), _commandPool, allocateInfo.commandBufferCount, commandBuffers );
    return true;
}

} // namespace rotating_mesh
