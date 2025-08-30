#include <precompiled_headers.hpp>
#include <av_assert.hpp>
#include <logger.hpp>
#include <platform/windows/pbr/ui_pass.hpp>
#include <trace.hpp>
#include <vulkan_utils.hpp>


// FUCK - remove namespace
namespace pbr::windows {

namespace {

constexpr size_t ALLOCATE_COMMAND_BUFFERS = 8U;
constexpr size_t COMMAND_BUFFERS_PER_TEXTURE = 1U;
constexpr size_t INITIAL_COMMAND_BUFFERS = 32U;

constexpr size_t MAX_VERTICES = 762600U;

constexpr std::string_view TEXT_LUT = "pbr/system/text-lut.png";

android_vulkan::Half2 const IMAGE_TOP_LEFT ( 0.0F, 0.0F );
android_vulkan::Half2 const IMAGE_BOTTOM_RIGHT ( 1.0F, 1.0F );

//----------------------------------------------------------------------------------------------------------------------

class ImageStorage final
{
    private:
        class Asset final
        {
            public:
                android_vulkan::Texture2D                   _texture {};
                size_t                                      _refs = 1U;
                uint16_t                                    _image = ResourceHeap::INVALID_UI_IMAGE;

            public:
                Asset () = default;

                Asset ( Asset const & ) = delete;
                Asset &operator = ( Asset const & ) = delete;

                Asset ( Asset && ) = default;
                Asset &operator = ( Asset && ) = default;

                ~Asset () = default;
        };

    private:
        static size_t                                           _commandBufferIndex;
        static std::vector<VkCommandBuffer>                     _commandBuffers;
        static VkCommandPool                                    _commandPool;
        static std::vector<VkFence>                             _fences;
        static android_vulkan::Renderer*                        _renderer;
        static ResourceHeap*                                    _resourceHeap;
        static std::unordered_map<std::string_view, Asset*>     _assetMap;
        static std::unordered_map<uint16_t, Asset>              _assets;
        static std::vector<Asset>                               _brokenAssets;

    public:
        ImageStorage () = delete;

        ImageStorage ( ImageStorage const & ) = delete;
        ImageStorage &operator = ( ImageStorage const & ) = delete;

        ImageStorage ( ImageStorage && ) = delete;
        ImageStorage &operator = ( ImageStorage && ) = delete;

        ~ImageStorage () = delete;

        static void ReleaseImage ( uint16_t image ) noexcept;
        [[nodiscard]] static std::optional<uint16_t> GetImage ( std::string const &asset ) noexcept;
        [[nodiscard]] static std::optional<uint16_t> GetImage ( std::string_view asset ) noexcept;

        [[nodiscard]] static bool OnInitDevice ( android_vulkan::Renderer &renderer ) noexcept;
        static void OnDestroyDevice () noexcept;

        static void SetResourceHeap ( ResourceHeap &resourceHeap ) noexcept;
        [[nodiscard]] static bool SyncGPU () noexcept;

    private:
        [[nodiscard]] static bool AllocateCommandBuffers ( size_t amount ) noexcept;
};

size_t ImageStorage::_commandBufferIndex = 0U;
std::vector<VkCommandBuffer> ImageStorage::_commandBuffers {};
VkCommandPool ImageStorage::_commandPool = VK_NULL_HANDLE;
std::vector<VkFence> ImageStorage::_fences {};
android_vulkan::Renderer* ImageStorage::_renderer = nullptr;
ResourceHeap* ImageStorage::_resourceHeap = nullptr;
std::unordered_map<std::string_view, ImageStorage::Asset*> ImageStorage::_assetMap {};
std::unordered_map<uint16_t, ImageStorage::Asset> ImageStorage::_assets {};
std::vector<ImageStorage::Asset> ImageStorage::_brokenAssets {};

void ImageStorage::ReleaseImage ( uint16_t image ) noexcept
{
    auto const findResult = _assets.find ( image );
    Asset &asset = findResult->second;

    if ( --asset._refs > 0U )
        return;

    asset._texture.FreeResources ( *_renderer );
    auto const end = _assetMap.cend ();

    for ( auto i = _assetMap.cbegin (); i != end; ++i )
    {
        if ( i->second != &asset )
            continue;

        _assetMap.erase ( i );
        break;
    }

    _resourceHeap->UnregisterResource ( asset._image );
    _assets.erase ( findResult );
}

std::optional<uint16_t> ImageStorage::GetImage ( std::string const &asset ) noexcept
{
    return GetImage ( std::string_view ( asset ) );
}

std::optional<uint16_t> ImageStorage::GetImage ( std::string_view asset ) noexcept
{
    if ( _commandBuffers.size () - _commandBufferIndex < COMMAND_BUFFERS_PER_TEXTURE )
    {
        if ( !AllocateCommandBuffers ( ALLOCATE_COMMAND_BUFFERS ) )
        {
            return std::nullopt;
        }
    }

    if ( auto findResult = _assetMap.find ( asset ); findResult != _assetMap.cend () )
    {
        Asset &ast = *findResult->second;
        ++ast._refs;
        return std::optional<uint16_t> { ast._image };
    }

    Asset ast {};

    // Note UNORM is correct mode because of pixel shader, alpha blending and swapchain UNORM format.
    bool const result = ast._texture.UploadData ( *_renderer,
        asset,
        android_vulkan::eColorSpace::Unorm,
        true,
        _commandBuffers[ _commandBufferIndex ],
        _fences[ _commandBufferIndex ]
    );

    if ( !result ) [[unlikely]]
        return std::nullopt;

    _commandBufferIndex += COMMAND_BUFFERS_PER_TEXTURE;
    auto const image = _resourceHeap->RegisterUISampledImage ( _renderer->GetDevice (), ast._texture.GetImageView () );

    if ( !image ) [[likely]]
    {
        // Note texture is not deleted because it's too late. Image data is uploading via command buffer.
        _brokenAssets.push_back ( std::move ( ast ) );
        return std::nullopt;
    }

    auto const img = static_cast<uint16_t> ( *image );
    Asset &a = _assets[ img ];
    a = std::move ( ast );
    _assetMap[ a._texture.GetName () ] = &a;
    return std::optional<uint16_t> { img };
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

void ImageStorage::OnDestroyDevice ( ) noexcept
{
    if ( !_assets.empty () ) [[unlikely]]
    {
        // FUCK - remove namespace
        android_vulkan::LogWarning ( "pbr::windows::ImageStorage::OnDestroyDevice - Memory leak." );
        AV_ASSERT ( false )
    }

    _assets.clear ();
    VkDevice device = _renderer->GetDevice ();

    if ( _commandPool != VK_NULL_HANDLE ) [[likely]]
        vkDestroyCommandPool ( device, std::exchange ( _commandPool, VK_NULL_HANDLE ), nullptr );

    constexpr auto clean = [] ( auto &v ) noexcept {
        v.clear ();
        v.shrink_to_fit ();
    };

    clean ( _commandBuffers );

    for ( auto fence : _fences )
        vkDestroyFence ( device, fence, nullptr );

    clean ( _fences );

    for ( auto &asset : _brokenAssets )
        asset._texture.FreeResources ( *_renderer );

    clean ( _brokenAssets );
    _renderer = nullptr;
}

void ImageStorage::SetResourceHeap ( ResourceHeap &resourceHeap ) noexcept
{
    _resourceHeap = &resourceHeap;
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

    // FUCK - setup BDA
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

VkDeviceAddress UIPass::BufferStream::GetBufferAddress () const noexcept
{
    return _bda;
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

void UIPass::InUseImageTracker::MarkInUse ( uint16_t image, size_t commandBufferIndex ) noexcept
{
    _registry[ commandBufferIndex ][ image ] = 2U;
}

//----------------------------------------------------------------------------------------------------------------------

UIPass::UIPass ( ResourceHeap &resourceHeap ) noexcept:
    _fontStorage ( resourceHeap )
{
    ImageStorage::SetResourceHeap ( resourceHeap );
}

bool UIPass::Execute ( VkCommandBuffer commandBuffer, size_t commandBufferIndex ) noexcept
{
    AV_TRACE ( "UI pass: Execute" )
    AV_VULKAN_GROUP ( commandBuffer, "UI" )

    if ( !ImageStorage::SyncGPU () ) [[unlikely]]
        return false;

    if ( !_vertices ) [[unlikely]]
    {
        _inUseImageTracker.CollectGarbage ( commandBufferIndex );
        return true;
    }

    _program.Bind ( commandBuffer );

    auto const start = static_cast<VkDeviceAddress> ( _readVertexIndex );
    _uiInfo._positionBDA = _positions.GetBufferAddress () + start * sizeof ( GXVec2 );
    _uiInfo._restBDA = _rest.GetBufferAddress () + start * sizeof ( UIVertex );

    _program.SetPushConstants ( commandBuffer, &_uiInfo );

    // FUCK - set descriptor buffer
    // FUCK - _inUseImageTracker.MarkInUse ( job._image, commandBufferIndex );

    vkCmdDraw ( commandBuffer, _vertices, 1U, 0U, 0U );

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

bool UIPass::OnInitDevice ( android_vulkan::Renderer &renderer ) noexcept
{
    bool const status = _fontStorage.Init ( renderer ) &&
        _positions.Init ( renderer, "UI positions", "UI position staging" ) &&
        _rest.Init ( renderer, "UI rest", "UI rest staging" ) &&
        ImageStorage::OnInitDevice ( renderer );

    if ( !status ) [[unlikely]]
        return false;

    auto const probe = ImageStorage::GetImage ( std::string ( TEXT_LUT ) );

    if ( !probe ) [[unlikely]]
        return false;

    _textLUT = static_cast<uint16_t> ( *probe );
    return true;
}

void UIPass::OnDestroyDevice ( android_vulkan::Renderer &renderer ) noexcept
{
    VkDevice device = renderer.GetDevice ();
    _program.Destroy ( device );

    _positions.Destroy ( renderer );
    _rest.Destroy ( renderer );

    _inUseImageTracker.Destroy ();
    _fontStorage.Destroy ( renderer );

    _writeVertexIndex = 0U;
    _readVertexIndex = 0U;

    _hasChanges = false;

    if ( _textLUT != ResourceHeap::INVALID_UI_IMAGE ) [[likely]]
        ImageStorage::ReleaseImage ( std::exchange ( _textLUT, ResourceHeap::INVALID_UI_IMAGE ) );

    ImageStorage::OnDestroyDevice ();
}

bool UIPass::OnSwapchainCreated ( android_vulkan::Renderer &renderer ) noexcept
{
    VkExtent2D const &resolution = renderer.GetSurfaceSize ();
    VkExtent2D &r = _currentResolution;

    if ( ( r.width == resolution.width ) & ( r.height == resolution.height ) ) [[unlikely]]
        return true;

    r = resolution;
    VkExtent2D const &viewport = renderer.GetViewportResolution ();
    _bottomRight = GXVec2 ( static_cast<float> ( viewport.width ), static_cast<float> ( viewport.height ) );
    _isTransformChanged = true;

    return SetBrightness ( renderer, _brightnessBalance );
}

void UIPass::OnSwapchainDestroyed () noexcept
{
    _inUseImageTracker.Destroy ();
}

void UIPass::RequestEmptyUI () noexcept
{
    _hasChanges = _hasChanges | ( std::exchange ( _vertices, 0U ) > 0U );
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

    return
    {
        {
            ._positions { static_cast<GXVec2*> ( _positions.GetData ( nextIdx ) ), neededVertices },
            ._vertices { static_cast<UIVertex*> ( _rest.GetData ( nextIdx ) ), neededVertices }
        }
    };
}

bool UIPass::SetBrightness ( android_vulkan::Renderer &renderer, float brightnessBalance ) noexcept
{
    _brightnessBalance = brightnessBalance;

    VkDevice device = renderer.GetDevice ();
    _program.Destroy ( device );

    return _program.Init ( device,
        renderer.GetSurfaceFormat (),
        BrightnessInfo ( brightnessBalance ),
        _currentResolution
    );
}

bool UIPass::UploadGPUData ( android_vulkan::Renderer &renderer, VkCommandBuffer commandBuffer ) noexcept
{
    AV_TRACE ( "UI pass: Upload GPU data" )
    AV_VULKAN_GROUP ( commandBuffer, "Upload UI data" )

    if ( !_fontStorage.UploadGPUData ( renderer, commandBuffer ) ) [[unlikely]]
        return false;

    if ( _isTransformChanged ) [[unlikely]]
        UpdateTransform ( renderer );

    if ( _hasChanges )
        UpdateGeometry ( commandBuffer );

    return true;
}

void UIPass::AppendImage ( GXVec2* targetPositions,
    UIVertex* targetVertices,
    GXColorUNORM color,
    GXVec2 const &topLeft,
    GXVec2 const &bottomRight,
    uint16_t image
) noexcept
{
    targetPositions[ 0U ] = topLeft;

    targetVertices[ 0U ] =
    {
        ._uv = IMAGE_TOP_LEFT,
        ._color = color,
        ._image = image,
        ._uiPrimitiveType = PBR_UI_PRIMITIVE_TYPE_IMAGE
    };

    targetPositions[ 1U ] = GXVec2 ( bottomRight._data[ 0U ], topLeft._data[ 1U ] );

    targetVertices[ 1U ] =
    {
        ._uv = android_vulkan::Half2 ( IMAGE_BOTTOM_RIGHT._data[ 0U ], IMAGE_TOP_LEFT._data[ 1U ] ),
        ._color = color,
        ._image = image,
        ._uiPrimitiveType = PBR_UI_PRIMITIVE_TYPE_IMAGE
    };

    targetPositions[ 2U ] = bottomRight;

    targetVertices[ 2U ] =
    {
        ._uv = IMAGE_BOTTOM_RIGHT,
        ._color = color,
        ._image = image,
        ._uiPrimitiveType = PBR_UI_PRIMITIVE_TYPE_IMAGE
    };

    targetPositions[ 3U ] = bottomRight;

    targetVertices[ 3U ] =
    {
        ._uv = IMAGE_BOTTOM_RIGHT,
        ._color = color,
        ._image = image,
        ._uiPrimitiveType = PBR_UI_PRIMITIVE_TYPE_IMAGE
    };

    targetPositions[ 4U ] = GXVec2 ( topLeft._data[ 0U ], bottomRight._data[ 1U ] );

    targetVertices[ 4U ] =
    {
        ._uv = android_vulkan::Half2 ( IMAGE_TOP_LEFT._data[ 0U ], IMAGE_BOTTOM_RIGHT._data[ 1U ] ),
        ._color = color,
        ._image = image,
        ._uiPrimitiveType = PBR_UI_PRIMITIVE_TYPE_IMAGE
    };

    targetPositions[ 5U ] = topLeft;

    targetVertices[ 5U ] =
    {
        ._uv = IMAGE_TOP_LEFT,
        ._color = color,
        ._image = image,
        ._uiPrimitiveType = PBR_UI_PRIMITIVE_TYPE_IMAGE
    };
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
    v0._color = color;
    v0._uiPrimitiveType = PBR_UI_PRIMITIVE_TYPE_GEOMETRY;

    targetPositions[ 1U ] = GXVec2 ( bottomRight._data[ 0U ], topLeft._data[ 1U ] );

    UIVertex &v1 = targetVertices[ 1U ];
    v1._color = color;
    v1._uiPrimitiveType = PBR_UI_PRIMITIVE_TYPE_GEOMETRY;

    targetPositions[ 2U ] = bottomRight;

    UIVertex &v2 = targetVertices[ 2U ];
    v2._color = color;
    v2._uiPrimitiveType = PBR_UI_PRIMITIVE_TYPE_GEOMETRY;

    targetPositions[ 3U ] = bottomRight;

    UIVertex &v3 = targetVertices[ 3U ];
    v3._color = color;
    v3._uiPrimitiveType = PBR_UI_PRIMITIVE_TYPE_GEOMETRY;

    targetPositions[ 4U ] = GXVec2 ( topLeft._data[ 0U ], bottomRight._data[ 1U ] );

    UIVertex &v4 = targetVertices[ 4U ];
    v4._color = color;
    v4._uiPrimitiveType = PBR_UI_PRIMITIVE_TYPE_GEOMETRY;

    targetPositions[ 5U ] = topLeft;

    UIVertex &v5 = targetVertices[ 5U ];
    v5._color = color;
    v5._uiPrimitiveType = PBR_UI_PRIMITIVE_TYPE_GEOMETRY;
}

void UIPass::AppendText ( GXVec2* targetPositions,
    UIVertex* targetVertices,
    GXColorUNORM color,
    GXVec2 const &topLeft,
    GXVec2 const &bottomRight,
    android_vulkan::Half2 const &glyphTopLeft,
    android_vulkan::Half2 const &glyphBottomRight,
    uint16_t atlas
) noexcept
{
    targetPositions[ 0U ] = topLeft;

    targetVertices[ 0U ] =
    {
        ._uv = glyphTopLeft,
        ._color = color,
        ._image = atlas,
        ._uiPrimitiveType = PBR_UI_PRIMITIVE_TYPE_TEXT
    };

    targetPositions[ 1U ] = GXVec2 ( bottomRight._data[ 0U ], topLeft._data[ 1U ] );

    targetVertices[ 1U ] =
    {
        ._uv = android_vulkan::Half2 ( glyphBottomRight._data[ 0U ], glyphTopLeft._data[ 1U ] ),
        ._color = color,
        ._image = atlas,
        ._uiPrimitiveType = PBR_UI_PRIMITIVE_TYPE_TEXT,
    };

    targetPositions[ 2U ] = bottomRight;

    targetVertices[ 2U ] =
    {
        ._uv = glyphBottomRight,
        ._color = color,
        ._image = atlas,
        ._uiPrimitiveType = PBR_UI_PRIMITIVE_TYPE_TEXT
    };

    targetPositions[ 3U ] = bottomRight;

    targetVertices[ 3U ] =
    {
        ._uv = glyphBottomRight,
        ._color = color,
        ._image = atlas,
        ._uiPrimitiveType = PBR_UI_PRIMITIVE_TYPE_TEXT
    };

    targetPositions[ 4U ] = GXVec2 ( topLeft._data[ 0U ], bottomRight._data[ 1U ] );

    targetVertices[ 4U ] =
    {
        ._uv = android_vulkan::Half2 ( glyphTopLeft._data[ 0U ], glyphBottomRight._data[ 1U ] ),
        ._color = color,
        ._image = atlas,
        ._uiPrimitiveType = PBR_UI_PRIMITIVE_TYPE_TEXT
    };

    targetPositions[ 5U ] = topLeft;

    targetVertices[ 5U ] =
    {
        ._uv = glyphTopLeft,
        ._color = color,
        ._image = atlas,
        ._uiPrimitiveType = PBR_UI_PRIMITIVE_TYPE_TEXT
    };
}

void UIPass::ReleaseImage ( uint16_t image ) noexcept
{
    ImageStorage::ReleaseImage ( image );
}

std::optional<uint16_t> UIPass::RequestImage ( std::string const &asset ) noexcept
{
    return ImageStorage::GetImage ( asset );
}

void UIPass::UpdateGeometry ( VkCommandBuffer commandBuffer ) noexcept
{
    _positions.UpdateGeometry ( commandBuffer, _readVertexIndex, _writeVertexIndex );
    _rest.UpdateGeometry ( commandBuffer, _readVertexIndex, _writeVertexIndex );
    _hasChanges = false;
}

void UIPass::UpdateTransform ( android_vulkan::Renderer &renderer ) noexcept
{
    float const scaleX = 2.0F / _bottomRight._data[ 0U ];
    float const scaleY = 2.0F / _bottomRight._data[ 1U ];
    GXMat4 const &orientation = renderer.GetPresentationEngineTransform ();

    _uiInfo._rotateScaleRow0.Multiply ( *reinterpret_cast<GXVec2 const*> ( orientation._data[ 0U ] ), scaleX );
    _uiInfo._rotateScaleRow1.Multiply ( *reinterpret_cast<GXVec2 const*> ( orientation._data[ 1U ] ), scaleY );
    _uiInfo._offset.Multiply ( _bottomRight, -0.5F );

    _isTransformChanged = false;
}

} // namespace pbr::windows
