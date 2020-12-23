#include <rotating_mesh/game_lut.h>
#include <GXCommon/GXWarning.h>

GX_DISABLE_COMMON_WARNINGS

#include <array>
#include <cmath>
#include <thread>

GX_RESTORE_WARNING_STATE

#include <half_types.h>


namespace rotating_mesh {

constexpr static const char* FRAGMENT_SHADER = "shaders/blinn-phong-lut-ps.spv";

constexpr static const size_t SPECULAR_ANGLE_SAMPLES = 512U;
constexpr static const size_t SPECULAR_EXPONENT_SAMPLES = 150U;
constexpr static const size_t SPECULAR_GENERATOR_THREADS = 4U;

constexpr static const size_t TEXTURE_COMMAND_BUFFERS = 7U;

//----------------------------------------------------------------------------------------------------------------------

GameLUT::GameLUT ():
    Game ( FRAGMENT_SHADER ),
    _specularLUTSampler ( VK_NULL_HANDLE ),
    _specularLUTTexture {}
{
    // NOTHING
}

bool GameLUT::CreateDescriptorSet ( android_vulkan::Renderer &renderer )
{
    VkDevice device = renderer.GetDevice ();
    constexpr const size_t uniqueFeatureCount = 3U;
    constexpr const size_t featureCount = 7U;

    VkDescriptorPoolSize features[ uniqueFeatureCount ];
    InitDescriptorPoolSizeCommon ( features );

    features[ 1U ].descriptorCount = static_cast<uint32_t> ( MATERIAL_COUNT * 3U );
    features[ 2U ].descriptorCount = static_cast<uint32_t> ( MATERIAL_COUNT * 3U );

    VkDescriptorPoolCreateInfo poolInfo;
    poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.pNext = nullptr;
    poolInfo.flags = 0U;
    poolInfo.maxSets = static_cast<uint32_t> ( MATERIAL_COUNT );
    poolInfo.poolSizeCount = static_cast<uint32_t> ( std::size ( features ) );
    poolInfo.pPoolSizes = features;

    bool result = android_vulkan::Renderer::CheckVkResult (
        vkCreateDescriptorPool ( device, &poolInfo, nullptr, &_descriptorPool ),
        "GameLUT::CreateDescriptorSet",
        "Can't create descriptor pool"
    );

    if ( !result )
        return false;

    AV_REGISTER_DESCRIPTOR_POOL ( "Game::_descriptorPool" )

    VkDescriptorSetLayout layouts[ MATERIAL_COUNT ];
    VkDescriptorSet sets[ MATERIAL_COUNT ];

    for ( auto& item : layouts )
        item = _descriptorSetLayout;

    VkDescriptorSetAllocateInfo setAllocateInfo;
    setAllocateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    setAllocateInfo.pNext = nullptr;
    setAllocateInfo.descriptorPool = _descriptorPool;
    setAllocateInfo.pSetLayouts = layouts;
    setAllocateInfo.descriptorSetCount = poolInfo.maxSets;

    result = android_vulkan::Renderer::CheckVkResult ( vkAllocateDescriptorSets ( device, &setAllocateInfo, sets ),
        "GameLUT::CreateDescriptorSet",
        "Can't allocate descriptor set"
    );

    if ( !result )
        return false;

    constexpr const size_t writeSetCount = featureCount * MATERIAL_COUNT;

    VkWriteDescriptorSet writeSets[ writeSetCount ];
    VkDescriptorImageInfo diffuseInfo[ MATERIAL_COUNT ];
    VkDescriptorImageInfo normalInfo[ MATERIAL_COUNT ];
    VkDescriptorImageInfo specInfo[ MATERIAL_COUNT ];

    VkDescriptorBufferInfo bufferInfo;
    bufferInfo.buffer = _transformBuffer.GetBuffer ();
    bufferInfo.range = _transformBuffer.GetSize ();
    bufferInfo.offset = 0U;

    for ( size_t i = 0U; i < MATERIAL_COUNT; ++i )
    {
        Drawcall& drawcall = _drawcalls[ i ];
        drawcall._descriptorSet = sets[ i ];

        VkDescriptorImageInfo& diffuseImage = diffuseInfo[ i ];
        diffuseImage.sampler = drawcall._diffuseSampler;
        diffuseImage.imageView = drawcall._diffuse.GetImageView ();
        diffuseImage.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

        VkDescriptorImageInfo& normalImage = normalInfo[ i ];
        normalImage.sampler = drawcall._normalSampler;
        normalImage.imageView = drawcall._normal.GetImageView ();
        normalImage.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

        VkDescriptorImageInfo& specImage = specInfo[ i ];
        specImage.sampler = _specularLUTSampler;
        specImage.imageView = _specularLUTTexture.GetImageView ();
        specImage.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

        const size_t pivotIndex = i * featureCount;

        VkWriteDescriptorSet& ubWriteSet = writeSets[ pivotIndex ];
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

        VkWriteDescriptorSet& diffuseImageWriteSet = writeSets[ pivotIndex + 1U ];
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

        VkWriteDescriptorSet& diffuseSamplerWriteSet = writeSets[ pivotIndex + 2U ];
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

        VkWriteDescriptorSet& normalImageWriteSet = writeSets[ pivotIndex + 3U ];
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

        VkWriteDescriptorSet& normalSamplerWriteSet = writeSets[ pivotIndex + 4U ];
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

        VkWriteDescriptorSet& specImageWriteSet = writeSets[ pivotIndex + 5U ];
        specImageWriteSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        specImageWriteSet.pNext = nullptr;
        specImageWriteSet.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
        specImageWriteSet.dstSet = drawcall._descriptorSet;
        specImageWriteSet.dstBinding = 5U;
        specImageWriteSet.dstArrayElement = 0U;
        specImageWriteSet.descriptorCount = 1U;
        specImageWriteSet.pBufferInfo = nullptr;
        specImageWriteSet.pImageInfo = &specImage;
        specImageWriteSet.pTexelBufferView = nullptr;

        VkWriteDescriptorSet& specSamplerWriteSet = writeSets[ pivotIndex + 6U ];
        specSamplerWriteSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        specSamplerWriteSet.pNext = nullptr;
        specSamplerWriteSet.dstSet = drawcall._descriptorSet;
        specSamplerWriteSet.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLER;
        specSamplerWriteSet.dstBinding = 6U;
        specSamplerWriteSet.dstArrayElement = 0U;
        specSamplerWriteSet.descriptorCount = 1U;
        specSamplerWriteSet.pBufferInfo = nullptr;
        specSamplerWriteSet.pImageInfo = &specImage;
        specSamplerWriteSet.pTexelBufferView = nullptr;
    }

    vkUpdateDescriptorSets ( device, static_cast<uint32_t> ( writeSetCount ), writeSets, 0U, nullptr );
    return true;
}

bool GameLUT::CreatePipelineLayout ( android_vulkan::Renderer &renderer )
{
    VkDescriptorSetLayoutBinding bindings[ 7U ];
    InitDescriptorSetLayoutBindingCommon ( bindings );

    VkDescriptorSetLayoutBinding& specImageInfo = bindings[ 5U ];
    specImageInfo.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
    specImageInfo.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
    specImageInfo.descriptorCount = 1U;
    specImageInfo.binding = 5U;
    specImageInfo.pImmutableSamplers = nullptr;

    VkDescriptorSetLayoutBinding& specSamplerInfo = bindings[ 6U ];
    specSamplerInfo.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
    specSamplerInfo.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLER;
    specSamplerInfo.descriptorCount = 1U;
    specSamplerInfo.binding = 6U;
    specSamplerInfo.pImmutableSamplers = nullptr;

    VkDescriptorSetLayoutCreateInfo descriptorSetInfo;
    descriptorSetInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    descriptorSetInfo.pNext = nullptr;
    descriptorSetInfo.flags = 0U;
    descriptorSetInfo.bindingCount = static_cast<uint32_t> ( std::size ( bindings ) );
    descriptorSetInfo.pBindings = bindings;

    VkDevice device = renderer.GetDevice ();

    bool result = android_vulkan::Renderer::CheckVkResult (
        vkCreateDescriptorSetLayout ( device, &descriptorSetInfo, nullptr, &_descriptorSetLayout ),
        "Game::CreatePipelineLayout",
        "Can't create descriptor set layout"
    );

    if ( !result )
        return false;

    AV_REGISTER_DESCRIPTOR_SET_LAYOUT ( "Game::_descriptorSetLayout" )

    VkPipelineLayoutCreateInfo pipelineLayoutInfo;
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.pNext = nullptr;
    pipelineLayoutInfo.flags = 0U;
    pipelineLayoutInfo.pushConstantRangeCount = 0U;
    pipelineLayoutInfo.pPushConstantRanges = nullptr;
    pipelineLayoutInfo.setLayoutCount = 1U;
    pipelineLayoutInfo.pSetLayouts = &_descriptorSetLayout;

    result = android_vulkan::Renderer::CheckVkResult (
        vkCreatePipelineLayout ( device, &pipelineLayoutInfo, nullptr, &_pipelineLayout ),
        "Game::CreatePipelineLayout",
        "Can't create pipeline layout"
    );

    if ( !result )
        return false;

    AV_REGISTER_PIPELINE_LAYOUT ( "Game::_pipelineLayout" )
    return true;
}

bool GameLUT::LoadGPUContent ( android_vulkan::Renderer &renderer )
{
    constexpr const size_t commandBufferCount = TEXTURE_COMMAND_BUFFERS + MATERIAL_COUNT;
    VkCommandBuffer commandBuffers[ commandBufferCount ] = { VK_NULL_HANDLE };

    VkCommandBufferAllocateInfo allocateInfo;
    allocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocateInfo.pNext = nullptr;
    allocateInfo.commandPool = _commandPool;
    allocateInfo.commandBufferCount = static_cast<uint32_t> ( commandBufferCount );
    allocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;

    VkDevice device = renderer.GetDevice ();

    bool result = android_vulkan::Renderer::CheckVkResult (
        vkAllocateCommandBuffers ( renderer.GetDevice (), &allocateInfo, commandBuffers ),
        "GameLUT::LoadGPUContent",
        "Can't allocate command buffers"
    );

    if ( !result )
        return false;

    if ( !CreateCommonTextures ( renderer, commandBuffers ) )
        return false;

    if ( !CreateSpecularLUTTexture ( renderer, commandBuffers[ 6U ] ) )
        return false;

    if ( !CreateMeshes ( renderer, commandBuffers + TEXTURE_COMMAND_BUFFERS ) )
        return false;

    result = android_vulkan::Renderer::CheckVkResult ( vkQueueWaitIdle ( renderer.GetQueue () ),
        "GameLUT::LoadGPUContent",
        "Can't run upload commands"
    );

    if ( !result )
        return false;

    for ( auto& item : _drawcalls )
    {
        item._mesh.FreeTransferResources ( renderer );
        item._diffuse.FreeTransferResources ( renderer );
        item._normal.FreeTransferResources ( renderer );
    }

    _specularLUTTexture.FreeTransferResources ( renderer );

    vkFreeCommandBuffers ( device, _commandPool, allocateInfo.commandBufferCount, commandBuffers );
    return true;
}

bool GameLUT::CreateSamplers ( android_vulkan::Renderer &renderer )
{
    bool result = Game::CreateSamplers ( renderer );

    if ( !result )
        return false;

    VkSamplerCreateInfo samplerInfo;
    samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    samplerInfo.pNext = nullptr;
    samplerInfo.flags = 0U;
    samplerInfo.unnormalizedCoordinates = VK_FALSE;
    samplerInfo.borderColor = VK_BORDER_COLOR_FLOAT_TRANSPARENT_BLACK;
    samplerInfo.mipLodBias = 0.0F;
    samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_NEAREST;
    samplerInfo.minLod = 0.0F;
    samplerInfo.maxLod = 1.0F;
    samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    samplerInfo.magFilter = VK_FILTER_LINEAR;
    samplerInfo.minFilter = VK_FILTER_LINEAR;
    samplerInfo.anisotropyEnable = VK_FALSE;
    samplerInfo.maxAnisotropy = 1.0F;
    samplerInfo.compareEnable = VK_FALSE;
    samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;

    result = android_vulkan::Renderer::CheckVkResult (
        vkCreateSampler ( renderer.GetDevice (), &samplerInfo, nullptr, &_specularLUTSampler ),
        "GameLUT::CreateSamplers",
        "Can't create specular LUT sampler"
    );

    if ( !result )
        return false;

    AV_REGISTER_SAMPLER ( "GameLUT::_specularLUTSampler" )
    return true;
}

void GameLUT::DestroySamplers ( android_vulkan::Renderer &renderer )
{
    Game::DestroySamplers ( renderer );

    if ( _specularLUTSampler == VK_NULL_HANDLE )
        return;

    vkDestroySampler ( renderer.GetDevice (), _specularLUTSampler, nullptr );
    _specularLUTSampler = VK_NULL_HANDLE;
    AV_UNREGISTER_SAMPLER ( "GameLUT::_specularLUTSampler" )
}

void GameLUT::DestroyTextures ( android_vulkan::Renderer &renderer )
{
    DestroySpecularLUTTexture ( renderer );
    Game::DestroyTextures ( renderer );
}

bool GameLUT::CreateSpecularLUTTexture ( android_vulkan::Renderer &renderer, VkCommandBuffer commandBuffer )
{
    constexpr const size_t totalSamples = SPECULAR_ANGLE_SAMPLES * SPECULAR_EXPONENT_SAMPLES;

    std::vector<uint8_t> lutData ( totalSamples * sizeof ( android_vulkan::Half ) );
    auto* samples = reinterpret_cast<android_vulkan::Half*> ( lutData.data () );

    auto jobThread = [ samples ] ( size_t startIndex, int shininess ) {
        do
        {
            constexpr const auto nextOffset = SPECULAR_GENERATOR_THREADS * SPECULAR_ANGLE_SAMPLES;
            constexpr const auto convert = 1.0F / static_cast<const float> ( SPECULAR_ANGLE_SAMPLES );

            for ( size_t i = 0U; i < SPECULAR_ANGLE_SAMPLES; ++i )
                samples[ startIndex + i ] = std::pow ( static_cast<float> ( i ) * convert, shininess );

            shininess += static_cast<int> ( SPECULAR_GENERATOR_THREADS );
            startIndex += nextOffset;
        }
        while ( startIndex < totalSamples );
    };

    std::array<std::thread, SPECULAR_GENERATOR_THREADS> jobPool;

    for ( size_t i = 0U; i < SPECULAR_GENERATOR_THREADS; ++i )
        jobPool[ i ] = std::thread ( jobThread, i * SPECULAR_ANGLE_SAMPLES, static_cast<int> ( i ) );

    for ( size_t i = 0U; i < SPECULAR_GENERATOR_THREADS; ++i )
        jobPool[ i ].join ();

    return _specularLUTTexture.UploadData ( lutData.data (),
        lutData.size (),

        VkExtent2D {
            .width = static_cast<uint32_t> ( SPECULAR_ANGLE_SAMPLES ),
            .height = static_cast<uint32_t> ( SPECULAR_EXPONENT_SAMPLES )
        },

        VK_FORMAT_R16_SFLOAT,
        false,
        renderer,
        commandBuffer
    );
}

void GameLUT::DestroySpecularLUTTexture ( android_vulkan::Renderer &renderer )
{
    _specularLUTTexture.FreeResources ( renderer );
}

} // namespace rotating_mesh
