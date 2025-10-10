#include <precompiled_headers.hpp>
#include <av_assert.hpp>
#include <platform/android/pbr/uniform_buffer.hpp>
#include <vulkan_utils.hpp>


namespace pbr {

namespace {

constexpr size_t KILOBYTES_TO_BYTES_SHIFT = 10U;

// [2024/11/09] CL compiler does not respect [[maybe_unused]].
// See https://developercommunity.visualstudio.com/t/10711255
GX_UNUSED_BEGIN

// see https://registry.khronos.org/vulkan/specs/latest/man/html/vkCmdUpdateBuffer.html
[[maybe_unused]] constexpr size_t UPDATE_BUFFER_MAX_SIZE = 65536U;

GX_UNUSED_END

} // end of anonymous namespace

//----------------------------------------------------------------------------------------------------------------------

UniformBuffer::UniformBuffer ( eUniformSize size ) noexcept:
    _size ( static_cast<size_t> ( size ) << KILOBYTES_TO_BYTES_SHIFT )
{
    // NOTHING
}

VkBuffer UniformBuffer::Push ( VkCommandBuffer commandBuffer, void const* data, size_t size ) noexcept
{
    AV_ASSERT ( size <= _itemSize )
    AV_ASSERT ( _buffers.size () > _index )

    VkBuffer buffer = _buffers[ _index++ ];
    vkCmdUpdateBuffer ( commandBuffer, buffer, 0U, size, data );
    return buffer;
}

void UniformBuffer::Reset () noexcept
{
    _index = 0U;
}

size_t UniformBuffer::GetAvailableItemCount () const noexcept
{
    return _buffers.size () - _index;
}

VkBuffer UniformBuffer::GetBuffer ( size_t bufferIndex ) const noexcept
{
    AV_ASSERT ( bufferIndex < _buffers.size () )
    return _buffers[ bufferIndex ];
}

bool UniformBuffer::Init ( android_vulkan::Renderer &renderer, size_t itemSize, char const* name ) noexcept
{
    AV_ASSERT ( itemSize > 0U )
    AV_ASSERT ( itemSize <= renderer.GetMaxUniformBufferRange () )
    AV_ASSERT ( itemSize <= UPDATE_BUFFER_MAX_SIZE )

    VkBufferCreateInfo const bufferInfo
    {
        .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0U,
        .size = static_cast<VkDeviceSize> ( itemSize ),
        .usage = AV_VK_FLAG ( VK_BUFFER_USAGE_TRANSFER_DST_BIT ) | AV_VK_FLAG ( VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT ),
        .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
        .queueFamilyIndexCount = 0U,
        .pQueueFamilyIndices = nullptr
    };

    VkMemoryRequirements requirements;

    if ( !ResolveMemoryRequirements ( renderer, requirements, bufferInfo ) ) [[unlikely]]
        return false;

    auto const alignment = static_cast<size_t> ( requirements.alignment );
    size_t const alignMultiplier = ( itemSize + alignment - 1U ) / alignment;
    size_t const alignedBlockSize = alignMultiplier * alignment;
    requirements.size = static_cast<VkDeviceSize> ( alignedBlockSize );

    _index = 0U;
    return AllocateBuffers ( renderer, requirements, _size / alignedBlockSize, bufferInfo, name );
}

void UniformBuffer::Destroy ( android_vulkan::Renderer &renderer ) noexcept
{
    VkDevice device = renderer.GetDevice ();

    for ( VkBuffer buffer : _buffers )
        vkDestroyBuffer ( device, buffer, nullptr );

    _buffers.clear ();

    if ( _memory == VK_NULL_HANDLE )
        return;

    renderer.FreeMemory ( _memory, _offset );
    _memory = VK_NULL_HANDLE;
    _offset = std::numeric_limits<VkDeviceSize>::max ();
}

bool UniformBuffer::AllocateBuffers ( android_vulkan::Renderer &renderer,
    VkMemoryRequirements &requirements,
    size_t itemCount,
    VkBufferCreateInfo const &bufferInfo,
    [[maybe_unused]] char const* name
) noexcept
{
    VkDevice device = renderer.GetDevice ();
    _buffers.reserve ( itemCount );

    VkDeviceSize const alignedBlockSize = requirements.size;
    requirements.size *= static_cast<VkDeviceSize> ( itemCount );

    bool result = renderer.TryAllocateMemory ( _memory,
        _offset,
        requirements,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
        "Can't allocate GPU memory (pbr::UniformBuffer::AllocateBuffers)"
    );

    if ( !result ) [[unlikely]]
        return false;

    VkDeviceSize offset = _offset;

    for ( size_t i = 0U; i < itemCount; ++i )
    {
        VkBuffer buffer;

        result = android_vulkan::Renderer::CheckVkResult (
            vkCreateBuffer ( device, &bufferInfo, nullptr, &buffer ),
            "pbr::UniformBuffer::AllocateBuffers",
            "Can't create uniform buffer"
        );

        if ( !result ) [[unlikely]]
            return false;

        AV_SET_VULKAN_OBJECT_NAME ( device, buffer, VK_OBJECT_TYPE_BUFFER, "%s #%zu", name, i )

        vkGetBufferMemoryRequirements ( device, buffer, &requirements );

        result = android_vulkan::Renderer::CheckVkResult (
            vkBindBufferMemory ( device, buffer, _memory, std::exchange ( offset, offset + alignedBlockSize ) ),
            "pbr::UniformBuffer::AllocateBuffers",
            "Can't bind uniform buffer memory"
        );

        if ( !result ) [[unlikely]]
            return false;

        _buffers.push_back ( buffer );
    }

    _itemSize = static_cast<size_t> ( bufferInfo.size );
    return true;
}

bool UniformBuffer::ResolveMemoryRequirements ( android_vulkan::Renderer &renderer,
    VkMemoryRequirements &requirements,
    VkBufferCreateInfo const &bufferInfo
) noexcept
{
    VkDevice device = renderer.GetDevice ();
    VkBuffer buffer;

    bool const result = android_vulkan::Renderer::CheckVkResult (
        vkCreateBuffer ( device, &bufferInfo, nullptr, &buffer ),
        "pbr::UniformBuffer::ResolveAlignment",
        "Can't create uniform buffer"
    );

    if ( !result ) [[unlikely]]
        return false;

    AV_SET_VULKAN_OBJECT_NAME ( device, buffer, VK_OBJECT_TYPE_BUFFER, "Uniform buffer probe" )

    vkGetBufferMemoryRequirements ( device, buffer, &requirements );
    vkDestroyBuffer ( device, buffer, nullptr );
    return true;
}

} // namespace pbr
