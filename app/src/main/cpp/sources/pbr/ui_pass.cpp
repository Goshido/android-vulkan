#include <pbr/ui_pass.h>
#include <av_assert.h>
#include <trace.h>


namespace pbr {

namespace {

constexpr size_t MAX_VERTICES = 762600U;
constexpr size_t BUFFER_BYTES = MAX_VERTICES * sizeof ( UIVertexInfo );
constexpr size_t SCENE_IMAGE_VERTICES = 6U;

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
    if ( !_fontStorage.Init () )
        return false;

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
    _fontStorage.Destroy ( renderer );
}

bool UIPass::Execute ( android_vulkan::Renderer &renderer, VkCommandBuffer commandBuffer ) noexcept
{
    AV_TRACE ( "UI pass execute" )

    if ( !_fontStorage.UploadGPUData ( renderer, commandBuffer ) )
        return false;

    // TODO
    return true;
}

FontStorage& UIPass::GetFontStorage () noexcept
{
    return _fontStorage;
}

void UIPass::RequestEmptyUI () noexcept
{
    // TODO
}

UIPass::UIBufferResponse UIPass::RequestUIBuffer ( size_t neededVertices ) noexcept
{
    size_t const actualVertices = neededVertices + SCENE_IMAGE_VERTICES;

    if ( actualVertices > MAX_VERTICES )
    {
        android_vulkan::LogWarning ( "pbr::UIPass::RequestUIBuffer - Too many vertices was requested: %zu + %zu.",
            neededVertices,
            SCENE_IMAGE_VERTICES
        );

        AV_ASSERT ( false )
        return std::nullopt;
    }

    constexpr size_t probeIdx = 1U;
    size_t const cases[] = { 0U, _currentVertexIndex + actualVertices };
    size_t const nextIdx = cases[ static_cast<size_t> ( cases[ probeIdx ] < MAX_VERTICES ) ];

    _sceneImageVertexIndex = _currentVertexIndex;
    UIVertexBuffer const result = UIVertexBuffer ( _data + _currentVertexIndex + SCENE_IMAGE_VERTICES, neededVertices );
    _currentVertexIndex = nextIdx;

    return result;
}

bool UIPass::SetResolution ( android_vulkan::Renderer &renderer, VkRenderPass renderPass ) noexcept
{
    VkExtent2D const& resolution = renderer.GetSurfaceSize ();

    if ( ( _resolution.width == resolution.width ) & ( _resolution.height == resolution.height ) )
        return true;

    if ( !_fontStorage.SetMediaResolution ( renderer, resolution ) )
        return false;

    _program.Destroy ( renderer.GetDevice () );

    if ( !_program.Init ( renderer, renderPass, 0U, resolution ) )
        return false;

    _resolution = resolution;
    return true;
}

void UIPass::SubmitImage () noexcept
{
    // TODO
}

void UIPass::SubmitRectangle () noexcept
{
    // TODO
}

void UIPass::SubmitText ( size_t /*usedVertices*/ ) noexcept
{
    // TODO
}

} // namespace pbr
