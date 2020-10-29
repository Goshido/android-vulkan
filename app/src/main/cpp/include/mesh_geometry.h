#ifndef MESH_GEOMETRY_H
#define MESH_GEOMETRY_H


#include <GXCommon/GXWarning.h>

GX_DISABLE_COMMON_WARNINGS

#include <string>

GX_RESTORE_WARNING_STATE

#include "renderer.h"


namespace android_vulkan {

struct BufferSyncItem final
{
    VkAccessFlags           _dstAccessMask;
    VkPipelineStageFlags    _dstStage;
    VkAccessFlags           _srcAccessMask;
    VkPipelineStageFlags    _srcStage;

    constexpr explicit BufferSyncItem ( VkAccessFlags srcAccessMask,
        VkPipelineStageFlags srcStage,
        VkAccessFlags dstAccessMask,
        VkPipelineStageFlags dstStage
    ):
        _dstAccessMask ( dstAccessMask ),
        _dstStage ( dstStage ),
        _srcAccessMask ( srcAccessMask ),
        _srcStage ( srcStage )
    {
        // NOTHING
    }
};

class MeshGeometry final
{
    private:
        [[maybe_unused]] GXAABB                                         _bounds;

        VkBuffer                                                        _vertexBuffer;
        [[maybe_unused]] VkBuffer                                       _indexBuffer;
        VkDeviceMemory                                                  _bufferMemory;

        VkBuffer                                                        _transferBuffer;
        VkDeviceMemory                                                  _transferMemory;

        uint32_t                                                        _vertexCount;
        std::string                                                     _fileName;

        static const std::map<VkBufferUsageFlags, BufferSyncItem>       _accessMapper;

    public:
        MeshGeometry ();

        MeshGeometry ( MeshGeometry const & ) = delete;
        MeshGeometry& operator = ( MeshGeometry const & ) = delete;

        MeshGeometry ( MeshGeometry && ) = delete;
        MeshGeometry& operator = ( MeshGeometry && ) = delete;

        ~MeshGeometry () = default;

        void FreeResources ( android_vulkan::Renderer &renderer );
        void FreeTransferResources ( android_vulkan::Renderer &renderer );

        [[maybe_unused]] [[nodiscard]] GXAABB const& GetBounds () const;
        [[nodiscard]] VkBuffer const& GetBuffer () const;
        [[nodiscard]] VkBuffer const& GetIndexBuffer () const;
        [[nodiscard]] std::string const& GetName () const;
        [[nodiscard]] uint32_t GetVertexCount () const;
        [[maybe_unused]] [[nodiscard]] bool IsIndexBufferPresent () const;

        // Mesh geometry is not unique if it was loaded from .mesh file.
        // Mesh geometry is unique if it was created from raw data.
        [[nodiscard]] bool IsUnique () const;

        [[nodiscard]] bool LoadMesh ( std::string &&fileName,
            VkBufferUsageFlags usage,
            android_vulkan::Renderer &renderer,
            VkCommandBuffer commandBuffer
        );

        [[maybe_unused]] bool LoadMesh ( uint8_t const* data,
            size_t size,
            uint32_t vertexCount,
            VkBufferUsageFlags usage,
            android_vulkan::Renderer &renderer,
            VkCommandBuffer commandBuffer
        );

    private:
        void FreeResourceInternal ( android_vulkan::Renderer &renderer );

        [[nodiscard]] bool LoadFromMesh ( std::string &&fileName,
            VkBufferUsageFlags usage,
            android_vulkan::Renderer &renderer,
            VkCommandBuffer commandBuffer
        );

        [[nodiscard]] bool LoadFromMesh2 ( std::string &&fileName,
            VkBufferUsageFlags usage,
            android_vulkan::Renderer &renderer,
            VkCommandBuffer commandBuffer
        );

        [[nodiscard]] bool LoadMeshInternal ( uint8_t const* data,
            size_t size,
            uint32_t vertexCount,
            VkBufferUsageFlags usage,
            android_vulkan::Renderer &renderer,
            VkCommandBuffer commandBuffer
        );
};

} // namespace android_vulkan


#endif // MESH_GEOMETRY_H
