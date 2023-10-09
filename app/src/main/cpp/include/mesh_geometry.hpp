#ifndef ANDROID_VULKAN_MESH_GEOMETRY_HPP
#define ANDROID_VULKAN_MESH_GEOMETRY_HPP


#include "renderer.hpp"

GX_DISABLE_COMMON_WARNINGS

#include <string>

GX_RESTORE_WARNING_STATE


namespace android_vulkan {

class MeshGeometry final
{
    private:
        GXAABB              _bounds {};

        VkBuffer            _indexBuffer = VK_NULL_HANDLE;
        VkDeviceMemory      _indexBufferMemory = VK_NULL_HANDLE;
        VkDeviceSize        _indexBufferOffset = std::numeric_limits<VkDeviceSize>::max ();

        VkBuffer            _vertexBuffer = VK_NULL_HANDLE;
        VkDeviceMemory      _vertexBufferMemory = VK_NULL_HANDLE;
        VkDeviceSize        _vertexBufferOffset = std::numeric_limits<VkDeviceSize>::max ();

        VkBuffer            _transferBuffer = VK_NULL_HANDLE;
        VkDeviceMemory      _transferBufferMemory = VK_NULL_HANDLE;
        VkDeviceSize        _transferBufferOffset = std::numeric_limits<VkDeviceSize>::max ();

        uint32_t            _vertexCount = 0U;
        std::string         _fileName {};

    public:
        MeshGeometry () = default;

        MeshGeometry ( MeshGeometry const & ) = delete;
        MeshGeometry &operator = ( MeshGeometry const & ) = delete;

        MeshGeometry ( MeshGeometry && ) = delete;
        MeshGeometry &operator = ( MeshGeometry && ) = delete;

        ~MeshGeometry () = default;

        void FreeResources ( Renderer &renderer ) noexcept;
        void FreeTransferResources ( Renderer &renderer ) noexcept;

        [[nodiscard]] GXAABB const &GetBounds () const noexcept;
        [[nodiscard]] VkBuffer const &GetVertexBuffer () const noexcept;
        [[nodiscard]] VkBuffer const &GetIndexBuffer () const noexcept;
        [[nodiscard]] std::string const &GetName () const noexcept;
        [[nodiscard]] uint32_t GetVertexCount () const noexcept;
        [[maybe_unused, nodiscard]] bool IsIndexBufferPresent () const noexcept;

        // Mesh geometry is not unique if it was loaded from .mesh file.
        // Mesh geometry is unique if it was created from raw data.
        [[nodiscard]] bool IsUnique () const noexcept;

        [[nodiscard]] bool LoadMesh ( std::string &&fileName,
            VkBufferUsageFlags usage,
            Renderer &renderer,
            VkCommandBuffer commandBuffer,
            VkFence fence
        ) noexcept;

        [[maybe_unused, nodiscard]] bool LoadMesh ( uint8_t const* data,
            size_t size,
            uint32_t vertexCount,
            VkBufferUsageFlags usage,
            Renderer &renderer,
            VkCommandBuffer commandBuffer,
            VkFence fence
        ) noexcept;

        [[nodiscard]] bool LoadMesh ( uint8_t const* vertexData,
            size_t vertexDataSize,
            uint32_t const* indices,
            uint32_t indexCount,
            GXAABB const &bounds,
            Renderer &renderer,
            VkCommandBuffer commandBuffer,
            VkFence fence
        ) noexcept;

    private:
        void FreeResourceInternal ( Renderer &renderer ) noexcept;

        [[nodiscard]] bool LoadFromMesh ( std::string &&fileName,
            VkBufferUsageFlags usage,
            Renderer &renderer,
            VkCommandBuffer commandBuffer,
            VkFence fence
        ) noexcept;

        [[nodiscard]] bool LoadFromMesh2 ( std::string &&fileName,
            Renderer &renderer,
            VkCommandBuffer commandBuffer,
            VkFence fence
        ) noexcept;

        [[nodiscard]] bool UploadComplex ( uint8_t const* data,
            size_t vertexDataSize,
            uint32_t indexCount,
            Renderer &renderer,
            VkCommandBuffer commandBuffer,
            VkFence fence
        ) noexcept;

        [[nodiscard]] bool UploadInternal ( size_t numUploads,
            VkBufferCopy const* copyJobs,
            VkBufferUsageFlags const* usages,
            VkBuffer const* dstBuffers,
            uint8_t const* data,
            size_t dataSize,
            Renderer &renderer,
            VkCommandBuffer commandBuffer,
            VkFence fence
        ) noexcept;

        [[nodiscard]] bool UploadSimple ( uint8_t const* data,
            size_t size,
            uint32_t vertexCount,
            VkBufferUsageFlags usage,
            Renderer &renderer,
            VkCommandBuffer commandBuffer,
            VkFence fence
        ) noexcept;
};

} // namespace android_vulkan


#endif // ANDROID_VULKAN_MESH_GEOMETRY_HPP
