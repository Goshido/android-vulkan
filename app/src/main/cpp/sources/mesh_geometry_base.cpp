#include <precompiled_headers.hpp>
#include <mesh_geometry_base.hpp>
#include <vulkan_utils.hpp>


namespace android_vulkan {

GXAABB const &MeshGeometryBase::GetBounds () const noexcept
{
    return _bounds;
}

uint32_t MeshGeometryBase::GetVertexBufferVertexCount () const noexcept
{
    return _vertexBufferVertexCount;
}

std::string const &MeshGeometryBase::GetName () const noexcept
{
    return _fileName;
}

uint32_t MeshGeometryBase::GetVertexCount () const noexcept
{
    return _vertexCount;
}

bool MeshGeometryBase::IsUnique () const noexcept
{
    return _fileName.empty ();
}

void MeshGeometryBase::MakeUnique () noexcept
{
    _fileName.clear ();
}

bool MeshGeometryBase::CreateBuffer ( Renderer &renderer,
    VkBuffer &buffer,
    Allocation &allocation,
    VkBufferCreateInfo const &createInfo,
    VkMemoryPropertyFlags memoryProperty,
    [[maybe_unused]] char const *name
) noexcept
{
    VkDevice device = renderer.GetDevice ();

    bool const result = Renderer::CheckVkResult ( vkCreateBuffer ( device, &createInfo, nullptr, &buffer ),
        "MeshGeometryBase::CreateBuffer",
        "Can't create buffer"
    );

    if ( !result ) [[unlikely]]
        return false;

    allocation._range = createInfo.size;
    AV_SET_VULKAN_OBJECT_NAME ( device, buffer, VK_OBJECT_TYPE_BUFFER, "%s", name )

    VkMemoryRequirements memoryRequirements;
    vkGetBufferMemoryRequirements ( device, buffer, &memoryRequirements );

    return
        renderer.TryAllocateMemory ( allocation._memory,
            allocation._offset,
            memoryRequirements,
            memoryProperty,
            "Can't allocate memory (MeshGeometryBase::CreateBuffer)"
        ) &&

        Renderer::CheckVkResult (
            vkBindBufferMemory ( device, buffer, allocation._memory, allocation._offset ),
            "MeshGeometryBase::CreateBuffer",
            "Can't bind memory"
        );
}

} // namespace android_vulkan
