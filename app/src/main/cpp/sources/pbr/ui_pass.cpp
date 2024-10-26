#include <precompiled_headers.hpp>
#include <pbr/ui_pass.hpp>
#include <pbr/ui_program.inc>
#include <av_assert.hpp>
#include <logger.hpp>
#include <trace.hpp>


namespace pbr {

namespace {

constexpr size_t ALLOCATE_COMMAND_BUFFERS = 8U;
constexpr size_t COMMAND_BUFFERS_PER_TEXTURE = 1U;
constexpr size_t INITIAL_COMMAND_BUFFERS = 32U;

constexpr size_t MAX_IMAGES = 1024U;
constexpr size_t MAX_VERTICES = 762600U;
constexpr size_t BUFFER_BYTES = MAX_VERTICES * sizeof ( UIVertexInfo );

constexpr size_t TRANSPARENT_DESCRIPTOR_SET_COUNT = 1U;

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

    bool const result = texture->UploadData ( *_renderer,
        asset,
        android_vulkan::eFormat::sRGB,
        true,
        _commandBuffers[ _commandBufferIndex ],
        _fences[ _commandBufferIndex ]
    );

    if ( !result )
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
        "pbr::ImageStorage::OnInitDevice",
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
        android_vulkan::LogWarning ( "pbr::ImageStorage::OnDestroyDevice - Memory leak." );
        AV_ASSERT ( false )
    }

    _textures.clear ();

    VkDevice device = _renderer->GetDevice ();

    if ( _commandPool != VK_NULL_HANDLE )
    {
        vkDestroyCommandPool ( device, _commandPool, nullptr );
        _commandPool = VK_NULL_HANDLE;
    }

    auto const clean = [] ( auto &v ) noexcept {
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
        "pbr::ImageStorage::SyncGPU",
        "Can't wait fence"
    );

    if ( !result ) [[unlikely]]
        return false;

    result = android_vulkan::Renderer::CheckVkResult ( vkResetFences ( device, fenceCount, fences ),
        "pbr::ImageStorage::SyncGPU",
        "Can't reset fence"
    );

    if ( !result ) [[unlikely]]
        return false;

    result = android_vulkan::Renderer::CheckVkResult (
        vkResetCommandPool ( device, _commandPool, 0U ),
        "pbr::ImageStorage::SyncGPU",
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
        "pbr::ImageStorage::AllocateCommandBuffers",
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
            "pbr::ImageStorage::AllocateCommandBuffers",
            "Can't create fence"
        );

        if ( !result ) [[unlikely]]
            return false;

        AV_SET_VULKAN_OBJECT_NAME ( device, fences[ i ], VK_OBJECT_TYPE_FENCE, "UI #%zu", i )
    }

#if defined ( ANDROID_VULKAN_ENABLE_VULKAN_VALIDATION_LAYERS ) ||       \
    defined ( ANDROID_VULKAN_ENABLE_RENDER_DOC_INTEGRATION )

    VkCommandBuffer* const buffers = _commandBuffers.data ();

    for ( size_t i = current; i < size; ++i )
        AV_SET_VULKAN_OBJECT_NAME ( device, buffers[ i ], VK_OBJECT_TYPE_COMMAND_BUFFER, "UI #%zu", i )

#endif // ANDROID_VULKAN_ENABLE_VULKAN_VALIDATION_LAYERS || ANDROID_VULKAN_ENABLE_RENDER_DOC_INTEGRATION

    return true;
}

} // end of anonymous namespace

//----------------------------------------------------------------------------------------------------------------------

bool UIPass::CommonDescriptorSet::Init ( VkDevice device,
    VkDescriptorPool descriptorPool,
    SamplerManager const &samplerManager
) noexcept
{
    if ( !_layout.Init ( device ) ) [[unlikely]]
        return false;

    VkDescriptorSetAllocateInfo const allocateInfo
    {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
        .pNext = nullptr,
        .descriptorPool = descriptorPool,
        .descriptorSetCount = 1U,
        .pSetLayouts = &_layout.GetLayout ()
    };

    bool const result = android_vulkan::Renderer::CheckVkResult (
        vkAllocateDescriptorSets ( device, &allocateInfo, &_descriptorSet ),
        "pbr::UIPass::CommonDescriptorSet::Init",
        "Can't allocate descriptor sets"
    );

    if ( !result ) [[unlikely]]
        return false;

    AV_SET_VULKAN_OBJECT_NAME ( device, _descriptorSet, VK_OBJECT_TYPE_DESCRIPTOR_SET, "UI common" )

    VkDescriptorImageInfo const imageInfo[] =
    {
        {
            .sampler = samplerManager.GetPointSampler (),
            .imageView = VK_NULL_HANDLE,
            .imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
        },
        {
            .sampler = samplerManager.GetMaterialSampler (),
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
    }

    if ( _memory == VK_NULL_HANDLE )
        return;

    renderer.FreeMemory ( _memory, _memoryOffset );
    _memory = VK_NULL_HANDLE;
    _memoryOffset = 0U;
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
        "pbr::UIPass::ImageDescriptorSets::Init",
        "Can't allocate descriptor sets"
    );

    if ( !result ) [[unlikely]]
        return false;

#if defined ( ANDROID_VULKAN_ENABLE_VULKAN_VALIDATION_LAYERS ) ||       \
    defined ( ANDROID_VULKAN_ENABLE_RENDER_DOC_INTEGRATION )

    for ( size_t i = 0U; i < count; ++i )
        AV_SET_VULKAN_OBJECT_NAME ( device, ds[ i ], VK_OBJECT_TYPE_DESCRIPTOR_SET, "UI image #%zu", i )

#endif // ANDROID_VULKAN_ENABLE_VULKAN_VALIDATION_LAYERS || ANDROID_VULKAN_ENABLE_RENDER_DOC_INTEGRATION

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

    auto const clear = [] ( auto &v ) noexcept {
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

    constexpr VkDeviceSize offset = 0U;
    vkCmdBindVertexBuffers ( commandBuffer, 0U, 1U, &_vertex._buffer, &offset );

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
    if ( !_fontStorage.Init () ) [[unlikely]]
        return false;

    constexpr auto stagingProps = static_cast<VkMemoryPropertyFlags> (
        AV_VK_FLAG ( VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT ) | AV_VK_FLAG ( VK_MEMORY_PROPERTY_HOST_COHERENT_BIT )
    );

    if ( !_staging.Init ( renderer, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, stagingProps, "UI pass staging" ) )
    {
        [[unlikely]]
        return false;
    }

    void* data;

    bool result = renderer.MapMemory ( data,
        _staging._memory,
        _staging._memoryOffset,
        "pbr::UIPass::OnInitDevice",
        "Can't map memory"
    );

    if ( !result ) [[unlikely]]
        return false;

    _data = static_cast<UIVertexInfo*> ( data );

    result = _vertex.Init ( renderer,
        AV_VK_FLAG ( VK_BUFFER_USAGE_TRANSFER_DST_BIT ) | AV_VK_FLAG ( VK_BUFFER_USAGE_VERTEX_BUFFER_BIT ),
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
        "UI pass"
    );

    if ( !result ) [[unlikely]]
        return false;

    _bufferBarrier =
    {
        .sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER,
        .pNext = nullptr,
        .srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT,
        .dstAccessMask = VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT,
        .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
        .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
        .buffer = _vertex._buffer,
        .offset = 0U,
        .size = 0U
    };

    constexpr size_t commonDescriptorSetCount = 1U;
    constexpr size_t descriptorSetCount = commonDescriptorSetCount + MAX_IMAGES + TRANSPARENT_DESCRIPTOR_SET_COUNT;

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

    VkDevice device = renderer.GetDevice ();

    result = android_vulkan::Renderer::CheckVkResult (
        vkCreateDescriptorPool ( device, &poolInfo, nullptr, &_descriptorPool ),
        "pbr::UIPass::OnInitDevice",
        "Can't create descriptor pool"
    );

    if ( !result ) [[unlikely]]
        return false;

    AV_SET_VULKAN_OBJECT_NAME ( device, _descriptorPool, VK_OBJECT_TYPE_DESCRIPTOR_POOL, "UI pass" )

    return _commonDescriptorSet.Init ( device, _descriptorPool, samplerManager ) &&
        _imageDescriptorSets.Init ( device, _descriptorPool, transparent ) &&
        _transformLayout.Init ( device ) &&
        ImageStorage::OnInitDevice ( renderer ) &&
        _uniformPool.Init ( renderer, _transformLayout, sizeof ( UIProgram::Transform ), BIND_TRANSFORM, "UI pass" );
}

void UIPass::OnDestroyDevice ( android_vulkan::Renderer &renderer ) noexcept
{
    _uniformPool.Destroy ( renderer );

    VkDevice device = renderer.GetDevice ();
    _imageDescriptorSets.Destroy ( device );
    _commonDescriptorSet.Destroy ( device );
    _transformLayout.Destroy ( device );

    if ( _descriptorPool != VK_NULL_HANDLE ) [[likely]]
    {
        vkDestroyDescriptorPool ( renderer.GetDevice (), _descriptorPool, nullptr );
        _descriptorPool = VK_NULL_HANDLE;
    }

    _program.Destroy ( device );

    if ( _data )
    {
        renderer.UnmapMemory ( _staging._memory );
        _data = nullptr;
    }

    _inUseImageTracker.Destroy ();
    _staging.Destroy ( renderer );
    _vertex.Destroy ( renderer );
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

    bool const result = _fontStorage.SetMediaResolution ( renderer, resolution ) &&
        _program.Init ( renderer,
            renderPass,
            subpass,
            BrightnessProgram::GetBrightnessInfo ( _brightnessBalance ),
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

    if ( neededVertices > MAX_VERTICES )
    {
        android_vulkan::LogWarning ( "pbr::UIPass::RequestUIBuffer - Too many vertices was requested: %zu + %zu.",
            neededVertices,
            GetVerticesPerRectangle ()
        );

        AV_ASSERT ( false )
        return std::nullopt;
    }

    size_t const cases[] = { 0U, _writeVertexIndex };
    size_t const nextIdx = cases[ _writeVertexIndex + neededVertices <= MAX_VERTICES ];

    _readVertexIndex = nextIdx;
    UIVertexBuffer const result = UIVertexBuffer ( _data + nextIdx, neededVertices );

    _writeVertexIndex = nextIdx + neededVertices;
    return result;
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
        BrightnessProgram::GetBrightnessInfo ( brightnessBalance ),
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

    if ( !_fontStorage.UploadGPUData ( renderer, commandBuffer, framebufferIndex ) ) [[unlikely]]
        return false;

    _commonDescriptorSet.Update ( device, _fontStorage.GetAtlasImageView () );

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
    constexpr size_t const elementSize = sizeof ( UIVertexInfo );
    auto const offset = static_cast<VkDeviceSize> ( elementSize * _readVertexIndex );
    auto const size = static_cast<VkDeviceSize> ( elementSize * ( _writeVertexIndex - _readVertexIndex ) );

    VkBufferCopy const copy
    {
        .srcOffset = offset,
        .dstOffset = offset,
        .size = size
    };

    vkCmdCopyBuffer ( commandBuffer, _staging._buffer, _vertex._buffer, 1U, &copy );

    _bufferBarrier.offset = offset;
    _bufferBarrier.size = size;

    vkCmdPipelineBarrier ( commandBuffer,
        VK_PIPELINE_STAGE_TRANSFER_BIT,
        VK_PIPELINE_STAGE_VERTEX_INPUT_BIT,
        0U,
        0U,
        nullptr,
        1U,
        &_bufferBarrier,
        0U,
        nullptr
    );

    _hasChanges = false;
}

void UIPass::UpdateTransform ( android_vulkan::Renderer &renderer, VkCommandBuffer commandBuffer ) noexcept
{
    float const scaleX = 2.0F / _bottomRight._data[ 0U ];
    float const scaleY = 2.0F / _bottomRight._data[ 1U ];
    GXMat4 const &orientation = renderer.GetPresentationEngineTransform ();

    UIProgram::Transform transform {};
    transform._rotateScaleRow0.Multiply ( *reinterpret_cast<GXVec2 const*> ( &orientation._m[ 0U ][ 0U ] ), scaleX );
    transform._rotateScaleRow1.Multiply ( *reinterpret_cast<GXVec2 const*> ( &orientation._m[ 1U ][ 0U ] ), scaleY );
    transform._offset.Multiply ( _bottomRight, -0.5F );

    _uniformPool.Push ( commandBuffer, &transform, sizeof ( transform ) );
    _transformDescriptorSet = _uniformPool.Acquire ();

    _uniformPool.IssueSync ( renderer.GetDevice (), commandBuffer );
    _uniformPool.Commit ();

    _isTransformChanged = false;
}

} // namespace pbr
