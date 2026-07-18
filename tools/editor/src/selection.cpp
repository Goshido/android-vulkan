#include <precompiled_headers.hpp>
#include <logger.hpp>
#include <native_renderer.hpp>
#include <program_info.hpp>
#include <resource_heap.hpp>
#include <selection.hpp>
#include <trace.hpp>
#include <vulkan_utils.hpp>


namespace editor {

bool Selection::Buffer::Init ( android_vulkan::Renderer &renderer,
    size_t size,
    VkBufferUsageFlags usage,
    bool map,
    [[maybe_unused]] char const* name
) noexcept
{
    VkBufferCreateInfo const bufferInfo
    {
        .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0U,
        .size = static_cast<VkDeviceSize> ( size ),
        .usage = usage,
        .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
        .queueFamilyIndexCount = 0U,
        .pQueueFamilyIndices = nullptr
    };

    VkDevice device = renderer.GetDevice ();

    bool result = android_vulkan::Renderer::CheckVkResult (
        vkCreateBuffer ( device, &bufferInfo, nullptr, &_buffer ),
        "Selection::Buffer::Init",
        "Can't create buffer"
    );

    if ( !result ) [[unlikely]]
        return false;

    AV_SET_VULKAN_OBJECT_NAME ( device, _buffer, VK_OBJECT_TYPE_BUFFER, "%s", name )

    VkMemoryRequirements memoryRequirements;
    vkGetBufferMemoryRequirements ( device, _buffer, &memoryRequirements );

    constexpr VkMemoryPropertyFlags cases[] =
    {
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
        AV_VK_FLAG ( VK_MEMORY_PROPERTY_HOST_COHERENT_BIT ) | AV_VK_FLAG ( VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT )
    };

    result =
        renderer.TryAllocateMemory ( _memory,
            _offset,
            memoryRequirements,
            cases[ static_cast<size_t> ( map ) ],
            "Can't allocate memory (Selection::Buffer::Init)"
        ) &&

        android_vulkan::Renderer::CheckVkResult ( vkBindBufferMemory ( device, _buffer, _memory, _offset ),
            "Selection::Buffer::Init",
            "Can't bind memory"
        );

    if ( !result ) [[unlikely]]
        return false;

    if ( !map )
    {
        _resourceIdx = ResourceHeap::Instance ().RegisterBuffer ( device, _buffer, bufferInfo.size );
        return static_cast<bool> ( _resourceIdx );
    }

    return renderer.MapMemory ( reinterpret_cast<void* &> ( _data ),
        _memory,
        _offset,
        "Selection::Buffer::Init",
        "Can't map memory"
    );
}

void Selection::Buffer::Destroy ( android_vulkan::Renderer &renderer ) noexcept
{
    if ( _buffer != VK_NULL_HANDLE ) [[likely]]
        vkDestroyBuffer ( renderer.GetDevice (), std::exchange ( _buffer, VK_NULL_HANDLE ), nullptr );

    if ( std::exchange ( _data, nullptr ) )
        renderer.UnmapMemory ( _memory );

    if ( _memory != VK_NULL_HANDLE ) [[likely]]
        renderer.FreeMemory ( std::exchange ( _memory, VK_NULL_HANDLE ), std::exchange ( _offset, 0U ) );

    if ( _resourceIdx ) [[likely]]
    {
        ResourceHeap::Instance ().UnregisterResource ( *std::exchange ( _resourceIdx, std::nullopt ) );
    }
}

//----------------------------------------------------------------------------------------------------------------------

bool Selection::Init ( MessageQueue &messageQueue, android_vulkan::Renderer &renderer ) noexcept
{
    VkDevice device = renderer.GetDevice ();
    auto idCollectProgram = std::make_unique<pbr::IDCollectProgram> ();

    if ( !idCollectProgram->Init ( device, nullptr ) ) [[unlikely]]
        return false;

    auto idCollectProgramReady = [ this ] ( ProgramRef program ) noexcept {
        // NOLINTNEXTLINE - downcast
        _idCollectProgram = std::unique_ptr<pbr::IDCollectProgram> (
            static_cast<pbr::IDCollectProgram*> ( program.release () )
        );
    };

    messageQueue.EnqueueBack (
        Message ( eMessageType::NewProgram,
            [
                info = ProgramInfo ( std::unique_ptr<pbr::Program> ( idCollectProgram.release () ),
                    std::move ( idCollectProgramReady )
                )
            ] () mutable noexcept -> void* {
                return &info;
            }
        )
    );

    auto idCompressProgram = std::make_unique<pbr::IDCompressProgram> ();

    if ( !idCompressProgram->Init ( device, nullptr ) ) [[unlikely]]
        return false;

    auto idCompressProgramReady = [ this ] ( ProgramRef program ) noexcept {
        // NOLINTNEXTLINE - downcast
        _idCompressProgram = std::unique_ptr<pbr::IDCompressProgram> (
            static_cast<pbr::IDCompressProgram*> ( program.release () )
        );
    };

    messageQueue.EnqueueBack (
        Message ( eMessageType::NewProgram,
            [
                info = ProgramInfo ( std::unique_ptr<pbr::Program> ( idCompressProgram.release () ),
                    std::move ( idCompressProgramReady )
                )
            ] () mutable noexcept -> void* {
                return &info;
            }
        )
    );

    return true;
}

void Selection::Destroy ( MessageQueue &messageQueue, android_vulkan::Renderer &renderer ) noexcept
{
    _idSet.Destroy ( renderer );
    _idDevice.Destroy ( renderer );

    for ( Buffer &idHost : _idHost )
        idHost.Destroy ( renderer );

    if ( _idCollectProgram ) [[likely]]
    {
        messageQueue.EnqueueBack (
            Message ( eMessageType::DestroyProgram,
                [ program = ProgramRef ( _idCollectProgram.release () ) ] () mutable noexcept -> void* {
                    return &program;
                }
            )
        );
    }

    if ( !_idCompressProgram ) [[unlikely]]
        return;

    messageQueue.EnqueueBack (
        Message ( eMessageType::DestroyProgram,
            [ program = ProgramRef ( _idCompressProgram.release () ) ] () mutable noexcept -> void* {
                return &program;
            }
        )
    );
}

bool Selection::IsReady () const noexcept
{
    return static_cast<bool> ( _idCollectProgram ) & static_cast<bool> ( _idCompressProgram );
}

uint32_t Selection::GetIDImageResourceIndex () const noexcept
{
    return _collectPushConstants._idImage;
}

void Selection::PrepareIDBuffer ( VkCommandBuffer commandBuffer ) noexcept
{
    Buffer* &counting = _counting;
    Buffer* &ready = _ready;

    if ( !_area & !counting & !ready ) [[likely]]
        return;

    AV_TRACE ( "ID buffer" )
    AV_VULKAN_GROUP ( commandBuffer, "ID buffer" )

    auto &free = _free;
    constexpr size_t counterIdx = 0UZ;
    constexpr size_t counterOffset = 1UZ;

    if ( ready )
    {
        uint64_t const* data = ready->_data;
        auto &lastSelection = _lastSelection;

        std::memcpy ( lastSelection.data () + ID_PREFETCH_ADDRESSES,
            data + counterOffset + ID_PREFETCH_ADDRESSES,
            ( data[ counterIdx ] - ID_PREFETCH_ADDRESSES ) * sizeof ( uint64_t )
        );

        CommitSelect ();
        free[ 1UZ ] = std::exchange ( free[ 0UZ ], std::exchange ( ready, nullptr ) );
    }

    _depInfo.bufferMemoryBarrierCount = 0U;

    if ( counting )
    {
        uint64_t const* data = counting->_data;
        uint64_t const items = data[ counterIdx ];

        auto &lastSelection = _lastSelection;
        lastSelection.resize ( static_cast<size_t> ( items ) );

        if ( items > 0ULL )
        {
            std::memcpy ( lastSelection.data (),
                data + counterOffset,
                std::min ( items, ID_PREFETCH_ADDRESSES ) * sizeof ( uint64_t )
            );
        }

        if ( items <= ID_PREFETCH_ADDRESSES )
        {
            CommitSelect ();
            free[ 1UZ ] = std::exchange ( free[ 0UZ ], std::exchange ( counting, nullptr ) );
        }
        else
        {
            VkBufferCopy const copy
            {
                .srcOffset = ID_PREFETCH_SIZE,
                .dstOffset = ID_PREFETCH_SIZE,
                .size = static_cast<VkDeviceSize> ( ( items - ID_PREFETCH_ADDRESSES ) * sizeof ( uint64_t ) )
            };

            VkBuffer idHostBuffer = counting->_buffer;
            _idHostBarrier001.buffer = idHostBuffer;
            _idHostBarrier001.size = copy.size;
            _depInfo.bufferMemoryBarrierCount = 1U;
            _depInfo.pBufferMemoryBarriers = &_idHostBarrier001;
            vkCmdPipelineBarrier2 ( commandBuffer, &_depInfo );

            vkCmdCopyBuffer ( commandBuffer, _idDevice._buffer, idHostBuffer, 1U, &copy );

            VkBufferMemoryBarrier2 &idHostBarrier = _barriers001[ 2U ];
            idHostBarrier.size = copy.size;

            if ( !_area )
            {
                _depInfo.pBufferMemoryBarriers = &idHostBarrier;
                vkCmdPipelineBarrier2 ( commandBuffer, &_depInfo );
            }

            ready = std::exchange ( counting, nullptr );
        }
    }

    if ( !_area )
        return;

    _depInfo.bufferMemoryBarrierCount += 2U;
    _depInfo.pBufferMemoryBarriers = _barriers001;
    vkCmdPipelineBarrier2 ( commandBuffer, &_depInfo );

    vkCmdFillBuffer ( commandBuffer, _idSet._buffer, 0U, VK_WHOLE_SIZE, 0U );
    vkCmdFillBuffer ( commandBuffer, _idDevice._buffer, 0U, static_cast<VkDeviceSize> ( sizeof ( uint64_t ) ), 0U );

    counting = std::exchange ( free[ 0UZ ], std::exchange ( free[ 1UZ ], nullptr ) );

    VkBufferMemoryBarrier2 &idHostBarrier = _barriers002[ 2U ];
    idHostBarrier.buffer = counting->_buffer;

    _depInfo.bufferMemoryBarrierCount = static_cast<uint32_t> ( std::size ( _barriers002 ) );
    _depInfo.pBufferMemoryBarriers = _barriers002;
    vkCmdPipelineBarrier2 ( commandBuffer, &_depInfo );
}

void Selection::OnGBufferResolutionChanged ( android_vulkan::Texture2D &idImage, uint32_t idResourceIdx ) noexcept
{
    pbr::IDCollectProgram::PushConstants &collectPushConstants = _collectPushConstants;
    pbr::IDCompressProgram::PushConstants &compressPushConstants = _compressPushConstants;
    collectPushConstants._idImage = idResourceIdx;

    android_vulkan::Renderer &renderer = NativeRenderer::Instance ();
    _idSet.Destroy ( renderer );
    _idDevice.Destroy ( renderer );

    VkExtent2D &resolution = _idImageResolution;
    resolution = idImage.GetResolution ();

    VkExtent2D const &v = NativeRenderer::Instance ().GetViewportResolution ();
    float const convX = static_cast<float> ( resolution.width - 1U ) / static_cast<float> ( v.width - 1U );
    float const convY = static_cast<float> ( resolution.height - 1U ) / static_cast<float> ( v.height - 1U );
    _areaConv = GXVec4 ( convX, convY, convX, convY );

    size_t const size = static_cast<size_t> ( resolution.width * resolution.height ) * sizeof ( uint64_t );

    // First item is counter
    auto const uniqueSize = sizeof ( uint64_t ) + size;

    bool const result =
        _idSet.Init ( renderer,
            size,

            AV_VK_FLAG ( VK_BUFFER_USAGE_STORAGE_BUFFER_BIT ) |
                AV_VK_FLAG ( VK_BUFFER_USAGE_TRANSFER_DST_BIT ) |
                AV_VK_FLAG ( VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT ),

            false,
            "ID set"
        ) &&

        _idDevice.Init ( renderer,
            uniqueSize,

            AV_VK_FLAG ( VK_BUFFER_USAGE_STORAGE_BUFFER_BIT ) |
                AV_VK_FLAG ( VK_BUFFER_USAGE_TRANSFER_SRC_BIT ) |
                AV_VK_FLAG ( VK_BUFFER_USAGE_TRANSFER_DST_BIT ) |
                AV_VK_FLAG ( VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT ),

            false,
            "ID (device)"
        );

    if ( !result )
        return;

    _barriers001[ 0U ].buffer = _idSet._buffer;
    _barriers001[ 1U ].buffer = _idDevice._buffer;

    _barriers002[ 0U ].buffer = _idSet._buffer;
    _barriers002[ 1U ].buffer = _idDevice._buffer;

    _idSetBarrier.buffer = _idSet._buffer;
    _idDeviceBarrier.buffer = _idDevice._buffer;

    for ( Buffer &idHost : _idHost )
    {
        idHost.Destroy ( renderer );

        if ( !idHost.Init ( renderer, uniqueSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT, true, "ID (host)" ) )
        {
            return;
        }
    }

    collectPushConstants._idSet = *_idSet._resourceIdx;
    compressPushConstants._idSet = collectPushConstants._idSet;
    compressPushConstants._uniqueIDs = *_idDevice._resourceIdx;
}

void Selection::Begin ( int32_t x, int32_t y ) noexcept
{
    _begin = std::optional<Point> (
        {
            ._x = x,
            ._y = y
        }
    );

    MessageQueue::Instance ().EnqueueBack (
        Message ( eMessageType::InvokeRenderSession,
            [ this, x, y ] () noexcept -> void* {
                CommitArea ( Rect ( x, x, y, y ) );
                return nullptr;
            }
        )
    );
}

std::optional<Rect> Selection::Update ( int32_t x, int32_t y ) noexcept
{
    if ( !_begin ) [[likely]]
        return std::nullopt;

    Rect area ( _begin->_x, x, _begin->_y, y );
    area.Normalize ();

    MessageQueue::Instance ().EnqueueBack (
        Message ( eMessageType::InvokeRenderSession,
            [ this, area ] () mutable noexcept -> void* {
                CommitArea ( std::move ( area ) );
                return nullptr;
            }
        )
    );

    return std::optional<Rect> { std::move ( area ) };
}

void Selection::End ( int32_t x, int32_t y, bool /*invert*/ ) noexcept
{
    if ( !_begin ) [[likely]]
        return;

    Rect area ( _begin->_x, x, _begin->_y, y );
    area.Normalize ();

    MessageQueue::Instance ().EnqueueBack (
        Message ( eMessageType::InvokeRenderSession,
            [ this, area = std::move ( area ) ] () mutable noexcept -> void* {
                CommitArea ( std::move ( area ) );
                return nullptr;
            }
        )
    );

    _begin = std::nullopt;
}

bool Selection::IsSelectionRequested () const noexcept
{
    return static_cast<bool> ( _area );
}

bool Selection::HasSelection () const noexcept
{
    return !_items.empty ();
}

void Selection::ComputeSelect ( VkCommandBuffer commandBuffer ) noexcept
{
    if ( !_area | !IsReady () ) [[likely]]
        return;

    AV_TRACE ( "Select" )
    AV_VULKAN_GROUP ( commandBuffer, "Select" )

    pbr::IDCollectProgram &collectProgram = *_idCollectProgram;
    collectProgram.Bind ( commandBuffer );
    collectProgram.SetPushConstants ( commandBuffer, &_collectPushConstants );

    VkExtent3D params = pbr::IDCollectProgram::DispatchParams ( _area->_left,
        _area->_right,
        _area->_top,
        _area->_bottom
    );

    vkCmdDispatch ( commandBuffer, params.width, params.height, params.depth );

    _depInfo.bufferMemoryBarrierCount = 1U;
    _depInfo.pBufferMemoryBarriers = &_idSetBarrier;
    vkCmdPipelineBarrier2 ( commandBuffer, &_depInfo );

    pbr::IDCompressProgram &compressProgram = *_idCompressProgram;
    compressProgram.Bind ( commandBuffer );
    compressProgram.SetPushConstants ( commandBuffer, &_compressPushConstants );

    params = pbr::IDCompressProgram::DispatchParams ( _compressPushConstants._capacity );
    vkCmdDispatch ( commandBuffer, params.width, params.height, params.depth );

    _depInfo.pBufferMemoryBarriers = &_idDeviceBarrier;
    vkCmdPipelineBarrier2 ( commandBuffer, &_depInfo );

    VkBufferCopy const copy
    {
        .srcOffset = 0U,
        .dstOffset = 0U,
        .size = ID_PREFETCH_SIZE
    };

    VkBuffer idHostBuffer = _counting->_buffer;
    vkCmdCopyBuffer ( commandBuffer, _idDevice._buffer, idHostBuffer, 1U, &copy );

    _idHostBarrier002.buffer = idHostBuffer;
    _depInfo.pBufferMemoryBarriers = &_idHostBarrier002;
    vkCmdPipelineBarrier2 ( commandBuffer, &_depInfo );

    _area = std::nullopt;
}

void Selection::CommitSelect () noexcept
{
    MessageQueue::Instance ().EnqueueBack (
        Message ( eMessageType::InvokeIO,
            [ this, selection = std::move ( _lastSelection ) ] () mutable noexcept -> void* {
                std::unordered_set<Actor*> selected {};
                selected.insert ( selection.cbegin (), selection.cend () );

                for ( Actor* actor : _items )
                {
                    if ( selected.contains ( actor ) )
                    {
                        selected.erase ( actor );
                        continue;
                    }

                    actor->Deselect ();
                }

                for ( Actor* actor : selected )
                    actor->Select ();

                _items.swap ( selection );
                return nullptr;
            }
        )
    );
}

void Selection::CommitArea ( Rect &&canvasArea ) noexcept
{
    VkExtent2D const &v = NativeRenderer::Instance ().GetViewportResolution ();

    GXVec4 a (
        static_cast<float> ( std::max ( 0, canvasArea._left ) ),
        static_cast<float> ( std::max ( 0, canvasArea._top ) ),
        static_cast<float> ( std::min ( static_cast<int32_t> ( v.width - 1U ), canvasArea._right ) ),
        static_cast<float> ( std::min ( static_cast<int32_t> ( v.height - 1U ), canvasArea._bottom ) )
    );

    a.Multiply ( a, _areaConv );

    Rect area (
        static_cast<int32_t> ( a._data[ 0U ] ),
        static_cast<int32_t> ( a._data[ 2U ] ),
        static_cast<int32_t> ( a._data[ 1U ] ),
        static_cast<int32_t> ( a._data[ 3U ] )
    );

    _collectPushConstants._offset =
    {
        .width = static_cast<uint32_t> ( area._left ),
        .height = static_cast<uint32_t> ( area._top )
    };

    int32_t w = area.GetWidth ();
    int32_t h = area.GetHeight ();

    _collectPushConstants._size =
    {
        .width = static_cast<uint32_t> ( w++ ),
        .height = static_cast<uint32_t> ( h++ )
    };

    _collectPushConstants._capacity = static_cast<uint32_t> ( w * h );
    _compressPushConstants._capacity = _collectPushConstants._capacity;
    _area = std::optional<Rect> ( std::move ( area ) );
}

} // namespace editor
