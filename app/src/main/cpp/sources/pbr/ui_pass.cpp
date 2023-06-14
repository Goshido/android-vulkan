#include <pbr/ui_pass.h>


namespace pbr {

namespace {

constexpr size_t MAX_VERTICES = 762600U;
constexpr size_t BUFFER_BYTES = MAX_VERTICES * sizeof ( UIVertexInfo );

} // end of anonymous namespace

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

    if ( !result )
        return false;

    AV_REGISTER_BUFFER ( _name )

    VkMemoryRequirements memoryRequirements;
    vkGetBufferMemoryRequirements ( device, _buffer, &memoryRequirements );

    result = renderer.TryAllocateMemory ( _memory,
        _memoryOffset,
        memoryRequirements,
        memoryProperties,
        ( std::string ( "pbr::UIPass::Init - Can't allocate device memory: " ) + _name ).c_str ()
    );

    if ( !result )
        return false;

    AV_REGISTER_DEVICE_MEMORY ( _name )

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
        AV_UNREGISTER_BUFFER ( _name )
    }

    if ( _memory == VK_NULL_HANDLE )
        return;

    renderer.FreeMemory ( _memory, _memoryOffset );
    _memory = VK_NULL_HANDLE;
    _memoryOffset = 0U;
    AV_UNREGISTER_DEVICE_MEMORY ( _name )
}

//----------------------------------------------------------------------------------------------------------------------

bool UIPass::Init ( android_vulkan::Renderer &renderer ) noexcept
{
    constexpr auto stagingProps = static_cast<VkMemoryPropertyFlags> (
        AV_VK_FLAG ( VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT ) | AV_VK_FLAG ( VK_MEMORY_PROPERTY_HOST_COHERENT_BIT )
    );

    if ( !_staging.Init ( renderer, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, stagingProps, "pbr::UIPass::_staging" ) )
        return false;

    void* data;

    bool const result = renderer.MapMemory ( data,
        _staging._memory,
        _staging._memoryOffset,
        "pbr::UIPass::Init",
        "Can't map memory"
    );

    if ( !result )
        return false;

    _data = static_cast<UIVertexInfo*> ( data );

    return _vertex.Init ( renderer,
        AV_VK_FLAG ( VK_BUFFER_USAGE_TRANSFER_DST_BIT ) | AV_VK_FLAG ( VK_BUFFER_USAGE_VERTEX_BUFFER_BIT ),
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
        "pbr::UIPass::_vertex"
    );
}

void UIPass::Destroy ( android_vulkan::Renderer &renderer ) noexcept
{
    _program.Destroy ( renderer.GetDevice () );

    if ( _data )
    {
        renderer.UnmapMemory ( _staging._memory );
        _data = nullptr;
    }

    _staging.Destroy ( renderer );
    _vertex.Destroy ( renderer );
}

bool UIPass::SetResolution ( android_vulkan::Renderer &renderer, VkRenderPass renderPass ) noexcept
{
    VkExtent2D const& resolution = renderer.GetSurfaceSize ();

    if ( ( _resolution.width == resolution.width ) & ( _resolution.height == resolution.height ) )
        return true;

    _program.Destroy ( renderer.GetDevice () );

    if ( !_program.Init ( renderer, renderPass, 0U, resolution ) )
        return false;

    _resolution = resolution;
    return true;
}

} // namespace pbr
