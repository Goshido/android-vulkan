#include <precompiled_headers.hpp>
#include <av_assert.hpp>
#include <logger.hpp>
#include <platform/windows/pbr/ui_pass.hpp>
#include <platform/windows/pbr/ui_program.inc>
#include <trace.hpp>
#include <vulkan_utils.hpp>


// FUCK - remove namespace
namespace pbr::windows {

namespace {

constexpr size_t ALLOCATE_COMMAND_BUFFERS = 8U;
constexpr size_t COMMAND_BUFFERS_PER_TEXTURE = 1U;
constexpr size_t INITIAL_COMMAND_BUFFERS = 32U;

constexpr size_t MAX_IMAGES = 1024U;
constexpr size_t MAX_VERTICES = 762600U;

constexpr size_t TRANSPARENT_DESCRIPTOR_SET_COUNT = 1U;

constexpr uint32_t COMMON_DESCRIPTOR_SET_IMAGE_COUNT = 2U;
constexpr uint32_t COMMON_DESCRIPTOR_SET_SAMPLER_COUNT = 3U;

constexpr std::string_view TEXT_LUT = "pbr/system/text-lut.png";

android_vulkan::Half2 const IMAGE_TOP_LEFT ( 0.0F, 0.0F );
android_vulkan::Half2 const IMAGE_BOTTOM_RIGHT ( 1.0F, 1.0F );

//----------------------------------------------------------------------------------------------------------------------

class ImageStorage final
{
    private:
        static size_t                                                   _commandBufferIndex;
        static std::vector<VkCommandBuffer>                             _commandBuffers;
        static VkCommandPool                                            _commandPool;
        static std::vector<VkFence>                                     _fences;
        static android_vulkan::Renderer*                                _renderer;
        static std::unordered_map<std::string, Texture2DRef const*>     _textureMap;
        static std::unordered_set<Texture2DRef>                         _textures;

    public:
        ImageStorage () = delete;

        ImageStorage ( ImageStorage const & ) = delete;
        ImageStorage &operator = ( ImageStorage const & ) = delete;

        ImageStorage ( ImageStorage && ) = delete;
        ImageStorage &operator = ( ImageStorage && ) = delete;

        ~ImageStorage () = delete;

        static void ReleaseImage ( Texture2DRef const &image ) noexcept;
        [[nodiscard]] static std::optional<Texture2DRef const> GetImage ( std::string const &asset ) noexcept;

        [[nodiscard]] static bool OnInitDevice ( android_vulkan::Renderer &renderer ) noexcept;
        static void OnDestroyDevice () noexcept;
        [[nodiscard]] static bool SyncGPU () noexcept;

    private:
        [[nodiscard]] static bool AllocateCommandBuffers ( size_t amount ) noexcept;
};

size_t ImageStorage::_commandBufferIndex = 0U;
std::vector<VkCommandBuffer> ImageStorage::_commandBuffers {};
VkCommandPool ImageStorage::_commandPool = VK_NULL_HANDLE;
std::vector<VkFence> ImageStorage::_fences {};
android_vulkan::Renderer* ImageStorage::_renderer = nullptr;
std::unordered_map<std::string, Texture2DRef const*> ImageStorage::_textureMap {};
std::unordered_set<Texture2DRef> ImageStorage::_textures {};

void ImageStorage::ReleaseImage ( Texture2DRef const &image ) noexcept
{
    // About to delete resource event somewhere in user code.
    // One reference inside '_textures'. One reference in user code. Two references in total.
    // Current reference count equal two: safe to delete all image resources.
    // More that two references means that someone is still holding reference. Do nothing in that case.

    if ( image.use_count () > 2U )
        return;

    auto const findResult = _textures.find ( image );
    Texture2DRef const &t = *findResult;
    t->FreeResources ( *_renderer );

    auto const end = _textureMap.cend ();

    for ( auto i = _textureMap.cbegin (); i != end; ++i )
    {
        if ( i->second != &t )
            continue;

        _textureMap.erase ( i );
        break;
    }

    _textures.erase ( findResult );
}

std::optional<Texture2DRef const> ImageStorage::GetImage ( std::string const &asset ) noexcept
{
    if ( _commandBuffers.size () - _commandBufferIndex < COMMAND_BUFFERS_PER_TEXTURE )
    {
        if ( !AllocateCommandBuffers ( ALLOCATE_COMMAND_BUFFERS ) )
        {
            return std::nullopt;
        }
    }

    if ( auto const findResult = _textureMap.find ( asset ); findResult != _textureMap.cend () )
        return *findResult->second;

    Texture2DRef texture = std::make_shared<android_vulkan::Texture2D> ();

    // Note UNORM is correct mode because of pixel shader, alpha blending and swapchain UNORM format.
    bool const result = texture->UploadData ( *_renderer,
        asset,
        android_vulkan::eColorSpace::Unorm,
        true,
        _commandBuffers[ _commandBufferIndex ],
        _fences[ _commandBufferIndex ]
    );

    if ( !result ) [[unlikely]]
        return std::nullopt;

    _commandBufferIndex += COMMAND_BUFFERS_PER_TEXTURE;

    auto status = _textures.emplace ( std::move ( texture ) );
    Texture2DRef const &t = *status.first;
    _textureMap.emplace ( asset, &t );

    return t;
}

bool ImageStorage::OnInitDevice ( android_vulkan::Renderer &renderer ) noexcept
{
    _renderer = &renderer;

    VkCommandPoolCreateInfo const createInfo
    {
        .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0U,
        .queueFamilyIndex = renderer.GetQueueFamilyIndex ()
    };

    bool const result = android_vulkan::Renderer::CheckVkResult (
        vkCreateCommandPool ( renderer.GetDevice (), &createInfo, nullptr, &_commandPool ),

        // FUCK - remove namespace
        "pbr::windows::ImageStorage::OnInitDevice",

        "Can't create command pool"
    );

    if ( !result ) [[unlikely]]
        return false;

    AV_SET_VULKAN_OBJECT_NAME ( renderer.GetDevice (), _commandPool, VK_OBJECT_TYPE_COMMAND_POOL, "UI image storage" )

    return AllocateCommandBuffers ( INITIAL_COMMAND_BUFFERS );
}

void ImageStorage::OnDestroyDevice () noexcept
{
    if ( !_textures.empty () ) [[unlikely]]
    {
        // FUCK - remove namespace
        android_vulkan::LogWarning ( "pbr::windows::ImageStorage::OnDestroyDevice - Memory leak." );
        AV_ASSERT ( false )
    }

    _textures.clear ();

    VkDevice device = _renderer->GetDevice ();

    if ( _commandPool != VK_NULL_HANDLE )
    {
        vkDestroyCommandPool ( device, _commandPool, nullptr );
        _commandPool = VK_NULL_HANDLE;
    }

    constexpr auto clean = [] ( auto &v ) noexcept {
        v.clear ();
        v.shrink_to_fit ();
    };

    clean ( _commandBuffers );

    for ( auto fence : _fences )
        vkDestroyFence ( device, fence, nullptr );

    clean ( _fences );
    _renderer = nullptr;
}

bool ImageStorage::SyncGPU () noexcept
{
    if ( !_commandBufferIndex ) [[likely]]
        return true;

    VkDevice device = _renderer->GetDevice ();
    auto const fenceCount = static_cast<uint32_t> ( _commandBufferIndex );
    VkFence* fences = _fences.data ();

    bool result = android_vulkan::Renderer::CheckVkResult (
        vkWaitForFences ( device, fenceCount, fences, VK_TRUE, std::numeric_limits<uint64_t>::max () ),

        // FUCK - remove namespace
        "pbr::windows::ImageStorage::SyncGPU",

        "Can't wait fence"
    );

    if ( !result ) [[unlikely]]
        return false;

    result = android_vulkan::Renderer::CheckVkResult ( vkResetFences ( device, fenceCount, fences ),

        // FUCK - remove namespace
        "pbr::windows::ImageStorage::SyncGPU",

        "Can't reset fence"
    );

    if ( !result ) [[unlikely]]
        return false;

    result = android_vulkan::Renderer::CheckVkResult (
        vkResetCommandPool ( device, _commandPool, 0U ),

        // FUCK - remove namespace
        "pbr::windows::ImageStorage::SyncGPU",

        "Can't reset command pool"
    );

    if ( !result ) [[unlikely]]
        return false;

    _commandBufferIndex = 0U;
    return true;
}

bool ImageStorage::AllocateCommandBuffers ( size_t amount ) noexcept
{
    size_t const current = _commandBuffers.size ();
    size_t const size = current + amount;
    _commandBuffers.resize ( size );

    VkCommandBufferAllocateInfo const allocateInfo
    {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
        .pNext = nullptr,
        .commandPool = _commandPool,
        .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
        .commandBufferCount = static_cast<uint32_t> ( amount )
    };

    VkDevice device = _renderer->GetDevice ();

    bool result = android_vulkan::Renderer::CheckVkResult (
        vkAllocateCommandBuffers ( device, &allocateInfo, &_commandBuffers[ current ] ),

        // FUCK - remove namespace
        "pbr::windows::ImageStorage::AllocateCommandBuffers",

        "Can't allocate command buffer"
    );

    if ( !result ) [[unlikely]]
        return false;

    _fences.resize ( size );

    constexpr VkFenceCreateInfo fenceInfo
    {
        .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0U
    };

    VkFence* const fences = _fences.data ();

    for ( size_t i = current; i < size; ++i )
    {
        result = android_vulkan::Renderer::CheckVkResult ( vkCreateFence ( device, &fenceInfo, nullptr, fences + i ),
        
            // FUCK - remove namespace
            "pbr::windows::ImageStorage::AllocateCommandBuffers",

            "Can't create fence"
        );

        if ( !result ) [[unlikely]]
            return false;

        AV_SET_VULKAN_OBJECT_NAME ( device, fences[ i ], VK_OBJECT_TYPE_FENCE, "UI #%zu", i )
    }

#if defined ( AV_ENABLE_VVL ) || defined ( AV_ENABLE_RENDERDOC ) || defined ( AV_ENABLE_NSIGHT )

    VkCommandBuffer* const buffers = _commandBuffers.data ();

    for ( size_t i = current; i < size; ++i )
        AV_SET_VULKAN_OBJECT_NAME ( device, buffers[ i ], VK_OBJECT_TYPE_COMMAND_BUFFER, "UI #%zu", i )

#endif // AV_ENABLE_VVL || AV_ENABLE_RENDERDOC || AV_ENABLE_NSIGHT

    return true;
}

} // end of anonymous namespace

//----------------------------------------------------------------------------------------------------------------------

bool UIPass::CommonDescriptorSet::Init ( android_vulkan::Renderer &renderer,
    VkDescriptorPool descriptorPool,
    SamplerManager const &samplerManager
) noexcept
{
    VkCommandPoolCreateInfo const poolInfo
    {
        .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
        .pNext = nullptr,
        .flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT,
        .queueFamilyIndex = renderer.GetQueueFamilyIndex ()
    };

    constexpr VkFenceCreateInfo fenceInfo
    {
        .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0U
    };

    VkDevice device = renderer.GetDevice ();

    bool result =
        android_vulkan::Renderer::CheckVkResult (
            vkCreateCommandPool ( device, &poolInfo, nullptr, &_pool ),

            // FUCK - remove namespace
            "pbr::windows::UIPass::CommonDescriptorSet::Init",

            "Can't create command pool"
        ) &&

        android_vulkan::Renderer::CheckVkResult ( vkCreateFence ( device, &fenceInfo, nullptr, &_fence ),

            // FUCK - remove namespace
            "pbr::windows::UIPass::CommonDescriptorSet::Init",

            "Can't create fence"
        ) &&

        _layout.Init ( device );

    if ( !result ) [[unlikely]]
        return false;

    AV_SET_VULKAN_OBJECT_NAME ( device, _pool, VK_OBJECT_TYPE_COMMAND_POOL, "Text LUT" );
    AV_SET_VULKAN_OBJECT_NAME ( device, _fence, VK_OBJECT_TYPE_FENCE, "Text LUT" );

    VkDescriptorSetAllocateInfo const allocateInfo
    {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
        .pNext = nullptr,
        .descriptorPool = descriptorPool,
        .descriptorSetCount = 1U,
        .pSetLayouts = &_layout.GetLayout ()
    };

    VkCommandBufferAllocateInfo const bufferAllocateInfo
    {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
        .pNext = nullptr,
        .commandPool = _pool,
        .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
        .commandBufferCount = 1U
    };

    VkCommandBuffer commandBuffer = VK_NULL_HANDLE;

    result =
        android_vulkan::Renderer::CheckVkResult (
            vkAllocateDescriptorSets ( device, &allocateInfo, &_descriptorSet ),

            // FUCK - remove namespace
            "pbr::windows::UIPass::CommonDescriptorSet::Init",

            "Can't allocate descriptor sets"
        ) &&

        android_vulkan::Renderer::CheckVkResult (
            vkAllocateCommandBuffers ( device, &bufferAllocateInfo, &commandBuffer ),

            // FUCK - remove namespace
            "pbr::windows::UIPass::CommonDescriptorSet::Init",

            "Can't allocate command buffer"
        );

    if ( !result ) [[unlikely]]
        return false;

    AV_SET_VULKAN_OBJECT_NAME ( device, _descriptorSet, VK_OBJECT_TYPE_DESCRIPTOR_SET, "UI common" )
    AV_SET_VULKAN_OBJECT_NAME ( device, commandBuffer, VK_OBJECT_TYPE_COMMAND_BUFFER, "Text LUT" )

    result = _textLUT.UploadData ( renderer,
        TEXT_LUT,
        android_vulkan::eColorSpace::Unorm,
        false,
        commandBuffer,
        _fence
    );

    if ( !result ) [[unlikely]]
        return false;

    VkDescriptorImageInfo const imageInfo[] =
    {
        {
            .sampler = samplerManager.GetPointSampler (),
            .imageView = VK_NULL_HANDLE,
            .imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
        },
        {
            .sampler = samplerManager.GetClampToEdgeSampler (),
            .imageView = _textLUT.GetImageView (),
            .imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
        },
        {
            .sampler = samplerManager.GetMaterialSampler (),
            .imageView = VK_NULL_HANDLE,
            .imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
        }
    };

    constexpr size_t textLUTResources = 1U;

    AV_SET_VULKAN_OBJECT_NAME ( device, _textLUT.GetImage (), VK_OBJECT_TYPE_IMAGE, "Text LUT" )
    AV_SET_VULKAN_OBJECT_NAME ( device, imageInfo[ textLUTResources ].imageView, VK_OBJECT_TYPE_IMAGE_VIEW, "Text LUT")

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
            .dstBinding = BIND_TEXT_LUT_TEXTURE,
            .dstArrayElement = 0U,
            .descriptorCount = 1U,
            .descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,
            .pImageInfo = imageInfo + textLUTResources,
            .pBufferInfo = nullptr,
            .pTexelBufferView = nullptr
        },
        {
            .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
            .pNext = nullptr,
            .dstSet = _descriptorSet,
            .dstBinding = BIND_TEXT_LUT_SAMPLER,
            .dstArrayElement = 0U,
            .descriptorCount = 1U,
            .descriptorType = VK_DESCRIPTOR_TYPE_SAMPLER,
            .pImageInfo = imageInfo + textLUTResources,
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
            .pImageInfo = imageInfo + 2U,
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

void UIPass::CommonDescriptorSet::Destroy ( android_vulkan::Renderer &renderer ) noexcept
{
    _layout.Destroy ( renderer.GetDevice () );
    _textLUT.FreeResources ( renderer );
    VkDevice device = renderer.GetDevice ();

    if ( _fence != VK_NULL_HANDLE ) [[unlikely]]
        vkDestroyFence ( device, std::exchange ( _fence, VK_NULL_HANDLE ), nullptr );

    if ( _pool != VK_NULL_HANDLE ) [[unlikely]]
    {
        vkDestroyCommandPool ( device, std::exchange ( _pool, VK_NULL_HANDLE ), nullptr );
    }
}

bool UIPass::CommonDescriptorSet::Update ( android_vulkan::Renderer &renderer, VkImageView currentAtlas ) noexcept
{
    if ( !FreeTransferResources ( renderer ) ) [[unlikely]]
        return false;

    if ( _imageInfo.imageView == currentAtlas ) [[likely]]
        return true;

    _imageInfo.imageView = currentAtlas;
    vkUpdateDescriptorSets ( renderer.GetDevice (), 1U, &_write, 0U, nullptr );
    return true;
}

[[nodiscard]] bool UIPass::CommonDescriptorSet::FreeTransferResources ( android_vulkan::Renderer &renderer ) noexcept
{
    if ( _pool == VK_NULL_HANDLE ) [[likely]]
        return true;

    VkDevice device = renderer.GetDevice ();

    bool const result = android_vulkan::Renderer::CheckVkResult (
        vkWaitForFences ( device, 1U, &_fence, VK_TRUE, std::numeric_limits<uint64_t>::max () ),

        // FUCK - remove namespace
        "pbr::windows::UIPass::CommonDescriptorSet::UploadTextLUT",

        "Can't wait for fence"
    );

    if ( !result ) [[unlikely]]
        return false;

    vkDestroyFence ( device, std::exchange ( _fence, VK_NULL_HANDLE ), nullptr );

    if ( _pool != VK_NULL_HANDLE ) [[unlikely]]
        vkDestroyCommandPool ( device, std::exchange ( _pool, VK_NULL_HANDLE ), nullptr );

    _textLUT.FreeTransferResources ( renderer );
    return true;
}

//----------------------------------------------------------------------------------------------------------------------

bool UIPass::Buffer::Init ( android_vulkan::Renderer &renderer,
    size_t size,
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
        .size = size,
        .usage = usage,
        .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
        .queueFamilyIndexCount = 0U,
        .pQueueFamilyIndices = nullptr
    };

    _name = name;
    VkDevice device = renderer.GetDevice ();

    bool result = android_vulkan::Renderer::CheckVkResult ( vkCreateBuffer ( device, &bufferInfo, nullptr, &_buffer ),

        // FUCK - remove namespace
        "pbr::windows::UIPass::Init",

        ( std::string ( "Can't create buffer: " ) + _name ).c_str ()
    );

    if ( !result ) [[unlikely]]
        return false;

    AV_SET_VULKAN_OBJECT_NAME ( device, _buffer, VK_OBJECT_TYPE_BUFFER, "%s", _name )

    VkMemoryRequirements memoryRequirements;
    vkGetBufferMemoryRequirements ( device, _buffer, &memoryRequirements );

    result = renderer.TryAllocateMemory ( _memory,
        _memoryOffset,
        memoryRequirements,
        memoryProperties,
        ( std::string ( "pbr::UIPass::Init - Can't allocate device memory: " ) + _name ).c_str ()
    );

    if ( !result ) [[unlikely]]
        return false;

    return android_vulkan::Renderer::CheckVkResult ( vkBindBufferMemory ( device, _buffer, _memory, _memoryOffset ),

        // FUCK - remove namespace
        "pbr::windows::UIPass::Init",

        ( std::string ( "Can't bind memory: " ) + _name ).c_str ()
    );
}

void UIPass::Buffer::Destroy ( android_vulkan::Renderer &renderer ) noexcept
{
    if ( _buffer != VK_NULL_HANDLE ) [[likely]]
        vkDestroyBuffer ( renderer.GetDevice (), std::exchange ( _buffer, VK_NULL_HANDLE ), nullptr );

    if ( _memory == VK_NULL_HANDLE ) [[unlikely]]
        return;

    renderer.FreeMemory ( std::exchange ( _memory, VK_NULL_HANDLE ), _memoryOffset );
    _memoryOffset = 0U;
}

//----------------------------------------------------------------------------------------------------------------------

UIPass::BufferStream::BufferStream ( size_t elementSize ) noexcept:
    _elementSize ( elementSize )
{
    // NOTHING
}

bool UIPass::BufferStream::Init ( android_vulkan::Renderer &renderer,
    char const *vertexName,
    char const *stagingName
) noexcept
{
    constexpr VkMemoryPropertyFlags stagingProps = AV_VK_FLAG ( VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT ) |
        AV_VK_FLAG ( VK_MEMORY_PROPERTY_HOST_COHERENT_BIT );

    size_t const size = MAX_VERTICES * _elementSize;

    if ( !_staging.Init ( renderer, size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, stagingProps, stagingName ) ) [[unlikely]]
        return false;

    void* data;

    bool const result = renderer.MapMemory ( data,
        _staging._memory,
        _staging._memoryOffset,

        // FUCK - remove namespace
        "pbr::windows::UIPass::BufferStream::Init",

        "Can't map memory"
    );

    if ( !result ) [[unlikely]]
        return false;

    _data = static_cast<uint8_t*> ( data );

    constexpr VkBufferUsageFlags vertexUsage = AV_VK_FLAG ( VK_BUFFER_USAGE_TRANSFER_DST_BIT ) |
        AV_VK_FLAG ( VK_BUFFER_USAGE_VERTEX_BUFFER_BIT );

    if ( !_vertex.Init ( renderer, size, vertexUsage, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, vertexName ) ) [[unlikely]]
        return false;

    _barrier.buffer = _vertex._buffer;
    return true;
}

void UIPass::BufferStream::Destroy ( android_vulkan::Renderer &renderer ) noexcept
{
    if ( _data ) [[likely]]
    {
        renderer.UnmapMemory ( _staging._memory );
        _data = nullptr;
    }

    _staging.Destroy ( renderer );
    _vertex.Destroy ( renderer );
}

VkBuffer UIPass::BufferStream::GetBuffer () const noexcept
{
    return _vertex._buffer;
}

void *UIPass::BufferStream::GetData ( size_t startIndex ) const noexcept
{
    return _data + startIndex * _elementSize;
}

void UIPass::BufferStream::UpdateGeometry ( VkCommandBuffer commandBuffer, size_t readIdx, size_t writeIdx ) noexcept
{
    auto const offset = static_cast<VkDeviceSize> ( _elementSize * readIdx );
    auto const size = static_cast<VkDeviceSize> ( _elementSize * ( writeIdx - readIdx ) );

    VkBufferCopy const copy
    {
        .srcOffset = offset,
        .dstOffset = offset,
        .size = size
    };

    vkCmdCopyBuffer ( commandBuffer, _staging._buffer, _vertex._buffer, 1U, &copy );

    _barrier.offset = offset;
    _barrier.size = size;

    vkCmdPipelineBarrier ( commandBuffer,
        VK_PIPELINE_STAGE_TRANSFER_BIT,
        VK_PIPELINE_STAGE_VERTEX_INPUT_BIT,
        0U,
        0U,
        nullptr,
        1U,
        &_barrier,
        0U,
        nullptr
    );
}

//----------------------------------------------------------------------------------------------------------------------

bool UIPass::ImageDescriptorSets::Init ( VkDevice device,
    VkDescriptorPool descriptorPool,
    VkImageView transparent
) noexcept
{
    if ( !_layout.Init ( device ) ) [[unlikely]]
        return false;

    constexpr size_t count = MAX_IMAGES + TRANSPARENT_DESCRIPTOR_SET_COUNT;
    _descriptorSets.resize ( count, VK_NULL_HANDLE );

    std::vector<VkDescriptorSetLayout> const layouts ( count, _layout.GetLayout () );
    VkDescriptorSet* ds = _descriptorSets.data ();

    VkDescriptorSetAllocateInfo const allocateInfo
    {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
        .pNext = nullptr,
        .descriptorPool = descriptorPool,
        .descriptorSetCount = static_cast<uint32_t> ( count ),
        .pSetLayouts = layouts.data ()
    };

    bool const result = android_vulkan::Renderer::CheckVkResult (
        vkAllocateDescriptorSets ( device, &allocateInfo, ds ),

        // FUCK - remove namespace
        "pbr::windows::UIPass::ImageDescriptorSets::Init",

        "Can't allocate descriptor sets"
    );

    if ( !result ) [[unlikely]]
        return false;

#if defined ( AV_ENABLE_VVL ) || defined ( AV_ENABLE_RENDERDOC ) || defined ( AV_ENABLE_NSIGHT )

    for ( size_t i = 0U; i < count; ++i )
        AV_SET_VULKAN_OBJECT_NAME ( device, ds[ i ], VK_OBJECT_TYPE_DESCRIPTOR_SET, "UI image #%zu", i )

#endif // AV_ENABLE_VVL || AV_ENABLE_RENDERDOC || AV_ENABLE_NSIGHT

    _transparent = _descriptorSets.back ();
    _descriptorSets.pop_back ();

    VkDescriptorImageInfo imageInfo
    {
        .sampler = VK_NULL_HANDLE,
        .imageView = VK_NULL_HANDLE,
        .imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
    };

    _imageInfo.resize ( MAX_IMAGES, imageInfo );

    VkWriteDescriptorSet writeSet
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
        VkWriteDescriptorSet &write = _writeSets[ i ];
        write.dstSet = ds[ i ];
        write.pImageInfo = imageInfoData + i;
    }

    _commitIndex = 0U;
    _startIndex = 0U;
    _written = 0U;

    // Init transparent descriptor set.
    imageInfo.imageView = transparent;
    writeSet.dstSet = _transparent;
    writeSet.pImageInfo = &imageInfo;
    vkUpdateDescriptorSets ( device, 1U, &writeSet, 0U, nullptr );

    return true;
}

void UIPass::ImageDescriptorSets::Destroy ( VkDevice device ) noexcept
{
    _layout.Destroy ( device );

    constexpr auto clear = [] ( auto &v ) noexcept {
        v.clear ();
        v.shrink_to_fit ();
    };

    clear ( _writeSets );
    clear ( _descriptorSets );
    clear ( _imageInfo );
}

void UIPass::ImageDescriptorSets::Commit ( VkDevice device ) noexcept
{
    if ( !_written )
        return;

    size_t const idx = _startIndex + _written;
    size_t const cases[] = { 0U, idx - MAX_IMAGES };
    size_t const more = cases[ static_cast<size_t> ( idx > MAX_IMAGES ) ];
    size_t const available = _written - more;

    VkWriteDescriptorSet const* writeSets = _writeSets.data ();
    vkUpdateDescriptorSets ( device, static_cast<uint32_t> ( available ), writeSets + _startIndex, 0U, nullptr );

    _commitIndex = _startIndex;
    _startIndex = idx % MAX_IMAGES;
    _written = 0U;

    if ( more >= 1U ) [[unlikely]]
    {
        vkUpdateDescriptorSets ( device, static_cast<uint32_t> ( more ), writeSets, 0U, nullptr );
    }
}

void UIPass::ImageDescriptorSets::Push ( VkImageView view ) noexcept
{
    _imageInfo[ ( _startIndex + _written ) % MAX_IMAGES ].imageView = view;
    ++_written;
}

//----------------------------------------------------------------------------------------------------------------------

void UIPass::InUseImageTracker::Destroy () noexcept
{
    for ( Entry &entry : _registry )
    {
        auto const end = entry.cend ();

        for ( auto e = entry.cbegin (); e != end; )
        {
            ImageStorage::ReleaseImage ( e->first );
            e = entry.erase ( e );
        }
    }
}

void UIPass::InUseImageTracker::CollectGarbage ( size_t commandBufferIndex ) noexcept
{
    Entry &entry = _registry[ commandBufferIndex ];
    auto end = entry.end ();

    for ( auto i = entry.begin (); i != end; )
    {
        size_t &references = i->second;

        if ( --references; references )
        {
            ++i;
            continue;
        }

        ImageStorage::ReleaseImage ( i->first );
        i = entry.erase ( i );
    }
}

void UIPass::InUseImageTracker::MarkInUse ( Texture2DRef const &texture, size_t commandBufferIndex )
{
    Entry &entry = _registry[ commandBufferIndex ];
    auto i = entry.find ( texture );

    if ( i == entry.end () )
    {
        auto result = entry.emplace ( texture, 1U );
        i = result.first;
    }

    i->second = 2U;
}

//----------------------------------------------------------------------------------------------------------------------

bool UIPass::Execute ( VkCommandBuffer commandBuffer, size_t commandBufferIndex ) noexcept
{
    AV_TRACE ( "UI pass: Execute" )
    AV_VULKAN_GROUP ( commandBuffer, "UI" )

    if ( !ImageStorage::SyncGPU () ) [[unlikely]]
        return false;

    if ( _jobs.empty () ) [[unlikely]]
    {
        _inUseImageTracker.CollectGarbage ( commandBufferIndex );
        return true;
    }

    _program.Bind ( commandBuffer );

    VkDescriptorSet const sets[] = { _transformDescriptorSet, _commonDescriptorSet._descriptorSet };
    _program.SetDescriptorSet ( commandBuffer, sets, 0U, static_cast<uint32_t> ( std::size ( sets ) ) );

    VkBuffer const buffers[] = { _positions.GetBuffer (), _rest.GetBuffer () };
    constexpr VkDeviceSize const offsets[] = { 0U, 0U };
    static_assert ( std::size ( buffers ) == std::size ( offsets ) );

    vkCmdBindVertexBuffers ( commandBuffer, 0U, static_cast<uint32_t> ( std::size ( buffers ) ), buffers, offsets );

    auto start = static_cast<uint32_t> ( _readVertexIndex );
    size_t imageIdx = _imageDescriptorSets._commitIndex;
    VkDescriptorSet* ds = _imageDescriptorSets._descriptorSets.data ();

    for ( auto const &job : _jobs )
    {
        if ( !job._texture )
        {
            _program.SetDescriptorSet ( commandBuffer, &_imageDescriptorSets._transparent, 2U, 1U );
        }
        else
        {
            _inUseImageTracker.MarkInUse ( *job._texture, commandBufferIndex );
            _program.SetDescriptorSet ( commandBuffer, ds + imageIdx, 2U, 1U );
            imageIdx = ( imageIdx + 1U ) % MAX_IMAGES;
        }

        vkCmdDraw ( commandBuffer, job._vertices, 1U, start, 0U );
        start += job._vertices;
    }

    _inUseImageTracker.CollectGarbage ( commandBufferIndex );
    return true;
}

FontStorage &UIPass::GetFontStorage () noexcept
{
    return _fontStorage;
}

size_t UIPass::GetUsedVertexCount () const noexcept
{
    return _writeVertexIndex - _readVertexIndex;
}

bool UIPass::OnInitDevice ( android_vulkan::Renderer &renderer,
    SamplerManager const &samplerManager,
    VkImageView transparent
) noexcept
{
    bool result = _fontStorage.Init ( renderer ) &&
        _positions.Init ( renderer, "UI positions", "UI position staging" ) &&
        _rest.Init ( renderer, "UI rest", "UI rest staging" );

    if ( !result ) [[unlikely]]
        return false;

    constexpr size_t commonDescriptorSetCount = 1U;
    constexpr size_t descriptorSetCount = commonDescriptorSetCount + MAX_IMAGES + TRANSPARENT_DESCRIPTOR_SET_COUNT;

    constexpr auto images = COMMON_DESCRIPTOR_SET_IMAGE_COUNT +
        static_cast<uint32_t> ( MAX_IMAGES + TRANSPARENT_DESCRIPTOR_SET_COUNT );

    constexpr static VkDescriptorPoolSize const poolSizes[] =
    {
        {
            .type = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,
            .descriptorCount = images

        },
        {
            .type = VK_DESCRIPTOR_TYPE_SAMPLER,
            .descriptorCount = COMMON_DESCRIPTOR_SET_SAMPLER_COUNT
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

    VkDevice device = renderer.GetDevice ();

    result = android_vulkan::Renderer::CheckVkResult (
        vkCreateDescriptorPool ( device, &poolInfo, nullptr, &_descriptorPool ),

        // FUCK - remove namespace
        "pbr::windows::UIPass::OnInitDevice",

        "Can't create descriptor pool"
    );

    if ( !result ) [[unlikely]]
        return false;

    AV_SET_VULKAN_OBJECT_NAME ( device, _descriptorPool, VK_OBJECT_TYPE_DESCRIPTOR_POOL, "UI pass" )

    return _commonDescriptorSet.Init ( renderer, _descriptorPool, samplerManager ) &&
        _imageDescriptorSets.Init ( device, _descriptorPool, transparent ) &&
        _transformLayout.Init ( device ) &&
        ImageStorage::OnInitDevice ( renderer ) &&

        _uniformPool.Init ( renderer,
            _transformLayout,

            // FUCK - remove namespace, one liner
            sizeof ( android::UIProgram::Transform ),
            BIND_TRANSFORM,
            "UI pass"
        );
}

void UIPass::OnDestroyDevice ( android_vulkan::Renderer &renderer ) noexcept
{
    _uniformPool.Destroy ( renderer );

    VkDevice device = renderer.GetDevice ();
    _imageDescriptorSets.Destroy ( device );
    _commonDescriptorSet.Destroy ( renderer );
    _transformLayout.Destroy ( device );

    if ( _descriptorPool != VK_NULL_HANDLE ) [[likely]]
        vkDestroyDescriptorPool ( device, std::exchange ( _descriptorPool, VK_NULL_HANDLE ), nullptr );

    _program.Destroy ( device );

    _positions.Destroy ( renderer );
    _rest.Destroy ( renderer );

    _inUseImageTracker.Destroy ();
    _fontStorage.Destroy ( renderer );

    _writeVertexIndex = 0U;
    _readVertexIndex = 0U;

    _jobs.clear ();
    _jobs.shrink_to_fit ();

    _hasChanges = false;

    ImageStorage::OnDestroyDevice ();
}

bool UIPass::OnSwapchainCreated ( android_vulkan::Renderer &renderer,
    VkRenderPass renderPass,
    uint32_t subpass
) noexcept
{
    VkExtent2D const &resolution = renderer.GetSurfaceSize ();
    VkExtent2D &r = _currentResolution;

    if ( ( r.width == resolution.width ) & ( r.height == resolution.height ) )
        return true;

    VkDevice device = renderer.GetDevice ();
    _program.Destroy ( device );

    bool const result = _program.Init ( renderer,
        renderPass,
        subpass,
        BrightnessInfo ( _brightnessBalance ),
        resolution
    );

    if ( !result ) [[unlikely]]
        return false;

    r = resolution;
    VkExtent2D const &viewport = renderer.GetViewportResolution ();
    _bottomRight = GXVec2 ( static_cast<float> ( viewport.width ), static_cast<float> ( viewport.height ) );
    _isTransformChanged = true;
    return true;
}

void UIPass::OnSwapchainDestroyed () noexcept
{
    _inUseImageTracker.Destroy ();
}

void UIPass::RequestEmptyUI () noexcept
{
    if ( !_jobs.empty () )
        _hasChanges = true;

    _jobs.clear ();
}

UIPass::UIBufferResponse UIPass::RequestUIBuffer ( size_t neededVertices ) noexcept
{
    RequestEmptyUI ();

    if ( neededVertices > MAX_VERTICES ) [[unlikely]]
    {
        // FUCK - remove namespace
        android_vulkan::LogWarning (
            "pbr::windows::UIPass::RequestUIBuffer - Too many vertices was requested: %zu + %zu.",
            neededVertices,
            GetVerticesPerRectangle ()
        );

        AV_ASSERT ( false )
        return std::nullopt;
    }

    size_t const cases[] = { 0U, _writeVertexIndex };
    size_t const nextIdx = cases[ _writeVertexIndex + neededVertices <= MAX_VERTICES ];

    _readVertexIndex = nextIdx;
    _writeVertexIndex = nextIdx + neededVertices;

    return UIPass::UIBufferResponse
    {
        {
            ._positions { static_cast<GXVec2*> ( _positions.GetData ( nextIdx ) ), neededVertices },
            ._vertices { static_cast<UIVertex*> ( _rest.GetData (nextIdx) ), neededVertices }
        }
    };
}

bool UIPass::SetBrightness ( android_vulkan::Renderer &renderer,
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
        _currentResolution
    );

    if ( !result ) [[unlikely]]
        return false;

    _brightnessBalance = brightnessBalance;
    return true;
}

void UIPass::SubmitImage ( Texture2DRef const &texture ) noexcept
{
    _imageDescriptorSets.Push ( texture->GetImageView () );

    _jobs.emplace_back (
        Job
        {
            ._texture = &texture,
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

bool UIPass::UploadGPUData ( android_vulkan::Renderer &renderer,
    VkCommandBuffer commandBuffer,
    size_t framebufferIndex
) noexcept
{
    AV_TRACE ( "UI pass: Upload GPU data" )
    AV_VULKAN_GROUP ( commandBuffer, "Upload UI data" )

    VkDevice device = renderer.GetDevice ();
    _imageDescriptorSets.Commit ( device );

    bool const result =
        _fontStorage.UploadGPUData ( renderer, commandBuffer, framebufferIndex ) &&
        _commonDescriptorSet.Update ( renderer, _fontStorage.GetAtlasImageView () );

    if ( !result ) [[unlikely]]
        return false;

    if ( _isTransformChanged )
        UpdateTransform ( renderer, commandBuffer );

    if ( _hasChanges )
        UpdateGeometry ( commandBuffer );

    return true;
}

void UIPass::AppendImage ( GXVec2* targetPositions,
    UIVertex* targetVertices,
    GXColorUNORM color,
    GXVec2 const &topLeft,
    GXVec2 const &bottomRight
) noexcept
{
    targetPositions[ 0U ] = topLeft;

    UIVertex &v0 = targetVertices[ 0U ];
    v0._uv = IMAGE_TOP_LEFT;
    v0._uiPrimitiveType = PBR_UI_PRIMITIVE_TYPE_IMAGE;
    v0._color = color;

    targetPositions[ 1U ] = GXVec2 ( bottomRight._data[ 0U ], topLeft._data[ 1U ] );

    UIVertex &v1 = targetVertices[ 1U ];
    v1._uv = android_vulkan::Half2 ( IMAGE_BOTTOM_RIGHT._data[ 0U ], IMAGE_TOP_LEFT._data[ 1U ] );
    v1._uiPrimitiveType = PBR_UI_PRIMITIVE_TYPE_IMAGE;
    v1._color = color;

    targetPositions[ 2U ] = bottomRight;

    UIVertex &v2 = targetVertices[ 2U ];
    v2._uv = IMAGE_BOTTOM_RIGHT;
    v2._uiPrimitiveType = PBR_UI_PRIMITIVE_TYPE_IMAGE;
    v2._color = color;

    targetPositions[ 3U ] = bottomRight;

    UIVertex &v3 = targetVertices[ 3U ];
    v3._uv = IMAGE_BOTTOM_RIGHT;
    v3._uiPrimitiveType = PBR_UI_PRIMITIVE_TYPE_IMAGE;
    v3._color = color;

    targetPositions[ 4U ] = GXVec2 ( topLeft._data[ 0U ], bottomRight._data[ 1U ] );

    UIVertex &v4 = targetVertices[ 4U ];
    v4._uv = android_vulkan::Half2 ( IMAGE_TOP_LEFT._data[ 0U ], IMAGE_BOTTOM_RIGHT._data[ 1U ] );
    v4._uiPrimitiveType = PBR_UI_PRIMITIVE_TYPE_IMAGE;
    v4._color = color;

    targetPositions[ 5U ] = topLeft;

    UIVertex &v5 = targetVertices[ 5U ];
    v5._uv = IMAGE_TOP_LEFT;
    v5._uiPrimitiveType = PBR_UI_PRIMITIVE_TYPE_IMAGE;
    v5._color = color;
}

void UIPass::AppendRectangle ( GXVec2* targetPositions,
    UIVertex* targetVertices,
    GXColorUNORM color,
    GXVec2 const &topLeft,
    GXVec2 const &bottomRight
) noexcept
{
    targetPositions[ 0U ] = topLeft;

    UIVertex &v0 = targetVertices[ 0U ];
    v0._uiPrimitiveType = PBR_UI_PRIMITIVE_TYPE_GEOMETRY;
    v0._color = color;

    targetPositions[ 1U ] = GXVec2 ( bottomRight._data[ 0U ], topLeft._data[ 1U ] );

    UIVertex &v1 = targetVertices[ 1U ];
    v1._uiPrimitiveType = PBR_UI_PRIMITIVE_TYPE_GEOMETRY;
    v1._color = color;

    targetPositions[ 2U ] = bottomRight;

    UIVertex &v2 = targetVertices[ 2U ];
    v2._uiPrimitiveType = PBR_UI_PRIMITIVE_TYPE_GEOMETRY;
    v2._color = color;

    targetPositions[ 3U ] = bottomRight;

    UIVertex &v3 = targetVertices[ 3U ];
    v3._uiPrimitiveType = PBR_UI_PRIMITIVE_TYPE_GEOMETRY;
    v3._color = color;

    targetPositions[ 4U ] = GXVec2 ( topLeft._data[ 0U ], bottomRight._data[ 1U ] );

    UIVertex &v4 = targetVertices[ 4U ];
    v4._uiPrimitiveType = PBR_UI_PRIMITIVE_TYPE_GEOMETRY;
    v4._color = color;

    targetPositions[ 5U ] = topLeft;

    UIVertex &v5 = targetVertices[ 5U ];
    v5._uiPrimitiveType = PBR_UI_PRIMITIVE_TYPE_GEOMETRY;
    v5._color = color;
}

void UIPass::AppendText ( GXVec2* targetPositions,
    UIVertex* targetVertices,
    GXColorUNORM color,
    GXVec2 const &topLeft,
    GXVec2 const &bottomRight,
    android_vulkan::Half2 const &glyphTopLeft,
    android_vulkan::Half2 const &glyphBottomRight,
    uint8_t atlasLayer
) noexcept
{
    targetPositions[ 0U ] = topLeft;

    UIVertex &v0 = targetVertices[ 0U ];
    v0._uv = glyphTopLeft;
    v0._atlasLayer = atlasLayer;
    v0._uiPrimitiveType = PBR_UI_PRIMITIVE_TYPE_TEXT;
    v0._color = color;

    targetPositions[ 1U ] = GXVec2 ( bottomRight._data[ 0U ], topLeft._data[ 1U ] );

    UIVertex &v1 = targetVertices[ 1U ];

    v1._uv = android_vulkan::Half2 ( glyphBottomRight._data[ 0U ], glyphTopLeft._data[ 1U ] );
    v1._atlasLayer = atlasLayer;
    v1._uiPrimitiveType = PBR_UI_PRIMITIVE_TYPE_TEXT;
    v1._color = color;

    targetPositions[ 2U ] = bottomRight;

    UIVertex &v2 = targetVertices[ 2U ];
    v2._uv = glyphBottomRight;
    v2._atlasLayer = atlasLayer;
    v2._uiPrimitiveType = PBR_UI_PRIMITIVE_TYPE_TEXT;
    v2._color = color;

    targetPositions[ 3U ] = bottomRight;

    UIVertex &v3 = targetVertices[ 3U ];
    v3._uv = glyphBottomRight;
    v3._atlasLayer = atlasLayer;
    v3._uiPrimitiveType = PBR_UI_PRIMITIVE_TYPE_TEXT;
    v3._color = color;

    targetPositions[ 4U ] = GXVec2 ( topLeft._data[ 0U ], bottomRight._data[ 1U ] );

    UIVertex &v4 = targetVertices[ 4U ];

    v4._uv = android_vulkan::Half2 ( glyphTopLeft._data[ 0U ], glyphBottomRight._data[ 1U ] );
    v4._atlasLayer = atlasLayer;
    v4._uiPrimitiveType = PBR_UI_PRIMITIVE_TYPE_TEXT;
    v4._color = color;

    targetPositions[ 5U ] = topLeft;

    UIVertex &v5 = targetVertices[ 5U ];
    v5._uv = glyphTopLeft;
    v5._atlasLayer = atlasLayer;
    v5._uiPrimitiveType = PBR_UI_PRIMITIVE_TYPE_TEXT;
    v5._color = color;
}

void UIPass::ReleaseImage ( Texture2DRef const &image ) noexcept
{
    ImageStorage::ReleaseImage ( image );
}

std::optional<Texture2DRef const> UIPass::RequestImage ( std::string const &asset ) noexcept
{
    return ImageStorage::GetImage ( asset );
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

    Job &last = _jobs.back ();

    if ( !last._texture )
    {
        last._vertices += static_cast<uint32_t> ( usedVertices );
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
    _positions.UpdateGeometry ( commandBuffer, _readVertexIndex, _writeVertexIndex );
    _rest.UpdateGeometry ( commandBuffer, _readVertexIndex, _writeVertexIndex );
    _hasChanges = false;
}

void UIPass::UpdateTransform ( android_vulkan::Renderer &renderer, VkCommandBuffer commandBuffer ) noexcept
{
    float const scaleX = 2.0F / _bottomRight._data[ 0U ];
    float const scaleY = 2.0F / _bottomRight._data[ 1U ];
    GXMat4 const &orientation = renderer.GetPresentationEngineTransform ();

    UIProgram::Transform transform {};
    transform._rotateScaleRow0.Multiply ( *reinterpret_cast<GXVec2 const*> ( orientation._data[ 0U ] ), scaleX );
    transform._rotateScaleRow1.Multiply ( *reinterpret_cast<GXVec2 const*> ( orientation._data[ 1U ] ), scaleY );
    transform._offset.Multiply ( _bottomRight, -0.5F );

    _uniformPool.Push ( commandBuffer, &transform, sizeof ( transform ) );
    _transformDescriptorSet = _uniformPool.Acquire ();

    _uniformPool.IssueSync ( renderer.GetDevice (), commandBuffer );
    _uniformPool.Commit ();

    _isTransformChanged = false;
}

} // namespace pbr::windows
