#include <memory_allocator.h>
#include <core.h>
#include <renderer.h>
#include <vulkan_utils.h>

GX_DISABLE_COMMON_WARNINGS

#include <cassert>
#include <chrono>
#include <fstream>

GX_RESTORE_WARNING_STATE


namespace android_vulkan {

constexpr static VkDeviceSize BYTES_PER_KILOBYTE = 1024U;
constexpr static VkDeviceSize BYTES_PER_MEGABYTE = 1024U * BYTES_PER_KILOBYTE;
constexpr static VkDeviceSize BYTES_PER_GIGABYTE = 1024U * BYTES_PER_MEGABYTE;

constexpr static VkDeviceSize MEGABYTES_PER_CHUNK = 128U;
constexpr static VkDeviceSize BYTES_PER_CHUNK = MEGABYTES_PER_CHUNK * BYTES_PER_MEGABYTE;

constexpr static size_t SNAPSHOT_INITIAL_SIZE_MEGABYTES = 4U;

//----------------------------------------------------------------------------------------------------------------------

void MemoryAllocator::Chunk::FreeMemory ( VkDeviceSize offset ) noexcept
{
    auto const findResult = _usedBlocks.find ( offset );
    assert ( findResult != _usedBlocks.cend () );

    Block* block = findResult->second;
    _usedBlocks.erase ( findResult );

    // "block" could be merged with other free blocks. So there are 4 cases:
    //  - used or none, block, used or none
    //  - used or none, free, block, used or none
    //  - used or none, block, free, used or none
    //  - used or none, free, block, free, used or none

    Offset mergeOffset = offset;
    VkDeviceSize mergeSize = block->_size;

    // By convention only used block has "_freePrevious" and "_freeNext" as nullptr.
    // Note "_freePrevious" and "_freeNext" could be nullptr if there is only one empty block in chunk.
    // So it's needed to check "_freeBlocks" link as well.

    if ( Block* before = block->_blockChainPrevious; before )
    {
        if ( before->_freePrevious != nullptr | _freeBlocks._head == before )
        {
            mergeOffset = before->_offset;
            mergeSize += before->_size;
            RemoveFreeBlock ( *before );
        }
    }

    if ( Block* after = block->_blockChainNext; after )
    {
        if ( after->_freePrevious != nullptr | _freeBlocks._head == after )
        {
            mergeSize += after->_size;
            RemoveFreeBlock ( *after );
        }
    }

    // Collect merged free block...
    block->_size = mergeSize;
    block->_offset = mergeOffset;
    LinkFreeBlock ( *block );
}

bool MemoryAllocator::Chunk::Init ( VkDevice device, size_t memoryTypeIndex ) noexcept
{
    VkMemoryAllocateInfo const allocateInfo
    {
        .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
        .pNext = nullptr,
        .allocationSize = BYTES_PER_CHUNK,
        .memoryTypeIndex = static_cast<uint32_t> ( memoryTypeIndex )
    };

    bool const result = Renderer::CheckVkResult ( vkAllocateMemory ( device, &allocateInfo, nullptr, &_memory ),
        "MemoryAllocator::Chunk::Init",
        "Can't allocate memory"
    );

    if ( !result )
        return false;

    AV_REGISTER_DEVICE_MEMORY ( "MemoryAllocator::Chunk::_memory" )

    _blockChain = new Block
    {
        ._blockChainPrevious = nullptr,
        ._blockChainNext = nullptr,
        ._freePrevious = nullptr,
        ._freeNext = nullptr,
        ._offset = 0U,
        ._size = BYTES_PER_CHUNK
    };

    _freeBlocks._head = _blockChain;
    _freeBlocks._tail = _blockChain;

    return true;
}

void MemoryAllocator::Chunk::Destroy ( VkDevice device ) noexcept
{
    if ( _memory == VK_NULL_HANDLE )
        return;

    vkFreeMemory ( device, _memory, nullptr );
    _memory = VK_NULL_HANDLE;
    AV_UNREGISTER_DEVICE_MEMORY ( "MemoryAllocator::Chunk::_memory" )

    Block* b = _blockChain;

    while ( b )
    {
        Block* p = b;
        b = b->_blockChainNext;
        delete p;
    }

    _blockChain = nullptr;
    _freeBlocks._head = nullptr;
    _freeBlocks._tail = nullptr;

    _usedBlocks.clear ();
}

bool MemoryAllocator::Chunk::IsUsed () const noexcept
{
    return !_usedBlocks.empty ();
}

void MemoryAllocator::Chunk::MakeJSONChunk ( std::string &json, size_t idChunks, size_t idChunk ) const noexcept
{
    char buffer[ 256U ];
    Offset offset = 0U;
    VkDeviceSize size = 0U;

    char idChunksString[ 32U ];
    std::snprintf ( idChunksString, std::size ( idChunksString ), "%zu", idChunks );

    char idChunkString[ 32U ];
    std::snprintf ( idChunkString, std::size ( idChunkString ), "%zu", idChunk );

    // This improve visualization in chrome://tracing by separating two and more sequentially used blocks.
    constexpr Offset separator = 1U;

    constexpr char const* cases[] = { "Free", "Used" };
    constexpr char const begin[] = R"__(,{"name": "%s", "ph": "B", "pid": %s, "tid": %s, "ts": %zu})__";
    constexpr char const end[] = R"__(,{"name": "%s", "ph": "E", "pid": %s, "tid": %s, "ts": %zu})__";

    for ( Block const* block = _blockChain; block; block = block->_blockChainNext )
    {
        // By convention only used block has "_freePrevious" and "_freeNext" as nullptr.
        // Note "_freePrevious" and "_freeNext" could be nullptr if there is only one empty block in chunk.
        // So it's needed to check "_freeBlocks" link as well.
        auto const idx = static_cast<size_t> ( block->_freePrevious == nullptr & _freeBlocks._head != block );
        char const* blockType = cases[ idx ];

        // Branchless optimization: Free block produces "idx" == 0U, used block produces "idx" == 1U.
        size += block->_size * static_cast<VkDeviceSize> ( idx );

        json.append ( buffer,
            static_cast<size_t> (
                std::snprintf ( buffer,
                    std::size ( buffer ),
                    begin,
                    blockType,
                    idChunksString,
                    idChunkString,
                    static_cast<size_t> ( offset )
                )
            )
        );

        offset += block->_size;

        json.append ( buffer,
            static_cast<size_t> (
                std::snprintf ( buffer,
                    std::size ( buffer ),
                    end,
                    blockType,
                    idChunksString,
                    idChunkString,
                    static_cast<size_t> ( offset )
                )
            )
        );

        offset += separator;
    }

    constexpr double toPercent = 100.0 / static_cast<double> ( BYTES_PER_CHUNK );

    constexpr char const meta[] =
        R"__(,{"name":"thread_name","ph":"M","pid":%s,"tid":%s,"args":{"name":"Chunk #%s: %s, %.0f%%"}})__";

    char buffer2[ 32U ];
    constexpr double toKB = 1.0 / static_cast<double> ( BYTES_PER_KILOBYTE );
    constexpr double toMB = 1.0 / static_cast<double> ( BYTES_PER_MEGABYTE );
    constexpr double toGB = 1.0 / static_cast<double> ( BYTES_PER_GIGABYTE );

    if ( size < BYTES_PER_KILOBYTE )
        std::snprintf ( buffer2, std::size ( buffer2 ), "%zu bytes", static_cast<size_t> ( size ) );
    else if ( size < BYTES_PER_MEGABYTE )
        std::snprintf ( buffer2, std::size ( buffer2 ), "%.2f Kb", static_cast<double> ( size ) * toKB );
    else if ( size < BYTES_PER_GIGABYTE )
        std::snprintf ( buffer2, std::size ( buffer2 ), "%.2f Mb", static_cast<double> ( size ) * toMB );
    else
        std::snprintf ( buffer2, std::size ( buffer2 ), "%.2f Gb", static_cast<double> ( size ) * toGB );

    json.append ( buffer,
        static_cast<size_t> (
            std::snprintf ( buffer,
                std::size ( buffer ),
                meta,
                idChunksString,
                idChunkString,
                idChunkString,
                buffer2,
                static_cast<double > ( size ) * toPercent
            )
        )
    );
}

bool MemoryAllocator::Chunk::TryAllocateMemory ( VkDeviceMemory &memory,
    VkDeviceSize &offset,
    VkMemoryRequirements const &requirements
) noexcept
{
    if ( !_freeBlocks._tail || requirements.size > _freeBlocks._tail->_size )
    {
        // Free blocks are sorted by size. So the biggest possible block should be at list tail.
        // The biggest block is less than requested. Early exit.
        return false;
    }

    for ( Block* block = _freeBlocks._head; block; block = block->_freeNext )
    {
        Offset const freeBlockOffset = block->_offset;
        auto const size = static_cast<size_t> ( block->_size );
        auto* ptr = reinterpret_cast<void*> ( freeBlockOffset );
        size_t space = size;

        if ( !ptr )
        {
            // Block with offset 0. Any alignment should work. Checking only required size.
            if ( requirements.size > size )
            {
                continue;
            }
        }
        else
        {
            ptr = std::align ( static_cast<size_t> ( requirements.alignment ),
                static_cast<size_t> ( requirements.size ),
                ptr,
                space
            );

            if ( !ptr )
            {
                // Failed to allocate block with requested size and alignment.
                continue;
            }
        }

        auto const resultOffset = reinterpret_cast<VkDeviceSize> ( ptr );
        UnlinkFreeBlock ( *block );

        // By convention used block has "_freePrevious" and "_freeNext" as nullptr.
        block->_freePrevious = nullptr;
        block->_freeNext = nullptr;

        block->_offset = resultOffset;
        block->_size = requirements.size;

        _usedBlocks.emplace ( resultOffset, block );

        // "block" could be transformed into 4 cases:
        //  - used block
        //  - free block, used block
        //  - used block, free block
        //  - free block, used block, free block

        if ( size_t const before = size - space; before )
        {
            // There is an empty space in front of occupied memory block because of alignment.
            Block* freeBlock = AppendFreeBlock ( freeBlockOffset, static_cast<VkDeviceSize> ( before ) );
            Block* prev = block->_blockChainPrevious;

            if ( prev )
                prev->_blockChainNext = freeBlock;
            else
                _blockChain = freeBlock;

            freeBlock->_blockChainNext = block;
            freeBlock->_blockChainPrevious = prev;
            block->_blockChainPrevious = freeBlock;
        }

        if ( size_t const left = space - static_cast<size_t> ( requirements.size ); left )
        {
            // There is an empty space after occupied memory block.
            Block* freeBlock = AppendFreeBlock ( resultOffset + requirements.size, static_cast<VkDeviceSize> ( left ) );
            Block* next = block->_blockChainNext;

            if ( next )
                next->_blockChainPrevious = freeBlock;

            freeBlock->_blockChainNext = next;
            freeBlock->_blockChainPrevious = block;
            block->_blockChainNext = freeBlock;
        }

        offset = resultOffset;
        memory = _memory;

        return true;
    }

    return false;
}

MemoryAllocator::Chunk::Block* MemoryAllocator::Chunk::AppendFreeBlock ( Offset offset, VkDeviceSize size ) noexcept
{
    auto* block = new Block
    {
        ._blockChainPrevious = nullptr,
        ._blockChainNext = nullptr,
        ._freePrevious = nullptr,
        ._freeNext = nullptr,
        ._offset = offset,
        ._size = size
    };

    LinkFreeBlock ( *block );
    return block;
}

void MemoryAllocator::Chunk::RemoveFreeBlock ( Block &block ) noexcept
{
    Block* p = block._blockChainPrevious;
    Block* n = block._blockChainNext;

    if ( p )
        p->_blockChainNext = n;
    else
        _blockChain = n;

    if ( n )
        n->_blockChainPrevious = p;

    UnlinkFreeBlock ( block );
    delete &block;
}

void MemoryAllocator::Chunk::LinkFreeBlock ( Block &block ) noexcept
{
    VkDeviceSize const size = block._size;

    for ( Block* freeBlock = _freeBlocks._head; freeBlock; freeBlock = freeBlock->_freeNext )
    {
        if ( size > freeBlock->_size )
            continue;

        Block* before = freeBlock->_freePrevious;
        block._freePrevious = before;
        block._freeNext = freeBlock;
        freeBlock->_freePrevious = &block;

        if ( before )
        {
            before->_freeNext = &block;
        }
        else
        {
            // The smallest block so far. It should be added to head.
            _freeBlocks._head = &block;
        }

        return;
    }

    if ( !_freeBlocks._head )
    {
        // "_freeBlocks" is empty.
        _freeBlocks._head = &block;
    }
    else
    {
        // The biggest block so far. It should be added to tail.
        block._freePrevious = _freeBlocks._tail;
        _freeBlocks._tail->_freeNext = &block;
    }

    _freeBlocks._tail = &block;
}

void MemoryAllocator::Chunk::UnlinkFreeBlock ( Block &block ) noexcept
{
    Block* p = block._freePrevious;
    Block* n = block._freeNext;

    if ( p )
        p->_freeNext = n;
    else
        _freeBlocks._head = n;

    if ( n )
    {
        n->_freePrevious = p;
        return;
    }

    _freeBlocks._tail = p;
}

//----------------------------------------------------------------------------------------------------------------------

void MemoryAllocator::FreeMemory ( VkDevice device,
    VkDeviceMemory memory,
    VkDeviceSize offset
) noexcept
{
    std::unique_lock<std::mutex> const lock ( _mutex );

    auto findResult = _chunkMap.find ( memory );
    assert ( findResult != _chunkMap.end () );

    ChunkInfo& chunkInfo = findResult->second;
    chunkInfo._chunk->FreeMemory ( offset );

    if ( chunkInfo._chunk->IsUsed () )
        return;

    chunkInfo._chunk->Destroy ( device );
    chunkInfo._chunks->erase ( chunkInfo._chunk );
    _chunkMap.erase ( findResult );
}

void MemoryAllocator::Init ( VkPhysicalDeviceMemoryProperties const &properties ) noexcept
{
    _properties = properties;
}

void MemoryAllocator::MakeSnapshot () noexcept
{
    // See chrome://tracing JSON format here:
    // https://docs.google.com/document/d/1CvAClvFfyA5R-PhYUmn5OOQtYMH4h6I0nSsKchNAySU/edit

    std::unique_lock<std::mutex> const lock ( _mutex );

    if ( _chunkMap.empty () )
        return;

    constexpr size_t snapshotInitialSizeBytes =
        SNAPSHOT_INITIAL_SIZE_MEGABYTES * static_cast<size_t> ( BYTES_PER_MEGABYTE );

    std::string json {};
    json.reserve ( snapshotInitialSizeBytes );

    constexpr std::string_view begin = R"__({"traceEvents":[)__";
    json.append ( begin.data (), begin.size () );
    bool isFirst = true;

    for ( size_t i = 0U; i < _properties.memoryTypeCount; ++i )
    {
        Chunks const& chunks = _memory[ i ];

        if ( chunks.empty () )
            continue;

        if ( isFirst )
            isFirst = false;
        else
            json.append ( ",", 1 );

        MakeJSONChunks ( json, i, _properties.memoryTypes[ i ].propertyFlags );
        size_t id = 0U;

        for ( Chunk const& chunk : chunks )
        {
            chunk.MakeJSONChunk ( json, i, id++ );
        }
    }

    constexpr std::string_view end = "]}";
    json.append ( end.data (), end.size () );

    auto const timeStamp = std::chrono::system_clock::to_time_t ( std::chrono::system_clock::now () );
    std::tm const& timeInfo = *std::localtime ( &timeStamp );

    char path[ 1024U ];

    std::snprintf ( path,
        std::size ( path ),
        R"__(%s/vulkan memory snapshot %d-%02d-%02d %02d-%02d-%02d %s.json)__",
        Core::GetCacheDirectory ().c_str (),
        1900 + timeInfo.tm_year,
        timeInfo.tm_mon,
        timeInfo.tm_mday,
        timeInfo.tm_hour,
        timeInfo.tm_min,
        timeInfo.tm_sec,
        timeInfo.tm_zone
    );

    std::ofstream report ( path, std::ios::binary | std::ios::trunc | std::ios::out );
    report.write ( json.data (), static_cast<std::streamsize> ( json.size () ) );
}

bool MemoryAllocator::MapMemory ( void*& ptr,
    VkDevice device,
    VkDeviceMemory memory,
    VkDeviceSize offset,
    char const* from,
    char const* message
) noexcept
{
    std::unique_lock<std::mutex> const lock ( _mutex );

    auto findResult = _chunkMap.find ( memory );
    assert ( findResult != _chunkMap.end () );
    ChunkInfo& chunkInfo = findResult->second;

    if ( ++chunkInfo._mapCounter > 1U )
    {
        ptr = static_cast<uint8_t*> ( chunkInfo._mapPointer ) + offset;
        return true;
    }

    bool const result = Renderer::CheckVkResult (
        vkMapMemory ( device, memory, 0U, BYTES_PER_CHUNK, 0U, &chunkInfo._mapPointer ),
        from,
        message
    );

    if ( !result )
        return false;

    ptr = static_cast<uint8_t*> ( chunkInfo._mapPointer ) + offset;
    return true;
}

void MemoryAllocator::UnmapMemory ( VkDevice device, VkDeviceMemory memory ) noexcept
{
    std::unique_lock<std::mutex> const lock ( _mutex );

    auto findResult = _chunkMap.find ( memory );
    assert ( findResult != _chunkMap.end () );
    ChunkInfo& chunkInfo = findResult->second;

    if ( --chunkInfo._mapCounter > 0U )
        return;

    vkUnmapMemory ( device, memory );
    chunkInfo._mapPointer = nullptr;
}

bool MemoryAllocator::TryAllocateMemory ( VkDeviceMemory &memory,
    VkDeviceSize &offset,
    VkDevice device,
    VkMemoryRequirements const &requirements,
    VkMemoryPropertyFlags properties
) noexcept
{
    size_t memoryTypeIndex = std::numeric_limits<uint32_t>::max ();

    for ( uint32_t i = 0U; i < _properties.memoryTypeCount; ++i )
    {
        if ( !( requirements.memoryTypeBits & ( 1U << i ) ) )
            continue;

        VkMemoryType const& memoryType = _properties.memoryTypes[ i ];

        if ( ( memoryType.propertyFlags & properties ) != properties )
            continue;

        memoryTypeIndex = static_cast<size_t> ( i );
        break;
    }

    if ( memoryTypeIndex > VK_MAX_MEMORY_TYPES )
        return false;

    std::unique_lock<std::mutex> const lock ( _mutex );
    auto& chunks = _memory[ memoryTypeIndex ];

    for ( auto& chunk : chunks )
    {
        if ( chunk.TryAllocateMemory ( memory, offset, requirements ) )
        {
            return true;
        }
    }

    Chunk& chunk = chunks.emplace_front ();

    bool const result = chunk.Init ( device, memoryTypeIndex ) &&
        chunk.TryAllocateMemory ( memory, offset, requirements );

    if ( !result )
        return false;

    _chunkMap.emplace ( memory,
        ChunkInfo
        {
            ._chunk = chunks.begin (),
            ._chunks = &chunks,
            ._mapCounter = 0U,
            ._mapPointer = nullptr
        }
    );

    return true;
}

void MemoryAllocator::MakeJSONChunks ( std::string &json, size_t id, VkMemoryPropertyFlags props ) noexcept
{
    constexpr char const begin[] =
        R"__({"name":"process_name","ph":"M","pid":%zu,"args":{"name":"Memory index #%zu:)__";

    char buffer[ 128U ];
    json.append ( buffer, static_cast<size_t> ( std::snprintf ( buffer, std::size ( buffer ), begin, id, id ) ) );

#define AV_ENTRY(x) std::make_pair ( static_cast<uint32_t> ( x ), std::string_view ( " "#x ) )

    constexpr std::pair<uint32_t, std::string_view> const cases[] =
    {
        AV_ENTRY ( VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT ),
        AV_ENTRY ( VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT ),
        AV_ENTRY ( VK_MEMORY_PROPERTY_HOST_COHERENT_BIT ),
        AV_ENTRY ( VK_MEMORY_PROPERTY_HOST_CACHED_BIT ),
        AV_ENTRY ( VK_MEMORY_PROPERTY_LAZILY_ALLOCATED_BIT ),
        AV_ENTRY ( VK_MEMORY_PROPERTY_PROTECTED_BIT ),
        AV_ENTRY ( VK_MEMORY_PROPERTY_DEVICE_COHERENT_BIT_AMD ),
        AV_ENTRY ( VK_MEMORY_PROPERTY_DEVICE_UNCACHED_BIT_AMD ),
        AV_ENTRY ( VK_MEMORY_PROPERTY_RDMA_CAPABLE_BIT_NV ),
        AV_ENTRY ( VK_MEMORY_PROPERTY_RDMA_CAPABLE_BIT_NV )
    };

#undef AV_ENTRY

    for ( auto const& [prop, name] : cases )
    {
        if ( prop & props )
        {
            json.append ( name.data (), name.size () );
        }
    }

    constexpr std::string_view end = R"__("}})__";
    json.append ( end.data (), end.size () );
}

} // namespace android_vulkan
