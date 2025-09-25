#include <precompiled_headers.hpp>
#include <av_assert.hpp>
#include <logger.hpp>
#include <platform/windows/pbr/ui_pass.hpp>
#include <trace.hpp>


// FUCK - remove namespace
namespace pbr::windows {

namespace {

constexpr size_t ALLOCATE_COMMAND_BUFFERS = 8U;
constexpr size_t COMMAND_BUFFERS_PER_TEXTURE = 1U;
constexpr size_t INITIAL_COMMAND_BUFFERS = 32U;

constexpr size_t INITIAL_USED_IMAGE_CAPACITY = 128U;

constexpr size_t MAX_VERTICES = 762600U;

constexpr std::string_view TEXT_LUT = "pbr/system/text-lut.png";

android_vulkan::Half2 const IMAGE_TOP_LEFT ( 0.0F, 0.0F );
android_vulkan::Half2 const IMAGE_BOTTOM_RIGHT ( 1.0F, 1.0F );

constexpr size_t STREAM_1_OFFSET = MAX_VERTICES * sizeof ( UIVertexBufferStream0 );

//----------------------------------------------------------------------------------------------------------------------

class ImageStorage final
{
    private:
        class Asset final
        {
            public:
                android_vulkan::Texture2D                       _texture {};
                size_t                                          _refs = 1U;
                uint16_t                                        _image = ResourceHeap::INVALID_UI_IMAGE;

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

        [[nodiscard]] static std::optional<UIPass::Image> GetImage ( std::string const &asset, bool useMips ) noexcept;
        [[nodiscard]] static std::optional<UIPass::Image> GetImage ( std::string_view asset, bool useMips ) noexcept;

        [[nodiscard]] static bool OnInitDevice ( android_vulkan::Renderer &renderer ) noexcept;
        static void OnDestroyDevice () noexcept;

        static void SetResourceHeap ( ResourceHeap &resourceHeap ) noexcept;
        [[nodiscard]] static bool SyncGPU () noexcept;

#if defined ( AV_ENABLE_VVL ) || defined ( AV_ENABLE_RENDERDOC ) || defined ( AV_ENABLE_NSIGHT )

        static void SetName ( uint16_t image, char const* name ) noexcept;

#endif // AV_ENABLE_VVL || AV_ENABLE_RENDERDOC || AV_ENABLE_NSIGHT

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

    _resourceHeap->UnregisterResource ( image );
    _assets.erase ( findResult );
}

std::optional<UIPass::Image> ImageStorage::GetImage ( std::string const &asset, bool useMips ) noexcept
{
    return GetImage ( std::string_view ( asset ), useMips );
}

std::optional<UIPass::Image> ImageStorage::GetImage ( std::string_view asset, bool useMips ) noexcept
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

        return std::optional<UIPass::Image> {
            {
                ._image = ast._image,
                ._resolution = ast._texture.GetResolution ()
            }
        };
    }

    Asset ast {};

    // Note UNORM is correct mode because of pixel shader, alpha blending and swapchain UNORM format.
    bool const result = ast._texture.UploadData ( *_renderer,
        asset,
        android_vulkan::eColorSpace::Unorm,
        useMips,
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

    return std::optional<UIPass::Image> {
        {
            ._image = img,
            ._resolution = a._texture.GetResolution ()
        }
    };
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

#if defined ( AV_ENABLE_VVL ) || defined ( AV_ENABLE_RENDERDOC ) || defined ( AV_ENABLE_NSIGHT )

void ImageStorage::SetName ( uint16_t image, char const* name ) noexcept
{
    android_vulkan::Texture2D const &tex = _assets.find ( image )->second._texture;
    VkDevice device = _renderer->GetDevice ();
    AV_SET_VULKAN_OBJECT_NAME ( device, tex.GetImage (), VK_OBJECT_TYPE_IMAGE, "%s", name )
    AV_SET_VULKAN_OBJECT_NAME ( device, tex.GetImageView (), VK_OBJECT_TYPE_IMAGE_VIEW, "%s", name )
}

#endif // AV_ENABLE_VVL || AV_ENABLE_RENDERDOC || AV_ENABLE_NSIGHT

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

bool UIPass::BufferStream::Init ( android_vulkan::Renderer &renderer ) noexcept
{
    constexpr VkMemoryPropertyFlags stagingProps = AV_VK_FLAG ( VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT ) |
        AV_VK_FLAG ( VK_MEMORY_PROPERTY_HOST_COHERENT_BIT );

    constexpr VkBufferUsageFlags usage = AV_VK_FLAG ( VK_BUFFER_USAGE_TRANSFER_DST_BIT ) |
        AV_VK_FLAG ( VK_BUFFER_USAGE_STORAGE_BUFFER_BIT ) |
        AV_VK_FLAG ( VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT );

    static_assert ( sizeof ( UIVertexBufferStream1 ) % 2U == 0U );
    constexpr size_t size = MAX_VERTICES * ( sizeof ( UIVertexBufferStream0 ) + sizeof ( UIVertexBufferStream1 ) );

    bool const result =
        _staging.Init ( renderer, size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, stagingProps, "UI staging" ) &&

        renderer.MapMemory ( reinterpret_cast<void* &> ( _data ),
            _staging._memory,
            _staging._memoryOffset,

            // FUCK - remove namespace
            "pbr::windows::UIPass::BufferStream::Init",

            "Can't map memory"
        ) &&

        _gpuBuffer.Init ( renderer, size, usage, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, "UI vertices" );

    if ( !result ) [[unlikely]]
        return false;

    VkBufferDeviceAddressInfo const info
    {
        .sType = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO,
        .pNext = nullptr,
        .buffer = _gpuBuffer._buffer
    };

    _bdaStream0 = vkGetBufferDeviceAddress ( renderer.GetDevice (), &info );
    _bdaStream1 = _bdaStream0 + static_cast<VkDeviceAddress> ( STREAM_1_OFFSET );

    _barriers[ 0U ].buffer = _gpuBuffer._buffer;
    _barriers[ 1U ].buffer = _gpuBuffer._buffer;

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
    _gpuBuffer.Destroy ( renderer );
}

VkDeviceAddress UIPass::BufferStream::GetStream0Address () const noexcept
{
    return _bdaStream0;
}

VkDeviceAddress UIPass::BufferStream::GetStream1Address () const noexcept
{
    return _bdaStream1;
}

UIBufferStreams UIPass::BufferStream::GetData ( size_t startIndex, size_t neededVertices ) const noexcept
{
    return
    {
        ._stream0
        {
            reinterpret_cast<UIVertexStream0*> ( _data + startIndex * sizeof ( UIVertexStream0 ) ),
            neededVertices
        },

        ._stream1
        {
            reinterpret_cast<UIVertexStream1*> ( _data + STREAM_1_OFFSET + startIndex * sizeof ( UIVertexStream1 ) ),
            neededVertices
        }
    };
}

void UIPass::BufferStream::UpdateGeometry ( VkCommandBuffer commandBuffer, size_t readIdx, size_t writeIdx ) noexcept
{
    auto const offset0 = static_cast<VkDeviceSize> ( readIdx * sizeof ( UIVertexStream0 ) );
    auto const offset1 = static_cast<VkDeviceSize> ( STREAM_1_OFFSET + readIdx * sizeof ( UIVertexStream1 ) );
    size_t const count = writeIdx - readIdx;

    VkBufferCopy const copy[]
    {
        {
            .srcOffset = offset0,
            .dstOffset = offset0,
            .size = static_cast<VkDeviceSize> ( count * sizeof ( UIVertexStream0 ) )
        },
        {
            .srcOffset = offset1,
            .dstOffset = offset1,
            .size = static_cast<VkDeviceSize> ( count * sizeof ( UIVertexStream1 ) )
        }
    };

    vkCmdCopyBuffer ( commandBuffer,
        _staging._buffer,
        _gpuBuffer._buffer,
        static_cast<uint32_t> ( std::size ( copy ) ),
        copy
    );

    VkBufferMemoryBarrier &b0 = _barriers[ 0U ];
    b0.offset = offset0;
    b0.size = copy[ 0U ].size;

    VkBufferMemoryBarrier &b1 = _barriers[ 1U ];
    b1.offset = offset1;
    b1.size = copy[ 1U ].size;

    vkCmdPipelineBarrier ( commandBuffer,
        VK_PIPELINE_STAGE_TRANSFER_BIT,
        VK_PIPELINE_STAGE_VERTEX_SHADER_BIT,
        0U,
        0U,
        nullptr,
        std::size ( _barriers ),
        _barriers,
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
    _fontStorage ( resourceHeap ),
    _resourceHeap ( resourceHeap )
{
    ImageStorage::SetResourceHeap ( resourceHeap );
}

// FUCK - remove maybe_unused
bool UIPass::Execute ( [[maybe_unused]] VkCommandBuffer commandBuffer, size_t commandBufferIndex ) noexcept
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
    _resourceHeap.Bind ( commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, _program.GetPipelineLayout () );

    _pushConstants._bdaStream0 = _uiVertices.GetStream0Address () +
        static_cast<VkDeviceAddress> ( _readVertexIndex * sizeof ( UIVertexStream0 ) );

    _pushConstants._bdaStream1 = _uiVertices.GetStream1Address () +
        static_cast<VkDeviceAddress> ( _readVertexIndex * sizeof ( UIVertexStream1 ) );

    _program.SetPushConstants ( commandBuffer, &_pushConstants );

    vkCmdDraw ( commandBuffer, _vertices, 1U, 0U, 0U );

    for ( uint16_t const image : _usedImages )
        _inUseImageTracker.MarkInUse ( image, commandBufferIndex );

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
        _uiVertices.Init ( renderer ) &&
        ImageStorage::OnInitDevice ( renderer );

    if ( !status ) [[unlikely]]
        return false;

    auto const probe = ImageStorage::GetImage ( std::string ( TEXT_LUT ), false );

    if ( !probe ) [[unlikely]]
        return false;

    _textLUT = static_cast<uint16_t> ( probe->_image );

#if defined ( AV_ENABLE_VVL ) || defined ( AV_ENABLE_RENDERDOC ) || defined ( AV_ENABLE_NSIGHT )

    ImageStorage::SetName ( _textLUT, "Text LUT" );

#endif // AV_ENABLE_VVL || AV_ENABLE_RENDERDOC || AV_ENABLE_NSIGHT

    _usedImages.reserve ( INITIAL_USED_IMAGE_CAPACITY );
    return true;
}

void UIPass::OnDestroyDevice ( android_vulkan::Renderer &renderer ) noexcept
{
    VkDevice device = renderer.GetDevice ();

    _program.Destroy ( device );
    _uiVertices.Destroy ( renderer );
    _inUseImageTracker.Destroy ();
    _fontStorage.Destroy ( renderer );

    _writeVertexIndex = 0U;
    _readVertexIndex = 0U;

    _usedImages.clear ();
    _usedImages.shrink_to_fit ();

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
    _usedImages.clear ();
    _vertices = 0U;
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
    _vertices = static_cast<uint32_t> ( neededVertices );

    return UIPass::UIBufferResponse { _uiVertices.GetData ( nextIdx, neededVertices ) };
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

void UIPass::SubmitImage ( uint16_t image ) noexcept
{
    _usedImages.push_back ( image );
    _hasChanges = true;
}

void UIPass::SubmitNonImage () noexcept
{
    _hasChanges = true;
}

bool UIPass::UploadGPUFontData ( android_vulkan::Renderer &renderer, VkCommandBuffer commandBuffer ) noexcept
{
    AV_TRACE ( "Upload GPU font data" )
    AV_VULKAN_GROUP ( commandBuffer, "Upload GPU font data" )
    return _fontStorage.UploadGPUData ( renderer, commandBuffer );
}

void UIPass::UploadGPUGeometryData ( android_vulkan::Renderer &renderer, VkCommandBuffer commandBuffer ) noexcept
{
    AV_TRACE ( "Upload UI geometry data" )
    AV_VULKAN_GROUP ( commandBuffer, "Upload UI geometry data" )

    if ( _isTransformChanged ) [[unlikely]]
        UpdateTransform ( renderer );

    if ( _hasChanges )
    {
        UpdateGeometry ( commandBuffer );
    }
}

void UIPass::AppendImage ( UIVertexStream0* stream0,
    UIVertexStream1* stream1,
    GXColorUNORM color,
    GXVec2 const &topLeft,
    GXVec2 const &bottomRight,
    uint16_t image
) noexcept
{
    stream0[ 0U ] =
    {
        ._position = topLeft,
        ._uv = IMAGE_TOP_LEFT,
        ._color = color
    };

    stream0[ 1U ] =
    {
        ._position = GXVec2 ( bottomRight._data[ 0U ], topLeft._data[ 1U ] ),
        ._uv = android_vulkan::Half2 ( IMAGE_BOTTOM_RIGHT._data[ 0U ], IMAGE_TOP_LEFT._data[ 1U ] ),
        ._color = color
    };

    stream0[ 2U ] =
    {
        ._position = bottomRight,
        ._uv = IMAGE_BOTTOM_RIGHT,
        ._color = color
    };

    stream0[ 3U ] =
    {
        ._position = bottomRight,
        ._uv = IMAGE_BOTTOM_RIGHT,
        ._color = color
    };

    stream0[ 4U ] =
    {
        ._position = GXVec2 ( topLeft._data[ 0U ], bottomRight._data[ 1U ] ),
        ._uv = android_vulkan::Half2 ( IMAGE_TOP_LEFT._data[ 0U ], IMAGE_BOTTOM_RIGHT._data[ 1U ] ),
        ._color = color
    };

    stream0[ 5U ] =
    {
        ._position = topLeft,
        ._uv = IMAGE_TOP_LEFT,
        ._color = color
    };

    stream1[ 0U ] =
    {
        ._image = image,
        ._uiPrimitiveType = PBR_UI_PRIMITIVE_TYPE_IMAGE
    };

    stream1[ 1U ] =
    {
        ._image = image,
        ._uiPrimitiveType = PBR_UI_PRIMITIVE_TYPE_IMAGE
    };

    stream1[ 2U ] =
    {
        ._image = image,
        ._uiPrimitiveType = PBR_UI_PRIMITIVE_TYPE_IMAGE
    };

    stream1[ 3U ] =
    {
        ._image = image,
        ._uiPrimitiveType = PBR_UI_PRIMITIVE_TYPE_IMAGE
    };

    stream1[ 4U ] =
    {
        ._image = image,
        ._uiPrimitiveType = PBR_UI_PRIMITIVE_TYPE_IMAGE
    };

    stream1[ 5U ] =
    {
        ._image = image,
        ._uiPrimitiveType = PBR_UI_PRIMITIVE_TYPE_IMAGE
    };
}

void UIPass::AppendRectangle ( UIVertexStream0* stream0,
    UIVertexStream1* stream1,
    GXColorUNORM color,
    GXVec2 const &topLeft,
    GXVec2 const &bottomRight
) noexcept
{
    UIVertexStream0 &x0 = stream0[ 0U ];
    x0._position = topLeft;
    x0._color = color;

    UIVertexStream0 &x1 = stream0[ 1U ];
    x1._position = GXVec2 ( bottomRight._data[ 0U ], topLeft._data[ 1U ] ),
    x1._color = color;

    UIVertexStream0 &x2 = stream0[ 2U ];
    x2._position = bottomRight;
    x2._color = color;

    UIVertexStream0 &x3 = stream0[ 3U ];
    x3._position = bottomRight;
    x3._color = color;

    UIVertexStream0 &x4 = stream0[ 4U ];
    x4._position = GXVec2 ( topLeft._data[ 0U ], bottomRight._data[ 1U ] );
    x4._color = color;

    UIVertexStream0 &x5 = stream0[ 5U ];
    x5._position = topLeft;
    x5._color = color;

    stream1[ 0U ]._uiPrimitiveType = PBR_UI_PRIMITIVE_TYPE_GEOMETRY;
    stream1[ 1U ]._uiPrimitiveType = PBR_UI_PRIMITIVE_TYPE_GEOMETRY;
    stream1[ 2U ]._uiPrimitiveType = PBR_UI_PRIMITIVE_TYPE_GEOMETRY;
    stream1[ 3U ]._uiPrimitiveType = PBR_UI_PRIMITIVE_TYPE_GEOMETRY;
    stream1[ 4U ]._uiPrimitiveType = PBR_UI_PRIMITIVE_TYPE_GEOMETRY;
    stream1[ 5U ]._uiPrimitiveType = PBR_UI_PRIMITIVE_TYPE_GEOMETRY;
}

void UIPass::AppendText ( UIVertexStream0* stream0,
    UIVertexStream1* stream1,
    GXColorUNORM color,
    GXVec2 const &topLeft,
    GXVec2 const &bottomRight,
    android_vulkan::Half2 const &glyphTopLeft,
    android_vulkan::Half2 const &glyphBottomRight,
    uint16_t atlas
) noexcept
{
    stream0[ 0U ] =
    {
        ._position = topLeft,
        ._uv = glyphTopLeft,
        ._color = color
    };

    stream0[ 1U ] =
    {
        ._position = GXVec2 ( bottomRight._data[ 0U ], topLeft._data[ 1U ] ),
        ._uv = android_vulkan::Half2 ( glyphBottomRight._data[ 0U ], glyphTopLeft._data[ 1U ] ),
        ._color = color
    };

    stream0[ 2U ] =
    {
        ._position = bottomRight,
        ._uv = glyphBottomRight,
        ._color = color
    };

    stream0[ 3U ] =
    {
        ._position = bottomRight,
        ._uv = glyphBottomRight,
        ._color = color
    };

    stream0[ 4U ] =
    {
        ._position = GXVec2 ( topLeft._data[ 0U ], bottomRight._data[ 1U ] ),
        ._uv = android_vulkan::Half2 ( glyphTopLeft._data[ 0U ], glyphBottomRight._data[ 1U ] ),
        ._color = color
    };

    stream0[ 5U ] =
    {
        ._position = topLeft,
        ._uv = glyphTopLeft,
        ._color = color
    };

    stream1[ 0U ] =
    {
        ._image = atlas,
        ._uiPrimitiveType = PBR_UI_PRIMITIVE_TYPE_TEXT
    };

    stream1[ 1U ] =
    {
        ._image = atlas,
        ._uiPrimitiveType = PBR_UI_PRIMITIVE_TYPE_TEXT,
    };

    stream1[ 2U ] =
    {
        ._image = atlas,
        ._uiPrimitiveType = PBR_UI_PRIMITIVE_TYPE_TEXT
    };

    stream1[ 3U ] =
    {
        ._image = atlas,
        ._uiPrimitiveType = PBR_UI_PRIMITIVE_TYPE_TEXT
    };

    stream1[ 4U ] =
    {
        ._image = atlas,
        ._uiPrimitiveType = PBR_UI_PRIMITIVE_TYPE_TEXT
    };

    stream1[ 5U ] =
    {
        ._image = atlas,
        ._uiPrimitiveType = PBR_UI_PRIMITIVE_TYPE_TEXT
    };
}

void UIPass::ReleaseImage ( uint16_t image ) noexcept
{
    ImageStorage::ReleaseImage ( image );
}

std::optional<UIPass::Image> UIPass::RequestImage ( std::string const &asset ) noexcept
{
    return ImageStorage::GetImage ( asset, true );
}

void UIPass::UpdateGeometry ( VkCommandBuffer commandBuffer ) noexcept
{
    _uiVertices.UpdateGeometry ( commandBuffer, _readVertexIndex, _writeVertexIndex );
    _hasChanges = false;
}

void UIPass::UpdateTransform ( android_vulkan::Renderer &renderer ) noexcept
{
    float const scaleX = 2.0F / _bottomRight._data[ 0U ];
    float const scaleY = 2.0F / _bottomRight._data[ 1U ];
    GXMat4 const &orientation = renderer.GetPresentationEngineTransform ();

    _pushConstants._rotateScaleRow0.Multiply ( *reinterpret_cast<GXVec2 const*> ( orientation._data[ 0U ] ), scaleX );
    _pushConstants._rotateScaleRow1.Multiply ( *reinterpret_cast<GXVec2 const*> ( orientation._data[ 1U ] ), scaleY );
    _pushConstants._offset.Multiply ( _bottomRight, -0.5F );

    _isTransformChanged = false;
}

} // namespace pbr::windows
