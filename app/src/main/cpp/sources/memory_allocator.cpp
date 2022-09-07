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

bool MemoryAllocator::Chunk::TryAllocateMemory ( VkDeviceMemory &memory,
    VkDeviceSize &offset,
    VkMemoryRequirements const &requirements
) noexcept
{
    if ( !_freeBlocks._tail || requirements.size > _freeBlocks._tail->_size )
        return false;

    for ( Block* block = _freeBlocks._head; block; block = block->_freeNext )
    {
        Offset const freeBlockOffset = block->_offset;
        auto const size = static_cast<size_t> ( block->_size );
        size_t space = size;
        auto* ptr = reinterpret_cast<void*> ( freeBlockOffset );

        ptr = std::align ( static_cast<size_t> ( requirements.alignment ),
            static_cast<size_t> ( requirements.size ),
            ptr,
            space
        );

        if ( !ptr )
            continue;

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

        block._freePrevious = freeBlock->_freePrevious;
        block._freeNext = freeBlock;
        freeBlock->_freePrevious = &block;

        if ( freeBlock == _freeBlocks._head )
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

[[maybe_unused]] void MemoryAllocator::FreeMemory ( VkDevice device,
    VkDeviceMemory memory,
    VkDeviceSize offset
) noexcept
{
    std::unique_lock<std::mutex> const lock ( _mutex );

    auto findResult = _chunkMap.find ( memory );
    assert ( findResult != _chunkMap.end () );

    auto [chunks, chunk] = findResult->second;
    chunk->FreeMemory ( offset );

    if ( chunk->IsUsed () )
        return;

    chunk->Destroy ( device );
    chunks->erase ( chunk );
    _chunkMap.erase ( findResult );
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

    Chunk& chunk = chunks.emplace_front ();

    bool const result = chunk.Init ( device, memoryTypeIndex ) &&
        chunk.TryAllocateMemory ( memory, offset, requirements );

    if ( !result )
        return false;

    _chunkMap.emplace ( memory, std::make_pair ( &chunks, chunks.begin () ) );
    return true;
}

} // namespace android_vulkan