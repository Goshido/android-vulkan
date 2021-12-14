#ifndef MESH_GEOMETRY_H
#define MESH_GEOMETRY_H


#include <GXCommon/GXWarning.h>

GX_DISABLE_COMMON_WARNINGS

#include <string>

GX_RESTORE_WARNING_STATE

#include "renderer.h"


namespace android_vulkan {

class MeshGeometry final
{
    private:
        GXAABB              _bounds;

        VkBuffer            _indexBuffer;
        VkBuffer            _vertexBuffer;
        VkDeviceMemory      _bufferMemory;

        VkBuffer            _transferBuffer;
        VkDeviceMemory      _transferMemory;

        uint32_t            _vertexCount;
        std::string         _fileName;

    public:
        MeshGeometry () noexcept;

        MeshGeometry ( MeshGeometry const & ) = delete;
        MeshGeometry& operator = ( MeshGeometry const & ) = delete;

        MeshGeometry ( MeshGeometry && ) = delete;
        MeshGeometry& operator = ( MeshGeometry && ) = delete;

        ~MeshGeometry () = default;

        void FreeResources ( VkDevice device ) noexcept;
        void FreeTransferResources ( VkDevice device ) noexcept;

        [[nodiscard]] GXAABB const& GetBounds () const noexcept;
        [[nodiscard]] VkBuffer const& GetVertexBuffer () const noexcept;
        [[nodiscard]] VkBuffer const& GetIndexBuffer () const noexcept;
        [[nodiscard]] std::string const& GetName () const noexcept;
        [[nodiscard]] uint32_t GetVertexCount () const noexcept;
        [[maybe_unused, nodiscard]] bool IsIndexBufferPresent () const noexcept;

        // Mesh geometry is not unique if it was loaded from .mesh file.
        // Mesh geometry is unique if it was created from raw data.
        [[nodiscard]] bool IsUnique () const noexcept;

        [[nodiscard]] bool LoadMesh ( std::string &&fileName,
            VkBufferUsageFlags usage,
            Renderer &renderer,
            VkCommandBuffer commandBuffer
        ) noexcept;

        [[maybe_unused, nodiscard]] bool LoadMesh ( uint8_t const* data,
            size_t size,
            uint32_t vertexCount,
            VkBufferUsageFlags usage,
            Renderer &renderer,
            VkCommandBuffer commandBuffer
        ) noexcept;

        [[nodiscard]] bool LoadMesh ( uint8_t const* vertexData,
            size_t vertexDataSize,
            uint32_t const* indices,
            uint32_t indexCount,
            GXAABB const &bounds,
            Renderer &renderer,
            VkCommandBuffer commandBuffer
        ) noexcept;

    private:
        void FreeResourceInternal ( VkDevice device ) noexcept;

        [[nodiscard]] bool LoadFromMesh ( std::string &&fileName,
            VkBufferUsageFlags usage,
            Renderer &renderer,
            VkCommandBuffer commandBuffer
        ) noexcept;

        [[nodiscard]] bool LoadFromMesh2 ( std::string &&fileName,
            Renderer &renderer,
            VkCommandBuffer commandBuffer
        ) noexcept;

        [[nodiscard]] bool UploadComplex ( uint8_t const* data,
            size_t vertexDataSize,
            uint32_t indexCount,
            Renderer &renderer,
            VkCommandBuffer commandBuffer
        ) noexcept;

        [[nodiscard]] bool UploadInternal ( size_t numUploads,
            VkBufferCopy const* copyJobs,
            VkBufferUsageFlags const* usages,
            VkBuffer const* dstBuffers,
            uint8_t const* data,
            size_t dataSize,
            Renderer &renderer,
            VkCommandBuffer commandBuffer
        ) noexcept;

        [[nodiscard]] bool UploadSimple ( uint8_t const* data,
            size_t size,
            uint32_t vertexCount,
            VkBufferUsageFlags usage,
            Renderer &renderer,
            VkCommandBuffer commandBuffer
        ) noexcept;
};

} // namespace android_vulkan


#endif // MESH_GEOMETRY_H
