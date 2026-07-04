#include <precompiled_headers.hpp>
#include <native_renderer.hpp>
#include <program_info.hpp>
#include <resource_heap.hpp>
#include <selection.hpp>
#include <trace.hpp>
#include <vulkan_utils.hpp>


namespace editor {

namespace {

constexpr size_t ID_PREFETCH_ADDRESSES = 64UZ;

constexpr auto ID_PREFETCH_SIZE =
    static_cast<VkDeviceSize> ( sizeof ( uint64_t ) + ID_PREFETCH_ADDRESSES * sizeof ( uint64_t ) );

} // end of anonymous namespace

//----------------------------------------------------------------------------------------------------------------------

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
        vkCreateBuffer ( device, &bufferInfo, nullptr, &_barrier.buffer ),
        "Selection::Buffer::Init",
        "Can't create buffer"
    );

    if ( !result ) [[unlikely]]
        return false;

    AV_SET_VULKAN_OBJECT_NAME ( device, _barrier.buffer, VK_OBJECT_TYPE_BUFFER, "%s", name )

    VkMemoryRequirements memoryRequirements;
    vkGetBufferMemoryRequirements ( device, _barrier.buffer, &memoryRequirements );

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

        android_vulkan::Renderer::CheckVkResult ( vkBindBufferMemory ( device, _barrier.buffer, _memory, _offset ),
            "Selection::Buffer::Init",
            "Can't bind memory"
        );

    if ( !result ) [[unlikely]]
        return false;

    if ( !map )
    {
        _resourceIdx = ResourceHeap::Instance ().RegisterBuffer ( device, _barrier.buffer, bufferInfo.size );
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
    if ( VkBuffer &buffer = _barrier.buffer; buffer != VK_NULL_HANDLE ) [[likely]]
        vkDestroyBuffer ( renderer.GetDevice (), std::exchange ( buffer, VK_NULL_HANDLE ), nullptr );

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

    if ( !_pendingSelect & !counting & !ready ) [[likely]]
        return;

    AV_TRACE ( "ID buffer" )
    AV_VULKAN_GROUP ( commandBuffer, "ID buffer" )

    VkBufferMemoryBarrier &idDeviceBarrier = _idDevice._barrier;
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

        // FUCK - handle in UI thread.
        free[ 1UZ ] = std::exchange ( free[ 0UZ ], std::exchange ( ready, nullptr ) );
    }

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
            // FUCK - handle in UI thread.
            free[ 1UZ ] = std::exchange ( free[ 0UZ ], std::exchange ( counting, nullptr ) );
        }
        else
        {
            auto const offset =
                static_cast<VkDeviceSize> ( ( counterOffset + ID_PREFETCH_ADDRESSES ) * sizeof ( uint64_t ) );

            VkBufferCopy const copy
            {
                .srcOffset = offset,
                .dstOffset = offset,
                .size = static_cast<VkDeviceSize> ( ( items - ID_PREFETCH_ADDRESSES ) * sizeof ( uint64_t ) )
            };

            VkBufferMemoryBarrier &idHostBarrier = counting->_barrier;
            idHostBarrier.srcAccessMask = VK_ACCESS_HOST_READ_BIT;
            idHostBarrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

            vkCmdPipelineBarrier ( commandBuffer,
                VK_PIPELINE_STAGE_HOST_BIT,
                VK_PIPELINE_STAGE_TRANSFER_BIT,
                0U,
                0U,
                nullptr,
                1U,
                &idHostBarrier,
                0U,
                nullptr
            );

            vkCmdCopyBuffer ( commandBuffer, idDeviceBarrier.buffer, idHostBarrier.buffer, 1U, &copy );

            idHostBarrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
            idHostBarrier.dstAccessMask = VK_ACCESS_HOST_READ_BIT;

            vkCmdPipelineBarrier ( commandBuffer,
                VK_PIPELINE_STAGE_TRANSFER_BIT,
                VK_PIPELINE_STAGE_HOST_BIT,
                0U,
                0U,
                nullptr,
                1U,
                &idHostBarrier,
                0U,
                nullptr
            );

            ready = std::exchange ( counting, nullptr );
        }
    }

    if ( !_pendingSelect )
        return;

    VkBufferMemoryBarrier &idSetBarrier = _idSet._barrier;
    idSetBarrier.srcAccessMask = AV_VK_FLAG ( VK_ACCESS_SHADER_READ_BIT ) | AV_VK_FLAG ( VK_ACCESS_SHADER_WRITE_BIT );
    idSetBarrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

    vkCmdPipelineBarrier ( commandBuffer,
        VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
        VK_PIPELINE_STAGE_TRANSFER_BIT,
        0U,
        0U,
        nullptr,
        1U,
        &idSetBarrier,
        0U,
        nullptr
    );

    idDeviceBarrier.srcAccessMask = AV_VK_FLAG ( VK_ACCESS_TRANSFER_READ_BIT ) |
        AV_VK_FLAG ( VK_ACCESS_SHADER_READ_BIT ) |
        AV_VK_FLAG ( VK_ACCESS_SHADER_WRITE_BIT );

    idDeviceBarrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    idDeviceBarrier.size = static_cast<VkDeviceSize> ( sizeof ( uint64_t ) );

    vkCmdPipelineBarrier ( commandBuffer,
        AV_VK_FLAG ( VK_PIPELINE_STAGE_TRANSFER_BIT ) | AV_VK_FLAG ( VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT ),
        VK_PIPELINE_STAGE_TRANSFER_BIT,
        0U,
        0U,
        nullptr,
        1U,
        &idDeviceBarrier,
        0U,
        nullptr
    );

    vkCmdFillBuffer ( commandBuffer, idSetBarrier.buffer, 0U, VK_WHOLE_SIZE, 0U );

    vkCmdFillBuffer ( commandBuffer,
        idDeviceBarrier.buffer,
        0U,
        static_cast<VkDeviceSize> ( sizeof ( uint64_t ) ),
        0U
    );

    idSetBarrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    idSetBarrier.dstAccessMask = AV_VK_FLAG ( VK_ACCESS_SHADER_READ_BIT ) | AV_VK_FLAG ( VK_ACCESS_SHADER_WRITE_BIT );

    vkCmdPipelineBarrier ( commandBuffer,
        VK_PIPELINE_STAGE_TRANSFER_BIT,
        VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
        0U,
        0U,
        nullptr,
        1U,
        &idSetBarrier,
        0U,
        nullptr
    );

    idDeviceBarrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

    idDeviceBarrier.dstAccessMask = AV_VK_FLAG ( VK_ACCESS_SHADER_READ_BIT ) |
        AV_VK_FLAG ( VK_ACCESS_SHADER_WRITE_BIT );

    vkCmdPipelineBarrier ( commandBuffer,
        VK_PIPELINE_STAGE_TRANSFER_BIT,
        VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
        0U,
        0U,
        nullptr,
        1U,
        &idDeviceBarrier,
        0U,
        nullptr
    );

    counting = std::exchange ( free[ 0UZ ], std::exchange ( free[ 1UZ ], nullptr ) );
    VkBufferMemoryBarrier &idHostBarrier = counting->_barrier;

    idHostBarrier.srcAccessMask = AV_VK_FLAG ( VK_ACCESS_TRANSFER_WRITE_BIT ) | AV_VK_FLAG ( VK_ACCESS_HOST_READ_BIT );
    idHostBarrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

    vkCmdPipelineBarrier ( commandBuffer,
        AV_VK_FLAG ( VK_PIPELINE_STAGE_HOST_BIT ) | AV_VK_FLAG ( VK_PIPELINE_STAGE_TRANSFER_BIT ),
        VK_PIPELINE_STAGE_TRANSFER_BIT,
        0U,
        0U,
        nullptr,
        1U,
        &idHostBarrier,
        0U,
        nullptr
    );
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
    collectPushConstants._capacity = resolution.width * resolution.height;
    compressPushConstants._capacity = collectPushConstants._capacity;
    auto const size = static_cast<size_t> ( collectPushConstants._capacity * sizeof ( uint64_t ) );

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

bool Selection::IsSelectionRequested () const noexcept
{
    return _pendingSelect;
}

bool Selection::HasSelection () const noexcept
{
    return !_items.empty ();
}

void Selection::Select ( Rect const &rect, bool invert ) noexcept
{
    MessageQueue::Instance ().EnqueueBack (
        Message ( eMessageType::InvokeRenderSession,
            [ this ]() noexcept -> void* {
                _pendingSelect = true;
                return nullptr;
            }
        )
    );

    // FUCK
    android_vulkan::LogDebug ( "[%d %d][%d %d] %s", rect._left, rect._top, rect._right, rect._bottom,
        invert ? "invert" : "single"
    );
}

void Selection::ComputeSelect ( VkCommandBuffer commandBuffer ) noexcept
{
    if ( !_pendingSelect | !IsReady () ) [[likely]]
        return;

    AV_TRACE ( "Select" )
    AV_VULKAN_GROUP ( commandBuffer, "Select" )

    pbr::IDCollectProgram &collectProgram = *_idCollectProgram;
    collectProgram.Bind ( commandBuffer );
    collectProgram.SetPushConstants ( commandBuffer, &_collectPushConstants );

    VkExtent3D params = pbr::IDCollectProgram::DispatchParams ( _idImageResolution );
    vkCmdDispatch ( commandBuffer, params.width, params.height, params.depth );

    VkBufferMemoryBarrier &idSetBarrier = _idSet._barrier;
    idSetBarrier.srcAccessMask = AV_VK_FLAG ( VK_ACCESS_SHADER_READ_BIT ) |
        AV_VK_FLAG ( VK_ACCESS_SHADER_WRITE_BIT );

    idSetBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

    vkCmdPipelineBarrier ( commandBuffer,
        VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
        VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
        0U,
        0U,
        nullptr,
        1U,
        &idSetBarrier,
        0U,
        nullptr
    );

    pbr::IDCompressProgram &compressProgram = *_idCompressProgram;
    compressProgram.Bind ( commandBuffer );
    compressProgram.SetPushConstants ( commandBuffer, &_compressPushConstants );

    params = pbr::IDCompressProgram::DispatchParams ( _idImageResolution );
    vkCmdDispatch ( commandBuffer, params.width, params.height, params.depth );

    VkBufferMemoryBarrier &idDeviceBarrier = _idDevice._barrier;

    idDeviceBarrier.srcAccessMask = AV_VK_FLAG ( VK_ACCESS_SHADER_READ_BIT ) |
        AV_VK_FLAG ( VK_ACCESS_SHADER_WRITE_BIT );

    idDeviceBarrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
    idDeviceBarrier.size = VK_WHOLE_SIZE;

    vkCmdPipelineBarrier ( commandBuffer,
        VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
        VK_PIPELINE_STAGE_TRANSFER_BIT,
        0U,
        0U,
        nullptr,
        1U,
        &idDeviceBarrier,
        0U,
        nullptr
    );

    VkBufferCopy const copy
    {
        .srcOffset = 0U,
        .dstOffset = 0U,
        .size = ID_PREFETCH_SIZE
    };

    VkBufferMemoryBarrier &idHostBarrier = _counting->_barrier;
    vkCmdCopyBuffer ( commandBuffer, idDeviceBarrier.buffer, idHostBarrier.buffer, 1U, &copy );

    idHostBarrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    idHostBarrier.dstAccessMask = VK_ACCESS_HOST_READ_BIT;

    vkCmdPipelineBarrier ( commandBuffer,
        VK_PIPELINE_STAGE_TRANSFER_BIT,
        VK_PIPELINE_STAGE_HOST_BIT,
        0U,
        0U,
        nullptr,
        1U,
        &idHostBarrier,
        0U,
        nullptr
    );

    _pendingSelect = false;
}

} // namespace editor
