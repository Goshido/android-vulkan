#ifndef ROTATING_MESH_MESH_GEOMETRY_H
#define ROTATING_MESH_MESH_GEOMETRY_H


#include <warning.h>

AV_DISABLE_COMMON_WARNINGS

#include <string>

AV_RESTORE_WARNING_STATE

#include <renderer.h>


namespace rotating_mesh {

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

        bool LoadMesh ( std::string &&fileName,
            VkBufferUsageFlags usage,
            android_vulkan::Renderer &renderer,
            VkCommandBuffer commandBuffer
        );

        bool LoadMesh ( const uint8_t* data,
            size_t size,
            VkBufferUsageFlags usage,
            android_vulkan::Renderer &renderer,
            VkCommandBuffer commandBuffer
        );
};

} // namespace rotating_mesh


#endif // ROTATING_MESH_MESH_GEOMETRY_H
