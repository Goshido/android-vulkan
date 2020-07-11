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
        VkBuffer                                                        _buffer;
        VkDeviceMemory                                                  _bufferMemory;

        VkBuffer                                                        _transferBuffer;
        VkDeviceMemory                                                  _transferMemory;

        uint32_t                                                        _vertexCount;

        std::string                                                     _fileName;

        static const std::map<VkBufferUsageFlags, BufferSyncItem>       _accessMapper;

    public:
        MeshGeometry ();
        ~MeshGeometry () = default;

        MeshGeometry ( MeshGeometry &other ) = delete;
        MeshGeometry ( MeshGeometry &&other ) = delete;

        MeshGeometry& operator = ( MeshGeometry &other ) = delete;
        MeshGeometry& operator = ( MeshGeometry &&other ) = delete;

        void FreeResources ( android_vulkan::Renderer &renderer );
        void FreeTransferResources ( android_vulkan::Renderer &renderer );

        const VkBuffer& GetBuffer () const;
        uint32_t GetVertexCount () const;

        bool LoadMesh ( std::string &&fileName,
            VkBufferUsageFlags usage,
            android_vulkan::Renderer &renderer,
            VkCommandBuffer commandBuffer
        );

        [[maybe_unused]] bool LoadMesh ( const uint8_t* data,
            size_t size,
            uint32_t vertexCount,
            VkBufferUsageFlags usage,
            android_vulkan::Renderer &renderer,
            VkCommandBuffer commandBuffer
        );

    private:
        void FreeResourceInternal ( android_vulkan::Renderer &renderer );

        bool LoadMeshInternal ( const uint8_t* data,
            size_t size,
            uint32_t vertexCount,
            VkBufferUsageFlags usage,
            android_vulkan::Renderer &renderer,
            VkCommandBuffer commandBuffer
        );
};

} // namespace android_vulkan


#endif // MESH_GEOMETRY_H
