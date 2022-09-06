#include <memory_allocator.h>
#include <renderer.h>
#include <vulkan_utils.h>

GX_DISABLE_COMMON_WARNINGS

#include <cassert>

GX_RESTORE_WARNING_STATE


namespace android_vulkan {

constexpr static VkDeviceSize MEGABYTES_PER_CHUNK = 128U;
constexpr static VkDeviceSize BYTES_PER_CHUNK = MEGABYTES_PER_CHUNK * 1024U * 1024U;

//----------------------------------------------------------------------------------------------------------------------

bool MemoryAllocator::Chunk::FreeMemory ( VkDeviceMemory memory, VkDeviceSize offset ) noexcept
{
    if ( memory != _memory )
        return false;

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

    Blocks mergeBlocks
    {
        ._head = block,
        ._tail = block
    };

    // Note by convention only free block has link to block in "_freeBlocks" list.

    if ( Block* before = block->_previous; before && before->_meInOtherList )
    {
        mergeBlocks._head = before;
        mergeOffset = before->_offset;
        mergeSize += before->_size;
        RemoveFreeBlock ( *before->_meInOtherList );
    }

    if ( Block* after = block->_next; after && after->_meInOtherList )
    {
        mergeBlocks._tail = after;
        mergeSize += after->_size;
        RemoveFreeBlock ( *after->_meInOtherList );
    }

    // Saving link to the next block after merged region.
    Block* nextBlock = mergeBlocks._tail->_next;

    // Collect merged block...
    Block& mergeBlock = *mergeBlocks._head;
    mergeBlock._offset = mergeOffset;
    mergeBlock._size = mergeSize;
    mergeBlock._meInOtherList = AppendFreeBlock ( mergeOffset, mergeSize );

    // Removing blocks which are merged...
    mergeBlocks._tail->_next = nullptr;
    block = mergeBlocks._head->_next;

    while ( block )
    {
        Block* p = block;
        block = block->_next;
        delete p;
    }

    // Linking merged region to rest of the "_blockChain" list.
    mergeBlock._next = nextBlock;
    return true;
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

    _blockChain = new Block {
        ._previous = nullptr,
        ._next = nullptr,
        ._meInOtherList = nullptr,
        ._offset = 0U,
        ._size = BYTES_PER_CHUNK
    };

    _freeBlocks._head = new Block {
        ._previous = nullptr,
        ._next = nullptr,
        ._meInOtherList = _blockChain,
        ._offset = 0U,
        ._size = BYTES_PER_CHUNK
    };

    _freeBlocks._tail = _freeBlocks._head;
    _blockChain->_meInOtherList = _freeBlocks._head;

    return true;
}

void MemoryAllocator::Chunk::Destroy ( VkDevice device ) noexcept
{
    if ( _memory == VK_NULL_HANDLE )
        return;

    vkFreeMemory ( device, _memory, nullptr );
    _memory = VK_NULL_HANDLE;
    AV_UNREGISTER_DEVICE_MEMORY ( "MemoryAllocator::Chunk::_memory" )

    auto const clear = [] ( Block*& block ) noexcept {
        Block* b = block;

        while ( b )
        {
            Block* p = b;
            b = b->_next;
            delete p;
        }

        block = nullptr;
    };

    clear ( _blockChain );

    clear ( _freeBlocks._head );
    _freeBlocks._tail = nullptr;

    _usedBlocks.clear ();
}

bool MemoryAllocator::Chunk::IsUsed () const noexcept
{
    return !_usedBlocks.empty ();
}

bool MemoryAllocator::Chunk::TryAllocateMemory ( VkDeviceMemory &memory,
    VkDeviceSize &offset,
    VkMemoryRequirements const &requirements
) noexcept
{
    for ( Block* freeBlock = _freeBlocks._head; freeBlock; freeBlock = freeBlock->_next )
    {
        auto const size = static_cast<size_t> ( freeBlock->_size );
        size_t space = size;
        auto* ptr = reinterpret_cast<void*> ( freeBlock->_offset );

        ptr = std::align ( static_cast<size_t> ( requirements.alignment ),
            static_cast<size_t> ( requirements.size ),
            ptr,
            space
        );

        if ( !ptr )
            continue;

        Block* blockChainItem = freeBlock->_meInOtherList;
        Block* nextBlock = blockChainItem->_next;
        Block* lastBlock = nullptr;

        // "blockChainBlock" could be transformed into 4 cases:
        //  - used block
        //  - free block, used block
        //  - used block, free block
        //  - free block, used block, free block

        auto const append = [ &lastBlock, &blockChainItem ] ( Block* meInOtherList,
            Offset offset,
            VkDeviceSize size
        ) noexcept {
            if ( !lastBlock )
            {
                blockChainItem->_meInOtherList = meInOtherList;
                blockChainItem->_offset = offset;
                blockChainItem->_size = size;

                lastBlock = blockChainItem;
                return;
            }

            auto* block = new Block
            {
                ._previous = lastBlock,
                ._next = nullptr,
                ._meInOtherList = meInOtherList,
                ._offset = offset,
                ._size = size
            };

            lastBlock->_next = block;
            lastBlock = block;
        };

        RemoveFreeBlock ( *freeBlock );

        if ( size_t const before = size - space; before )
        {
            // There is an empty space in front of occupied memory block because of alignment.
            append ( AppendFreeBlock ( freeBlock->_offset, static_cast<VkDeviceSize> ( before ) ),
                freeBlock->_offset,
                static_cast<VkDeviceSize> ( before )
            );
        }

        auto const resultOffset = reinterpret_cast<VkDeviceSize> ( ptr );
        offset = resultOffset;
        memory = _memory;

        // Note by convention used block has no link to any block in "_blockChain" list.
        append ( nullptr, resultOffset, requirements.size );
        _usedBlocks.emplace ( resultOffset, lastBlock );

        if ( size_t const left = space - static_cast<size_t> ( requirements.size ); left )
        {
            // There is an empty space after occupied memory block.
            Offset const blockOffset = resultOffset + requirements.size;

            append ( AppendFreeBlock ( blockOffset, static_cast<VkDeviceSize> ( left ) ),
                blockOffset,
                static_cast<VkDeviceSize> ( left )
            );
        }

        lastBlock->_next = nextBlock;
        return true;
    }

    return false;
}

MemoryAllocator::Chunk::Block* MemoryAllocator::Chunk::AppendFreeBlock ( Offset offset, VkDeviceSize size ) noexcept
{
    auto* block = new Block
    {
        ._previous = nullptr,
        ._next = nullptr,
        ._meInOtherList = nullptr,
        ._offset = offset,
        ._size = size
    };

    for ( Block* freeBlock = _freeBlocks._head; freeBlock; freeBlock = freeBlock->_next )
    {
        if ( size > freeBlock->_size )
            continue;

        block->_previous = freeBlock->_previous;
        block->_next = freeBlock;

        if ( freeBlock == _freeBlocks._head )
        {
            // The smallest block so far. It should be added to head.
            _freeBlocks._head = block;
        }

        return block;
    }

    if ( !_freeBlocks._head )
    {
        // "_freeBlocks" is empty.
        _freeBlocks._head = block;
    }
    else
    {
        // The biggest block so far. It should be added to tail.
        block->_previous = _freeBlocks._tail;
    }

    _freeBlocks._tail = block;
    return block;
}

void MemoryAllocator::Chunk::RemoveFreeBlock ( Block const &block ) noexcept
{
    Block* p = block._previous;
    Block* n = block._next;

    if ( p )
        p->_next = n;

    if ( n )
        n->_previous = p;

    Block* headCases[] = { _freeBlocks._head, _freeBlocks._head->_next };
    Block* tailCases[] = { _freeBlocks._tail, _freeBlocks._tail->_previous };

    _freeBlocks._head = headCases[ static_cast<size_t> ( &block == _freeBlocks._head ) ];
    _freeBlocks._tail = tailCases[ static_cast<size_t> ( &block == _freeBlocks._tail ) ];

    delete &block;
}

//----------------------------------------------------------------------------------------------------------------------

[[maybe_unused]] void MemoryAllocator::FreeMemory ( VkDevice device,
    VkDeviceMemory memory,
    VkDeviceSize offset
) noexcept
{
    std::unique_lock<std::mutex> const lock ( _mutex );

    for ( auto& chunks : _memory )
    {
        auto const end = chunks.end ();

        for ( auto chunk = chunks.begin (); chunk != end; ++chunk )
        {
            if ( chunk->FreeMemory ( memory, offset ) )
            {
                if ( !chunk->IsUsed () )
                {
                    chunk->Destroy ( device );
                    chunks.erase ( chunk );
                }

                return;
            }
        }
    }

    LogError ( "MemoryAllocator::FreeMemory - Can't free Vulkan memory!" );
    assert ( false );
}

[[maybe_unused]] void MemoryAllocator::Init ( VkPhysicalDeviceMemoryProperties const &properties ) noexcept
{
    _properties = properties;
}

[[maybe_unused]] bool MemoryAllocator::TryAllocateMemory ( VkDeviceMemory &memory,
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

    Chunk& chunk = chunks.emplace_back ();
    return chunk.Init ( device, memoryTypeIndex ) && chunk.TryAllocateMemory ( memory, offset, requirements );
}

} // namespace android_vulkan
