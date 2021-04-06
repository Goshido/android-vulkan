#include <pbr/reflection_local_pass.h>


namespace pbr {

constexpr static size_t const BUFFERS_PER_CALL = 2U;
constexpr static size_t const DESCRIPTORS_PER_CALL = 2U;
constexpr static size_t const IMAGES_PER_CALL = 1U;
constexpr static size_t const WRITES_PER_CALL = BUFFERS_PER_CALL + IMAGES_PER_CALL;

ReflectionLocalPass::Call::Call ( GXVec3 const &location, TextureCubeRef &prefilter, float size ):
    _location ( location ),
    _prefilter ( prefilter ),
    _size ( size )
{
    // NOTHING
}

ReflectionLocalPass::ReflectionLocalPass () noexcept:
    _bufferInfo {},
    _calls {},
    _commandPool ( VK_NULL_HANDLE ),
    _descriptorPool ( VK_NULL_HANDLE ),
    _descriptorSets {},
    _imageInfo {},
    _lightPassNotifier ( nullptr ),
    _lightVolumeUniforms ( eUniformPoolSize::Tiny_4M ),
    _program {},
    _reflectionUniforms ( eUniformPoolSize::Tiny_4M ),
    _transfer ( VK_NULL_HANDLE ),
    _transferFence ( VK_NULL_HANDLE ),
    _transferSubmit {},
    _writeSets {}
{
    // NOTHING
}

void ReflectionLocalPass::Append ( TextureCubeRef &prefilter, GXVec3 const &location, float size )
{
    _calls.emplace_back ( Call ( location, prefilter, size ) );
}

bool ReflectionLocalPass::Execute ( android_vulkan::Renderer &renderer,
    LightVolume &lightVolume,
    android_vulkan::MeshGeometry &unitCube,
    VkCommandBuffer commandBuffer
)
{
    VkDevice device = renderer.GetDevice ();

    bool result = android_vulkan::Renderer::CheckVkResult (
        vkWaitForFences ( device, 1U, &_transferFence, VK_TRUE, UINT64_MAX ),
        "ReflectionLocalPass::Execute",
        "Can't wait for fence"
    );

    if ( !result )
        return false;

    result = android_vulkan::Renderer::CheckVkResult ( vkResetFences ( device, 1U, &_transferFence ),
        "ReflectionLocalPass::Execute",
        "Can't reset fence"
    );

    if ( !result )
        return false;

    size_t const callCount = _calls.size ();

    vkUpdateDescriptorSets ( renderer.GetDevice (),
        static_cast<uint32_t> ( callCount * WRITES_PER_CALL ),
        _writeSets.data (),
        0U,
        nullptr
    );

    constexpr VkDeviceSize const offset = 0U;

    uint32_t const vertexCount = unitCube.GetVertexCount ();
    vkCmdBindVertexBuffers ( commandBuffer, 0U, 1U, &unitCube.GetVertexBuffer (), &offset );
    vkCmdBindIndexBuffer ( commandBuffer, unitCube.GetIndexBuffer (), 0U, VK_INDEX_TYPE_UINT32 );

    size_t descriptorIndex = 0U;

    for ( size_t i = 0U; i < callCount; ++i )
    {
        _lightPassNotifier->OnBeginLightWithVolume ( commandBuffer );

        lightVolume.Execute ( vertexCount, _descriptorSets[ descriptorIndex ], commandBuffer );
        vkCmdNextSubpass ( commandBuffer, VK_SUBPASS_CONTENTS_INLINE );

        _program.Bind ( commandBuffer );
        _program.SetLightData ( commandBuffer, _descriptorSets[ descriptorIndex + 1U ] );
        vkCmdDrawIndexed ( commandBuffer, vertexCount, 1U, 0U, 0, 0U );

        descriptorIndex += 2U;
        _lightPassNotifier->OnEndLightWithVolume ( commandBuffer );
    }

    return true;
}

size_t ReflectionLocalPass::GetReflectionLocalCount () const
{
    return _calls.size ();
}

bool ReflectionLocalPass::Init ( android_vulkan::Renderer &renderer,
    LightPassNotifier &notifier,
    VkCommandPool commandPool,
    VkRenderPass renderPass,
    uint32_t subpass,
    VkExtent2D const &viewport
)
{
    _lightPassNotifier = &notifier;
    VkDevice device = renderer.GetDevice ();

    if ( !_program.Init ( renderer, renderPass, subpass, viewport ) )
    {
        Destroy ( device );
        return false;
    }

    if ( !_lightVolumeUniforms.Init ( renderer, sizeof ( ReflectionLocalProgram::VolumeData ) ) )
    {
        Destroy ( device );
        return false;
    }

    if ( !_reflectionUniforms.Init ( renderer, sizeof ( ReflectionLocalProgram::LightData ) ) )
    {
        Destroy ( device );
        return false;
    }

    _commandPool = commandPool;

    VkCommandBufferAllocateInfo const allocateInfo
    {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
        .pNext = nullptr,
        .commandPool = _commandPool,
        .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
        .commandBufferCount = 1U
    };

    bool result = android_vulkan::Renderer::CheckVkResult (
        vkAllocateCommandBuffers ( device, &allocateInfo, &_transfer ),
        "ReflectionLocalPass::Init",
        "Can't allocate command buffers"
    );

    if ( !result )
    {
        Destroy ( device );
        return false;
    }

    constexpr VkFenceCreateInfo const fenceInfo
    {
        .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0U
    };

    result = android_vulkan::Renderer::CheckVkResult ( vkCreateFence ( device, &fenceInfo, nullptr, &_transferFence ),
        "ReflectionLocalPass::Init",
        "Can't create fence"
    );

    if ( !result )
    {
        Destroy ( device );
        return false;
    }

    AV_REGISTER_FENCE ( "ReflectionLocalPass::_transferFence" )

    _transferSubmit =
    {
        .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
        .pNext = nullptr,
        .waitSemaphoreCount = 0U,
        .pWaitSemaphores = nullptr,
        .pWaitDstStageMask = nullptr,
        .commandBufferCount = 1U,
        .pCommandBuffers = &_transfer,
        .signalSemaphoreCount = 0U,
        .pSignalSemaphores = nullptr
    };

    return true;
}

void ReflectionLocalPass::Destroy ( VkDevice device )
{
    if ( _transferFence != VK_NULL_HANDLE )
    {
        vkDestroyFence ( device, _transferFence, nullptr );
        _transferFence = VK_NULL_HANDLE;
        AV_UNREGISTER_FENCE ( "ReflectionLocalPass::_transferFence" )
    }

    vkFreeCommandBuffers ( device, _commandPool, 1U, &_transfer );
    _transfer = VK_NULL_HANDLE;
    _transferSubmit = {};

    _commandPool = VK_NULL_HANDLE;

    _reflectionUniforms.Destroy ( device );
    _lightVolumeUniforms.Destroy ( device );

    _program.Destroy ( device );
    DestroyDescriptorPool ( device );

    _bufferInfo.clear ();
    _bufferInfo.shrink_to_fit ();

    _calls.clear ();
    _calls.shrink_to_fit ();

    _descriptorSets.clear ();
    _descriptorSets.shrink_to_fit ();

    _imageInfo.clear ();
    _imageInfo.shrink_to_fit ();

    _writeSets.clear ();
    _writeSets.shrink_to_fit ();

    _lightPassNotifier = nullptr;
}

void ReflectionLocalPass::Reset ()
{
    _calls.clear ();
}

bool ReflectionLocalPass::UploadGPUData ( android_vulkan::Renderer &renderer,
    GXMat4 const &view,
    GXMat4 const &viewProjection
)
{
    size_t const callCount = _calls.size ();

    if ( !callCount )
        return true;

    _reflectionUniforms.Reset ();
    _lightVolumeUniforms.Reset ();

    if ( !AllocateDescriptorSets ( renderer, _calls.size () ) )
        return false;

    constexpr VkCommandBufferBeginInfo const beginInfo
    {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
        .pNext = nullptr,
        .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
        .pInheritanceInfo = nullptr
    };

    bool result = android_vulkan::Renderer::CheckVkResult ( vkBeginCommandBuffer ( _transfer, &beginInfo ),
        "ReflectionLocalPass::UploadGPUData",
        "Can't begin command buffer"
    );

    if ( !result )
        return false;

    ReflectionLocalProgram::LightData lightData;

    ReflectionLocalProgram::VolumeData volumeData;
    GXMat4 alpha;
    alpha.Identity ();

    size_t bufferIndex = 0U;
    size_t imageIndex = 0U;

    for ( auto const& call : _calls )
    {
        alpha._data[ 0U ] = call._size;
        alpha._data[ 5U ] = call._size;
        alpha._data[ 10U ] = call._size;
        alpha.SetW ( call._location );
        volumeData._transform.Multiply ( alpha, viewProjection );

        VkDescriptorBufferInfo& lightVolumeBuffer = _bufferInfo[ bufferIndex++ ];
        lightVolumeBuffer.range = static_cast<VkDeviceSize> ( sizeof ( volumeData ) );

        lightVolumeBuffer.buffer = _lightVolumeUniforms.Acquire ( renderer,
            _transfer,
            &volumeData,
            VK_PIPELINE_STAGE_VERTEX_SHADER_BIT
        );

        _imageInfo[ imageIndex++ ].imageView = call._prefilter->GetImageView ();

        lightData._invSize = 2.0F / call._size;
        view.MultiplyAsPoint ( lightData._locationView, call._location );

        VkDescriptorBufferInfo& lightDataBuffer = _bufferInfo[ bufferIndex++ ];
        lightDataBuffer.range = static_cast<VkDeviceSize> ( sizeof ( lightData ) );

        lightDataBuffer.buffer = _reflectionUniforms.Acquire ( renderer,
            _transfer,
            &lightData,
            VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT
        );
    }

    result = android_vulkan::Renderer::CheckVkResult ( vkEndCommandBuffer ( _transfer ),
        "ReflectionLocalPass::UploadGPUData",
        "Can't end command buffer"
    );

    if ( !result )
        return false;

    return android_vulkan::Renderer::CheckVkResult (
        vkQueueSubmit ( renderer.GetQueue (), 1U, &_transferSubmit, _transferFence ),
        "ReflectionLocalPass::UploadGPUData",
        "Can't submit command buffer"
    );
}

bool ReflectionLocalPass::AllocateDescriptorSets ( android_vulkan::Renderer &renderer, size_t neededCalls )
{
    size_t const neededDescriptors = neededCalls * DESCRIPTORS_PER_CALL;

    if ( neededDescriptors <= _descriptorSets.size () )
        return true;

    VkDevice device = renderer.GetDevice ();
    DestroyDescriptorPool ( device );

    size_t const bufferCount = BUFFERS_PER_CALL * neededCalls;
    size_t const imageCount = IMAGES_PER_CALL * neededCalls;

    VkDescriptorPoolSize const poolSizes[] =
    {
        {
            .type = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,
            .descriptorCount = static_cast<uint32_t> ( imageCount )
        },
        {
            .type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
            .descriptorCount = static_cast<uint32_t> ( bufferCount )
        }
    };

    VkDescriptorPoolCreateInfo const poolInfo
    {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0U,
        .maxSets = static_cast<uint32_t> ( neededDescriptors ),
        .poolSizeCount = static_cast<uint32_t> ( std::size ( poolSizes ) ),
        .pPoolSizes = poolSizes
    };

    bool result = android_vulkan::Renderer::CheckVkResult (
        vkCreateDescriptorPool ( device, &poolInfo, nullptr, &_descriptorPool ),
        "ReflectionLocalPass::AllocateDescriptorSets",
        "Can't create descriptor pool"
    );

    if ( !result )
        return false;

    AV_REGISTER_DESCRIPTOR_POOL ( "ReflectionLocalPass::_descriptorPool" )

    LightVolumeDescriptorSetLayout const lightVolumeLayout;
    VkDescriptorSetLayout lightVolumeNative = lightVolumeLayout.GetLayout ();

    ReflectionLocalDescriptorSetLayout const reflectionLayout;
    VkDescriptorSetLayout reflectionNative = reflectionLayout.GetLayout ();

    std::vector<VkDescriptorSetLayout> layouts ( neededDescriptors );

    for ( size_t i = 0U; i < neededDescriptors; i += DESCRIPTORS_PER_CALL )
    {
        layouts[ i ] = lightVolumeNative;
        layouts[ i + 1U ] = reflectionNative;
    }

    VkDescriptorSetAllocateInfo const allocateInfo
    {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
        .pNext = nullptr,
        .descriptorPool = _descriptorPool,
        .descriptorSetCount = poolInfo.maxSets,
        .pSetLayouts = layouts.data ()
    };

    _descriptorSets.resize ( neededDescriptors );

    result = android_vulkan::Renderer::CheckVkResult (
        vkAllocateDescriptorSets ( device, &allocateInfo, _descriptorSets.data () ),
        "ReflectionLocalPass::AllocateDescriptorSets",
        "Can't allocate descriptor sets"
    );

    if ( !result )
        return false;

    // Initialize all immutable constant fields.

    constexpr VkDescriptorBufferInfo const bufferInfo
    {
        .buffer = VK_NULL_HANDLE,
        .offset = 0U,
        .range = 0U
    };

    _bufferInfo.resize ( bufferCount, bufferInfo );

    constexpr VkDescriptorImageInfo const imageInfo
    {
        .sampler = VK_NULL_HANDLE,
        .imageView = VK_NULL_HANDLE,
        .imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
    };

    _imageInfo.resize ( imageCount, imageInfo );

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

    size_t const writeCount = WRITES_PER_CALL * neededCalls;
    _writeSets.resize ( writeCount, writeSet );

    size_t setIndex = 0U;
    size_t imageIndex = 0U;
    size_t bufferIndex = 0U;

    for ( size_t i = 0U; i < writeCount; i += WRITES_PER_CALL )
    {
        VkWriteDescriptorSet& lightVolumeBuffer = _writeSets[ i ];
        VkWriteDescriptorSet& reflectionImage = _writeSets[ i + 1U ];
        VkWriteDescriptorSet& reflectionBuffer = _writeSets[ i + 2U ];

        lightVolumeBuffer.dstSet = _descriptorSets[ setIndex++ ];
        lightVolumeBuffer.dstBinding = 0U;
        lightVolumeBuffer.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        lightVolumeBuffer.pBufferInfo = &_bufferInfo[ bufferIndex++ ];

        VkDescriptorSet reflectionSet = _descriptorSets[ setIndex++ ];

        reflectionImage.dstSet = reflectionSet;
        reflectionImage.dstBinding = 0U;
        reflectionImage.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
        reflectionImage.pImageInfo = &_imageInfo[ imageIndex++ ];

        reflectionBuffer.dstSet = reflectionSet;
        reflectionBuffer.dstBinding = 1U;
        reflectionBuffer.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        reflectionBuffer.pBufferInfo = &_bufferInfo[ bufferIndex++ ];
    }

    // Now all what is needed to do is to init "_imageInfo::imageView", "_bufferInfo::buffer" and "_bufferInfo::range".
    // Then to invoke vkUpdateDescriptorSets.
    return true;
}

void ReflectionLocalPass::DestroyDescriptorPool ( VkDevice device )
{
    if ( _descriptorPool == VK_NULL_HANDLE )
        return;

    vkDestroyDescriptorPool ( device, _descriptorPool, nullptr );
    _descriptorPool = VK_NULL_HANDLE;
    AV_UNREGISTER_DESCRIPTOR_POOL ( "ReflectionLocalPass::_descriptorPool" )
}

} // namespace pbr
