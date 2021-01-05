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
        MeshGeometry ();

        MeshGeometry ( MeshGeometry const & ) = delete;
        MeshGeometry& operator = ( MeshGeometry const & ) = delete;

        MeshGeometry ( MeshGeometry && ) = delete;
        MeshGeometry& operator = ( MeshGeometry && ) = delete;

        ~MeshGeometry () = default;

        void FreeResources ( Renderer &renderer );
        void FreeTransferResources ( Renderer &renderer );

        [[nodiscard]] GXAABB const& GetBounds () const;
        [[nodiscard]] VkBuffer const& GetVertexBuffer () const;
        [[nodiscard]] VkBuffer const& GetIndexBuffer () const;
        [[nodiscard]] std::string const& GetName () const;
        [[nodiscard]] uint32_t GetVertexCount () const;
        [[maybe_unused]] [[nodiscard]] bool IsIndexBufferPresent () const;

        // Mesh geometry is not unique if it was loaded from .mesh file.
        // Mesh geometry is unique if it was created from raw data.
        [[nodiscard]] bool IsUnique () const;

        [[nodiscard]] bool LoadMesh ( std::string &&fileName,
            VkBufferUsageFlags usage,
            Renderer &renderer,
            VkCommandBuffer commandBuffer
        );

        [[maybe_unused]] [[nodiscard]] bool LoadMesh ( uint8_t const* data,
            size_t size,
            uint32_t vertexCount,
            VkBufferUsageFlags usage,
            Renderer &renderer,
            VkCommandBuffer commandBuffer
        );

        [[maybe_unused]] [[nodiscard]] bool LoadMesh ( uint8_t const* vertexData,
            size_t vertexDataSize,
            uint32_t const* indices,
            uint32_t indexCount,
            GXAABB const &bounds,
            Renderer &renderer,
            VkCommandBuffer commandBuffer
        );

    private:
        void FreeResourceInternal ( Renderer &renderer );

        [[nodiscard]] bool LoadFromMesh ( std::string &&fileName,
            VkBufferUsageFlags usage,
            Renderer &renderer,
            VkCommandBuffer commandBuffer
        );

        [[nodiscard]] bool LoadFromMesh2 ( std::string &&fileName,
            Renderer &renderer,
            VkCommandBuffer commandBuffer
        );

        [[nodiscard]] bool UploadComplex ( uint8_t const* data,
            size_t vertexDataSize,
            uint32_t indexCount,
            Renderer &renderer,
            VkCommandBuffer commandBuffer
        );

        [[nodiscard]] bool UploadInternal ( size_t numUploads,
            VkBufferCopy const* copyJobs,
            VkBufferUsageFlags const* usages,
            VkBuffer const* dstBuffers,
            uint8_t const* data,
            size_t dataSize,
            Renderer &renderer,
            VkCommandBuffer commandBuffer
        );

        [[nodiscard]] bool UploadSimple ( uint8_t const* data,
            size_t size,
            uint32_t vertexCount,
            VkBufferUsageFlags usage,
            Renderer &renderer,
            VkCommandBuffer commandBuffer
        );
};

} // namespace android_vulkan


#endif // MESH_GEOMETRY_H
