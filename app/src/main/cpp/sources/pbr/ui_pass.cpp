#include <pbr/ui_pass.h>
#include <pbr/ui_program.inc>
#include <av_assert.h>
#include <trace.h>


namespace pbr {

namespace {

constexpr size_t MAX_IMAGES = 1024U;
constexpr size_t MAX_VERTICES = 762600U;
constexpr size_t BUFFER_BYTES = MAX_VERTICES * sizeof ( UIVertexInfo );

} // end of anonymous namespace

//----------------------------------------------------------------------------------------------------------------------

bool UIPass::CommonDescriptorSet::Init ( VkDevice device,
    VkDescriptorPool descriptorPool,
    SamplerManager const &samplerManager
) noexcept
{
    if ( !_layout.Init ( device ) )
        return false;

    VkDescriptorSetLayout layout = _layout.GetLayout ();

    VkDescriptorSetAllocateInfo const allocateInfo
    {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
        .pNext = nullptr,
        .descriptorPool = descriptorPool,
        .descriptorSetCount = 1U,
        .pSetLayouts = &layout
    };

    bool const result = android_vulkan::Renderer::CheckVkResult (
        vkAllocateDescriptorSets ( device, &allocateInfo, &_descriptorSet ),
        "pbr::UIPass::CommonDescriptorSet::Init",
        "Can't allocate descriptor sets"
    );

    if ( !result )
        return false;

    VkDescriptorImageInfo const imageInfo[] =
    {
        {
            .sampler = samplerManager.GetPointSampler ()->GetSampler (),
            .imageView = VK_NULL_HANDLE,
            .imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
        },
        {
            .sampler = samplerManager.GetMaterialSampler ()->GetSampler (),
            .imageView = VK_NULL_HANDLE,
            .imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
        }
    };

    VkWriteDescriptorSet const writes[] =
    {
        {
            .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
            .pNext = nullptr,
            .dstSet = _descriptorSet,
            .dstBinding = BIND_ATLAS_SAMPLER,
            .dstArrayElement = 0U,
            .descriptorCount = 1U,
            .descriptorType = VK_DESCRIPTOR_TYPE_SAMPLER,
            .pImageInfo = imageInfo,
            .pBufferInfo = nullptr,
            .pTexelBufferView = nullptr
        },
        {
            .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
            .pNext = nullptr,
            .dstSet = _descriptorSet,
            .dstBinding = BIND_IMAGE_SAMPLER,
            .dstArrayElement = 0U,
            .descriptorCount = 1U,
            .descriptorType = VK_DESCRIPTOR_TYPE_SAMPLER,
            .pImageInfo = imageInfo + 1U,
            .pBufferInfo = nullptr,
            .pTexelBufferView = nullptr
        }
    };

    vkUpdateDescriptorSets ( device, static_cast<uint32_t> ( std::size ( writes ) ), writes, 0U, nullptr );

    _imageInfo =
    {
        .sampler = VK_NULL_HANDLE,
        .imageView = VK_NULL_HANDLE,
        .imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
    };

    _write =
    {
        .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
        .pNext = nullptr,
        .dstSet = _descriptorSet,
        .dstBinding = BIND_ATLAS_TEXTURE,
        .dstArrayElement = 0U,
        .descriptorCount = 1U,
        .descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,
        .pImageInfo = &_imageInfo,
        .pBufferInfo = nullptr,
        .pTexelBufferView = nullptr
    };

    return true;
}

void UIPass::CommonDescriptorSet::Destroy ( VkDevice device ) noexcept
{
    _layout.Destroy ( device );
}

void UIPass::CommonDescriptorSet::Update ( VkDevice device, VkImageView currentAtlas ) noexcept
{
    if ( _imageInfo.imageView == currentAtlas )
        return;

    _imageInfo.imageView = currentAtlas;
    vkUpdateDescriptorSets ( device, 1U, &_write, 0U, nullptr );
}

//----------------------------------------------------------------------------------------------------------------------

bool UIPass::Buffer::Init ( android_vulkan::Renderer &renderer,
    VkBufferUsageFlags usage,
    VkMemoryPropertyFlags memoryProperties,
    char const* name
) noexcept
{
    VkBufferCreateInfo const bufferInfo
    {
        .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0U,
        .size = BUFFER_BYTES,
        .usage = usage,
        .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
        .queueFamilyIndexCount = 0U,
        .pQueueFamilyIndices = nullptr
    };

    _name = name;
    VkDevice device = renderer.GetDevice ();

    bool result = android_vulkan::Renderer::CheckVkResult ( vkCreateBuffer ( device, &bufferInfo, nullptr, &_buffer ),
        "pbr::UIPass::Init",
        ( std::string ( "Can't create buffer: " ) + _name ).c_str ()
    );

    if ( !result )
        return false;

    AV_REGISTER_BUFFER ( _name )

    VkMemoryRequirements memoryRequirements;
    vkGetBufferMemoryRequirements ( device, _buffer, &memoryRequirements );

    result = renderer.TryAllocateMemory ( _memory,
        _memoryOffset,
        memoryRequirements,
        memoryProperties,
        ( std::string ( "pbr::UIPass::Init - Can't allocate device memory: " ) + _name ).c_str ()
    );

    if ( !result )
        return false;

    AV_REGISTER_DEVICE_MEMORY ( _name )

    return android_vulkan::Renderer::CheckVkResult ( vkBindBufferMemory ( device, _buffer, _memory, _memoryOffset ),
        "pbr::UIPass::Init",
        ( std::string ( "Can't bind memory: " ) + _name ).c_str ()
    );
}

void UIPass::Buffer::Destroy ( android_vulkan::Renderer &renderer ) noexcept
{
    if ( _buffer != VK_NULL_HANDLE )
    {
        vkDestroyBuffer ( renderer.GetDevice (), _buffer, nullptr );
        _buffer = VK_NULL_HANDLE;
        AV_UNREGISTER_BUFFER ( _name )
    }

    if ( _memory == VK_NULL_HANDLE )
        return;

    renderer.FreeMemory ( _memory, _memoryOffset );
    _memory = VK_NULL_HANDLE;
    _memoryOffset = 0U;
    AV_UNREGISTER_DEVICE_MEMORY ( _name )
}

//----------------------------------------------------------------------------------------------------------------------

bool UIPass::ImageDescriptorSets::Init ( VkDevice device, VkDescriptorPool descriptorPool ) noexcept
{
    if ( !_layout.Init ( device ) )
        return false;

    _descriptorSets.resize ( MAX_IMAGES, VK_NULL_HANDLE );
    std::vector<VkDescriptorSetLayout> const layouts ( MAX_IMAGES, _layout.GetLayout () );
    VkDescriptorSet* ds = _descriptorSets.data ();

    VkDescriptorSetAllocateInfo const allocateInfo
    {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
        .pNext = nullptr,
        .descriptorPool = descriptorPool,
        .descriptorSetCount = static_cast<uint32_t> ( MAX_IMAGES ),
        .pSetLayouts = layouts.data ()
    };

    bool const result = android_vulkan::Renderer::CheckVkResult (
        vkAllocateDescriptorSets ( device, &allocateInfo, ds ),
        "pbr::UIPass::ImageDescriptorSets::Init",
        "Can't allocate descriptor sets"
    );

    if ( !result )
        return false;

    VkDescriptorImageInfo const imageInfo
    {
        .sampler = VK_NULL_HANDLE,
        .imageView = VK_NULL_HANDLE,
        .imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
    };

    _imageInfo.resize ( MAX_IMAGES, imageInfo );

    VkWriteDescriptorSet const writeSet
    {
        .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
        .pNext = nullptr,
        .dstSet = VK_NULL_HANDLE,
        .dstBinding = BIND_IMAGE_TEXTURE,
        .dstArrayElement = 0U,
        .descriptorCount = 1U,
        .descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,
        .pImageInfo = nullptr,
        .pBufferInfo = nullptr,
        .pTexelBufferView = nullptr
    };

    _writeSets.resize ( MAX_IMAGES, writeSet );
    VkDescriptorImageInfo const* imageInfoData = _imageInfo.data ();

    for ( size_t i = 0U; i < MAX_IMAGES; ++i )
    {
        VkWriteDescriptorSet& write = _writeSets[ i ];
        write.dstSet = ds[ i ];
        write.pImageInfo = imageInfoData + i;
    }

    _readIndex = 0U;
    _writeIndex = 0U;

    return true;
}

void UIPass::ImageDescriptorSets::Destroy ( VkDevice device ) noexcept
{
    _layout.Destroy ( device );

    auto const clear = [] ( auto &v ) noexcept {
        v.clear ();
        v.shrink_to_fit ();
    };

    clear ( _writeSets );
    clear ( _descriptorSets );
    clear ( _imageInfo );
}

//----------------------------------------------------------------------------------------------------------------------

[[nodiscard]] bool UIPass::AcquirePresentTarget ( android_vulkan::Renderer &renderer,
    size_t &swapchainImageIndex
) noexcept
{
    AV_TRACE ( "Acquire swapchain image" )

    _framebufferIndex = std::numeric_limits<uint32_t>::max ();

    bool const result = android_vulkan::Renderer::CheckVkResult (
        vkAcquireNextImageKHR ( renderer.GetDevice (),
            renderer.GetSwapchain (),
            std::numeric_limits<uint64_t>::max (),
            _targetAcquiredSemaphore,
            VK_NULL_HANDLE,
            &_framebufferIndex
        ),

        "pbr::UIPass::AcquirePresentTarget",
        "Can't get presentation image index"
    );

    swapchainImageIndex = _framebufferIndex;
    return result;
}

bool UIPass::Init ( android_vulkan::Renderer &renderer,
    SamplerManager const &samplerManager,
    VkImageView transparent
) noexcept
{
    _transparent = transparent;

    if ( !_fontStorage.Init () )
        return false;

    constexpr auto stagingProps = static_cast<VkMemoryPropertyFlags> (
        AV_VK_FLAG ( VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT ) | AV_VK_FLAG ( VK_MEMORY_PROPERTY_HOST_COHERENT_BIT )
    );

    if ( !_staging.Init ( renderer, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, stagingProps, "pbr::UIPass::_staging" ) )
        return false;

    void* data;

    bool result = renderer.MapMemory ( data,
        _staging._memory,
        _staging._memoryOffset,
        "pbr::UIPass::Init",
        "Can't map memory"
    );

    if ( !result )
        return false;

    _data = static_cast<UIVertexInfo*> ( data );

    constexpr VkSemaphoreCreateInfo const semaphoreInfo
    {
        .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0U
    };

    VkDevice device = renderer.GetDevice ();

    result = android_vulkan::Renderer::CheckVkResult (
        vkCreateSemaphore ( device, &semaphoreInfo, nullptr, &_renderEndSemaphore ),
        "pbr::UIPass::Init",
        "Can't create render pass end semaphore"
    );

    if ( !result )
        return false;

    AV_REGISTER_SEMAPHORE ( "pbr::UIPass::_renderEndSemaphore" )

    result = android_vulkan::Renderer::CheckVkResult (
        vkCreateSemaphore ( device, &semaphoreInfo, nullptr, &_targetAcquiredSemaphore ),
        "pbr::UIPass::Init",
        "Can't create render target acquired semaphore"
    );

    if ( !result )
        return false;

    AV_REGISTER_SEMAPHORE ( "pbr::UIPass::_targetAcquiredSemaphore" )

    InitCommonStructures ();

    constexpr size_t commonDescriptorSetCount = 1U;
    constexpr size_t imageDescriptorSetCount = MAX_IMAGES;
    constexpr size_t descriptorSetCount = commonDescriptorSetCount + imageDescriptorSetCount;

    constexpr static VkDescriptorPoolSize const poolSizes[] =
    {
        {
            .type = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,
            .descriptorCount = static_cast<uint32_t> ( descriptorSetCount )
        },
        {
            .type = VK_DESCRIPTOR_TYPE_SAMPLER,
            .descriptorCount = 2U
        }
    };

    constexpr VkDescriptorPoolCreateInfo poolInfo
    {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0U,
        .maxSets = static_cast<uint32_t> ( descriptorSetCount ),
        .poolSizeCount = static_cast<uint32_t> ( std::size ( poolSizes ) ),
        .pPoolSizes = poolSizes
    };

    result = android_vulkan::Renderer::CheckVkResult (
        vkCreateDescriptorPool ( device, &poolInfo, nullptr, &_descriptorPool ),
        "pbr::UIPass::Init",
        "Can't create descriptor pool"
    );

    if ( !result )
        return false;

    AV_REGISTER_DESCRIPTOR_POOL ( "pbr::UIPass::_descriptorPool" )

    return _commonDescriptorSet.Init ( device, _descriptorPool, samplerManager ) &&
        _imageDescriptorSets.Init ( device, _descriptorPool ) &&
        _transformLayout.Init ( device ) &&

        _vertex.Init ( renderer,
            AV_VK_FLAG ( VK_BUFFER_USAGE_TRANSFER_DST_BIT ) | AV_VK_FLAG ( VK_BUFFER_USAGE_VERTEX_BUFFER_BIT ),
            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
            "pbr::UIPass::_vertex"
        ) &&

        _uniformPool.Init ( renderer,
            _transformLayout,
            sizeof ( UIProgram::Transform ),
            BIND_TRANSFORM,
            "pbr::UIPass::_uniformPool"
        );
}

void UIPass::Destroy ( android_vulkan::Renderer &renderer ) noexcept
{
    _uniformPool.Destroy ( renderer, "pbr::UIPass::_uniformPool" );

    VkDevice device = renderer.GetDevice ();
    _imageDescriptorSets.Destroy ( device );
    _commonDescriptorSet.Destroy ( device );
    _transformLayout.Destroy ( device );

    if ( _descriptorPool != VK_NULL_HANDLE )
    {
        vkDestroyDescriptorPool ( renderer.GetDevice (), _descriptorPool, nullptr );
        _descriptorPool = VK_NULL_HANDLE;
        AV_UNREGISTER_DESCRIPTOR_POOL ( "pbr::UIPass::_descriptorPool" )
    }

    if ( _targetAcquiredSemaphore != VK_NULL_HANDLE )
    {
        vkDestroySemaphore ( device, _targetAcquiredSemaphore, nullptr );
        _targetAcquiredSemaphore = VK_NULL_HANDLE;
        AV_UNREGISTER_SEMAPHORE ( "pbr::UIPass::_targetAcquiredSemaphore" )
    }

    if ( _renderEndSemaphore != VK_NULL_HANDLE )
    {
        vkDestroySemaphore ( device, _renderEndSemaphore, nullptr );
        _renderEndSemaphore = VK_NULL_HANDLE;
        AV_UNREGISTER_SEMAPHORE ( "pbr::UIPass::_renderEndSemaphore" )
    }

    _program.Destroy ( device );

    if ( _data )
    {
        renderer.UnmapMemory ( _staging._memory );
        _data = nullptr;
    }

    DestroyFramebuffers ( device );
    DestroyRenderPass ( device );

    _renderInfo.renderArea.extent =
    {
        .width = 0U,
        .height = 0U
    };

    _staging.Destroy ( renderer );
    _vertex.Destroy ( renderer );
    _fontStorage.Destroy ( renderer );

    _currentVertexIndex = 0U;
    _sceneImageVertexIndex = 0U;

    _jobs.clear ();
    _jobs.shrink_to_fit ();

    _isSceneImageEmbedded = false;
    _hasChanges = false;
}

bool UIPass::Commit () noexcept
{
    if ( _isSceneImageEmbedded )
        return true;

    if ( !_hasChanges )
    {
        // Passing zero vertices because "RequestUIBuffer" implicitly adds 6 vertices for scene image rectangle needs.
        if ( UIBufferResponse const response = RequestUIBuffer ( 0U ); !response )
        {
            return false;
        }
    }

    constexpr GXColorRGB white ( 1.0F, 1.0F, 1.0F, 1.0F );
    constexpr GXVec2 topLeft ( 0.0F, 0.0F );

    constexpr GXVec2 imageTopLeft ( 0.0F, 0.0F );
    constexpr GXVec2 imageBottomRight ( 1.0F, 1.0F );

    FontStorage::GlyphInfo const& g = _fontStorage.GetTransparentGlyphInfo ();

    AppendRectangle ( _data + _sceneImageVertexIndex,
        white,
        topLeft,
        _bottomRight,
        g._topLeft,
        g._bottomRight,
        imageTopLeft,
        imageBottomRight
    );

    _isSceneImageEmbedded = true;
    return true;
}

bool UIPass::Execute ( android_vulkan::Renderer &renderer, VkCommandBuffer commandBuffer, VkFence fence ) noexcept
{
    AV_TRACE ( "UI pass: Execute" )
    _renderInfo.framebuffer = _framebuffers[ _framebufferIndex ];

    vkCmdBeginRenderPass ( commandBuffer, &_renderInfo, VK_SUBPASS_CONTENTS_INLINE );
    _program.Bind ( commandBuffer );

    VkDescriptorSet const sets[] =
    {
        _transformDescriptorSet,
        _commonDescriptorSet._descriptorSet,

    };

    // TODO iterate over all jobs as well.
    _program.SetDescriptorSet ( commandBuffer, sets, 0U, static_cast<uint32_t> ( std::size ( sets ) ) );

    vkCmdDraw ( commandBuffer, GetVerticesPerRectangle (), 1U, 0U, 0U );

    vkCmdEndRenderPass ( commandBuffer );

    bool result = android_vulkan::Renderer::CheckVkResult ( vkEndCommandBuffer ( commandBuffer ),
        "pbr::UIPass::Execute",
        "Can't end command buffer"
    );

    if ( !result )
        return false;

    _submitInfo.pCommandBuffers = &commandBuffer;

    result = android_vulkan::Renderer::CheckVkResult (
        vkQueueSubmit ( renderer.GetQueue (), 1U, &_submitInfo, fence ),
        "pbr::UIPass::End",
        "Can't submit command buffer"
    );

    if ( !result )
        return false;

    VkResult presentResult = VK_ERROR_DEVICE_LOST;

    _presentInfo.pResults = &presentResult;
    _presentInfo.pSwapchains = &renderer.GetSwapchain ();
    _presentInfo.pImageIndices = &_framebufferIndex;

    result = android_vulkan::Renderer::CheckVkResult ( vkQueuePresentKHR ( renderer.GetQueue (), &_presentInfo ),
        "pbr::UIPass::EndFrame",
        "Can't present frame"
    );

    if ( !result )
        return false;

    return android_vulkan::Renderer::CheckVkResult ( presentResult,
        "pbr::UIPass::EndFrame",
        "Present queue has been failed"
    );
}

FontStorage& UIPass::GetFontStorage () noexcept
{
    return _fontStorage;
}

bool UIPass::OnSwapchainCrated ( android_vulkan::Renderer &renderer, VkImageView scene ) noexcept
{
    _scene = scene;

    VkExtent2D const& resolution = renderer.GetSurfaceSize ();
    VkExtent2D& r = _renderInfo.renderArea.extent;

    if ( ( r.width == resolution.width ) & ( r.height == resolution.height ) )
        return true;

    bool const result = _fontStorage.SetMediaResolution ( renderer, resolution ) &&
        CreateRenderPass ( renderer ) &&
        _program.Init ( renderer, _renderInfo.renderPass, 0U, resolution ) &&
        CreateFramebuffers ( renderer, resolution );

    if ( !result )
        return false;

    r = resolution;
    _bottomRight = GXVec2 ( static_cast<float> ( resolution.width ), static_cast<float> ( resolution.height ) );

    _isTransformChanged = true;
    return true;
}

void UIPass::OnSwapchainDestroyed ( VkDevice device ) noexcept
{
    _program.Destroy ( device );
    DestroyFramebuffers ( device );
}

void UIPass::RequestEmptyUI () noexcept
{
    if ( !_jobs.empty () )
    {
        _hasChanges = true;
        _isSceneImageEmbedded = false;
    }

    _jobs.clear ();
}

UIPass::UIBufferResponse UIPass::RequestUIBuffer ( size_t neededVertices ) noexcept
{
    size_t const actualVertices = neededVertices + GetVerticesPerRectangle ();

    if ( actualVertices > MAX_VERTICES )
    {
        android_vulkan::LogWarning ( "pbr::UIPass::RequestUIBuffer - Too many vertices was requested: %zu + %zu.",
            neededVertices,
            GetVerticesPerRectangle ()
        );

        AV_ASSERT ( false )
        return std::nullopt;
    }

    constexpr size_t probeIdx = 1U;
    size_t const cases[] = { 0U, _currentVertexIndex + actualVertices };
    size_t const nextIdx = cases[ static_cast<size_t> ( cases[ probeIdx ] < MAX_VERTICES ) ];

    _sceneImageVertexIndex = _currentVertexIndex;

    UIVertexBuffer const result = UIVertexBuffer ( _data + _currentVertexIndex + GetVerticesPerRectangle (),
        neededVertices
    );

    _currentVertexIndex = nextIdx;
    _isSceneImageEmbedded = false;

    return result;
}

[[maybe_unused]] void UIPass::SubmitImage ( Texture2DRef const &texture ) noexcept
{
    // TODO actual image and in use|free logic

    _jobs.emplace_back (
        Job
        {
            ._texture = texture,
            ._vertices = static_cast<uint32_t> ( GetVerticesPerRectangle () )
        }
    );

    _hasChanges = true;
}

void UIPass::SubmitRectangle () noexcept
{
    SubmitNonImage ( GetVerticesPerRectangle () );
}

void UIPass::SubmitText ( size_t usedVertices ) noexcept
{
    SubmitNonImage ( usedVertices );
}

bool UIPass::UploadGPUData ( android_vulkan::Renderer &renderer, VkCommandBuffer commandBuffer ) noexcept
{
    AV_TRACE ( "UI pass: Upload GPU data" )

    if ( !_fontStorage.UploadGPUData ( renderer, commandBuffer ) )
        return false;

    if ( _isTransformChanged )
        UpdateTransform ( renderer, commandBuffer );

    if ( _hasChanges )
        UpdateGeometry ( commandBuffer );

    return true;
}

void UIPass::AppendRectangle ( UIVertexInfo* target,
    GXColorRGB const &color,
    GXVec2 const &topLeft,
    GXVec2 const &bottomRight,
    GXVec3 const &glyphTopLeft,
    GXVec3 const &glyphBottomRight,
    GXVec2 const &imageTopLeft,
    GXVec2 const &imageBottomRight
) noexcept
{
    target[ 0U ] =
    {
        ._vertex = topLeft,
        ._color = color,
        ._atlas = glyphTopLeft,
        ._imageUV = imageTopLeft
    };

    target[ 1U ] =
    {
        ._vertex = GXVec2 ( bottomRight._data[ 0U ], topLeft._data[ 1U ] ),
        ._color = color,
        ._atlas = GXVec3 ( glyphBottomRight._data[ 0U ], glyphTopLeft._data[ 1U ], glyphTopLeft._data[ 2U ] ),
        ._imageUV = GXVec2 ( imageBottomRight._data[ 0U ], imageTopLeft._data[ 1U ] )
    };

    target[ 2U ] =
    {
        ._vertex = bottomRight,
        ._color = color,
        ._atlas = glyphBottomRight,
        ._imageUV = imageBottomRight
    };

    target[ 3U ] =
    {
        ._vertex = bottomRight,
        ._color = color,
        ._atlas = glyphBottomRight,
        ._imageUV = imageBottomRight
    };

    target[ 4U ] =
    {
        ._vertex = GXVec2 ( topLeft._data[ 0U ], bottomRight._data[ 1U ] ),
        ._color = color,
        ._atlas = GXVec3 ( glyphTopLeft._data[ 0U ], glyphBottomRight._data[ 1U ], glyphTopLeft._data[ 2U ] ),
        ._imageUV = GXVec2 ( imageTopLeft._data[ 0U ], imageBottomRight._data[ 1U ] )
    };

    target[ 5U ] =
    {
        ._vertex = topLeft,
        ._color = color,
        ._atlas = glyphTopLeft,
        ._imageUV = imageTopLeft
    };
}

bool UIPass::CreateFramebuffers ( android_vulkan::Renderer &renderer, VkExtent2D const &resolution ) noexcept
{
    size_t const framebufferCount = renderer.GetPresentImageCount ();
    _framebuffers.reserve ( framebufferCount );

    VkFramebufferCreateInfo framebufferInfo
    {
        .sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0U,
        .renderPass = _renderInfo.renderPass,
        .attachmentCount = 1U,
        .pAttachments = nullptr,
        .width = resolution.width,
        .height = resolution.height,
        .layers = 1U
    };

    VkDevice device = renderer.GetDevice ();

    for ( size_t i = 0U; i < framebufferCount; ++i )
    {
        framebufferInfo.pAttachments = &renderer.GetPresentImageView ( i );
        VkFramebuffer framebuffer;

        bool const result = android_vulkan::Renderer::CheckVkResult (
            vkCreateFramebuffer ( device, &framebufferInfo, nullptr, &framebuffer ),
            "pbr::UIPass::CreateFramebuffers",
            "Can't create a framebuffer"
        );

        if ( !result )
            return false;

        _framebuffers.push_back ( framebuffer );
        AV_REGISTER_FRAMEBUFFER ( "pbr::UIPass::_framebuffers" )
    }

    return true;
}

void UIPass::DestroyFramebuffers ( VkDevice device ) noexcept
{
    if ( _framebuffers.empty () )
        return;

    for ( auto framebuffer : _framebuffers )
    {
        vkDestroyFramebuffer ( device, framebuffer, nullptr );
        AV_UNREGISTER_FRAMEBUFFER ( "pbr::UIPass::_framebuffers" )
    }

    _framebuffers.clear ();
}

bool UIPass::CreateRenderPass ( android_vulkan::Renderer &renderer ) noexcept
{
    if ( _renderInfo.renderPass != VK_NULL_HANDLE )
        return true;

    VkAttachmentDescription const attachment
    {
        .flags = 0U,
        .format = renderer.GetSurfaceFormat (),
        .samples = VK_SAMPLE_COUNT_1_BIT,
        .loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
        .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
        .stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
        .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
        .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
        .finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR
    };

    constexpr static VkAttachmentReference reference
    {
        .attachment = 0U,
        .layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
    };

    constexpr VkSubpassDescription subpass
    {
        .flags = 0U,
        .pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS,
        .inputAttachmentCount = 0U,
        .pInputAttachments = nullptr,
        .colorAttachmentCount = 1U,
        .pColorAttachments = &reference,
        .pResolveAttachments = nullptr,
        .pDepthStencilAttachment = nullptr,
        .preserveAttachmentCount = 0U,
        .pPreserveAttachments = nullptr
    };

    constexpr VkSubpassDependency dependency
    {
        .srcSubpass = 0U,
        .dstSubpass = VK_SUBPASS_EXTERNAL,
        .srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
        .dstStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
        .srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
        .dstAccessMask = VK_ACCESS_NONE,
        .dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT
    };

    VkRenderPassCreateInfo const info
    {
        .sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0U,
        .attachmentCount = 1U,
        .pAttachments = &attachment,
        .subpassCount = 1U,
        .pSubpasses = &subpass,
        .dependencyCount = 1U,
        .pDependencies = &dependency
    };

    bool const result = android_vulkan::Renderer::CheckVkResult (
        vkCreateRenderPass ( renderer.GetDevice (), &info, nullptr, &_renderInfo.renderPass ),
        "pbr::UIPass::CreateRenderPass",
        "Can't create render pass"
    );

    if ( !result )
        return false;

    AV_REGISTER_RENDER_PASS ( "pbr::UIPass::_renderPass" )
    return true;
}

void UIPass::DestroyRenderPass ( VkDevice device ) noexcept
{
    if ( _renderInfo.renderPass == VK_NULL_HANDLE )
        return;

    vkDestroyRenderPass ( device, _renderInfo.renderPass, nullptr );
    _renderInfo.renderPass = VK_NULL_HANDLE;
    AV_UNREGISTER_RENDER_PASS ( "pbr::UIPass::_renderPass" )
}

void UIPass::InitCommonStructures () noexcept
{
    _renderInfo =
    {
        .sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
        .pNext = nullptr,
        .renderPass = VK_NULL_HANDLE,
        .framebuffer = VK_NULL_HANDLE,

        .renderArea =
        {
            .offset
            {
                .x = 0,
                .y = 0
            },

            .extent =
            {
                .width = 0U,
                .height = 0U
            }
        },

        .clearValueCount = 0U,
        .pClearValues = nullptr
    };

    _presentInfo =
    {
        .sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
        .pNext = nullptr,
        .waitSemaphoreCount = 1U,
        .pWaitSemaphores = &_renderEndSemaphore,
        .swapchainCount = 1U,
        .pSwapchains = nullptr,
        .pImageIndices = nullptr,
        .pResults = nullptr
    };

    constexpr static VkPipelineStageFlags waitStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;

    _submitInfo =
    {
        .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
        .pNext = nullptr,
        .waitSemaphoreCount = 1U,
        .pWaitSemaphores = &_targetAcquiredSemaphore,
        .pWaitDstStageMask = &waitStage,
        .commandBufferCount = 1U,
        .pCommandBuffers = nullptr,
        .signalSemaphoreCount = 1U,
        .pSignalSemaphores = &_renderEndSemaphore
    };
}

void UIPass::SubmitNonImage ( size_t usedVertices ) noexcept
{
    _hasChanges = true;

    if ( _jobs.empty () )
    {
        _jobs.emplace_back (
            Job
            {
                ._texture = {},
                ._vertices = static_cast<uint32_t> ( usedVertices )
            }
        );

        return;
    }

    Job& last = _jobs.back ();

    if ( !last._texture )
    {
        last._vertices += static_cast<uint32_t> ( GetVerticesPerRectangle () );
        return;
    }

    _jobs.emplace_back (
        Job
        {
            ._texture = {},
            ._vertices = static_cast<uint32_t> ( usedVertices )
        }
    );
}

void UIPass::UpdateGeometry ( VkCommandBuffer commandBuffer ) noexcept
{
    constexpr size_t const elementSize = sizeof ( UIVertexInfo );
    auto const offset = static_cast<VkDeviceSize> ( _sceneImageVertexIndex * elementSize );
    auto const size = static_cast<VkDeviceSize> ( elementSize * ( _currentVertexIndex - _sceneImageVertexIndex ) );

    VkBufferCopy const copy
    {
        .srcOffset = offset,
        .dstOffset = offset,
        .size = size
    };

    vkCmdCopyBuffer ( commandBuffer, _staging._buffer, _vertex._buffer, 1U, &copy );

    VkBufferMemoryBarrier const barrier
    {
        .sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER,
        .pNext = nullptr,
        .srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT,
        .dstAccessMask = VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT,
        .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
        .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
        .buffer = _vertex._buffer,
        .offset = offset,
        .size = size
    };

    vkCmdPipelineBarrier ( commandBuffer,
        VK_PIPELINE_STAGE_TRANSFER_BIT,
        VK_PIPELINE_STAGE_VERTEX_INPUT_BIT,
        0U,
        0U,
        nullptr,
        1U,
        &barrier,
        0U,
        nullptr
    );

    _hasChanges = false;
}

void UIPass::UpdateTransform ( android_vulkan::Renderer &renderer, VkCommandBuffer commandBuffer ) noexcept
{
    GXMat4 projection {};
    projection.Ortho ( _bottomRight._data[ 0U ], _bottomRight._data[ 1U ], 0.0F, 1.0F );

    GXVec2 offset {};
    offset.Multiply ( _bottomRight, -0.5F );

    GXMat4 translate {};
    translate.Translation ( offset._data[ 0U ], offset._data[ 1U ], 0.0F );

    GXMat4 alpha {};
    alpha.Multiply ( translate, renderer.GetPresentationEngineTransform () );

    UIProgram::Transform transform {};
    transform._transform.Multiply ( alpha, projection );

    _uniformPool.Push ( commandBuffer, &transform, sizeof ( transform ) );
    _transformDescriptorSet = _uniformPool.Acquire ();
    _uniformPool.Commit ();

    _uniformPool.IssueSync ( renderer.GetDevice (), commandBuffer );
    _isTransformChanged = false;
}

} // namespace pbr
