#include <memory_allocator.h>
#include <renderer.h>
#include <vulkan_utils.h>


namespace android_vulkan {

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

    _blockChain.emplace_front ( Kind::Free, BYTES_PER_CHUNK );
    _freeBlocks.emplace_front ( static_cast<Offset> ( 0U ), BYTES_PER_CHUNK );
    return true;
}

void MemoryAllocator::Chunk::Destroy ( VkDevice device ) noexcept
{
    if ( _memory == VK_NULL_HANDLE )
        return;

    vkFreeMemory ( device, _memory, nullptr );
    _memory = VK_NULL_HANDLE;
    AV_UNREGISTER_DEVICE_MEMORY ( "MemoryAllocator::Chunk::_memory" )

    _freeBlocks.clear ();
    _freeBlocks.shrink_to_fit ();
    _usedBlocks.clear ();
    _blockChain.clear ();
}

bool MemoryAllocator::Chunk::TryAllocateMemory ( VkDeviceMemory &memory,
    VkDeviceSize &offset,
    VkMemoryRequirements const &requirements
) noexcept
{
    auto const end = _freeBlocks.cend ();

    for ( auto freeBlock = _freeBlocks.cbegin (); freeBlock != end; ++freeBlock )
    {
        auto const [chunkOffset, chunkSize] = *freeBlock;

        auto const size = static_cast<size_t> ( chunkSize );
        size_t space = size;
        auto* ptr = reinterpret_cast<void*> ( chunkOffset );

        ptr = std::align ( static_cast<size_t> ( requirements.alignment ),
            static_cast<size_t> ( requirements.size ),
            ptr,
            space
        );

        if ( !ptr )
            continue;

        auto const resultOffset = reinterpret_cast<VkDeviceSize> ( ptr );
        _usedBlocks.emplace ( resultOffset, requirements.size );
        _freeBlocks.erase ( freeBlock );

        if ( size_t const before = size - space; before )
        {
            // There is an empty space in front of occupied memory block because of alignment.
            AppendFreeBlock ( chunkOffset, static_cast<Size> ( before ) );
        }

        if ( size_t const left = space - static_cast<size_t> ( requirements.size ); left )
        {
            // There is an empty space after occupied memory block.
            AppendFreeBlock ( resultOffset + requirements.size, static_cast<Size> ( left ) );
        }

        offset = resultOffset;
        memory = _memory;
        return true;
    }

    return false;
}

void MemoryAllocator::Chunk::AppendFreeBlock ( Offset offset, Size size ) noexcept
{
    auto const findResult = std::find_if ( _freeBlocks.cbegin (),
        _freeBlocks.cend (),

        [ size ] ( BlockInfo const &block ) noexcept -> bool {
            return size <= block.second;
        }
    );

    _freeBlocks.emplace ( findResult, offset, size );
}

//----------------------------------------------------------------------------------------------------------------------

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
