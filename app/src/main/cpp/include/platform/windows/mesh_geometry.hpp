#ifndef ANDROID_VULKAN_MESH_GEOMETRY_HPP
#define ANDROID_VULKAN_MESH_GEOMETRY_HPP


#include "mesh_buffer_info.hpp"
#include <mesh_geometry_base.hpp>


namespace android_vulkan {

class MeshGeometry final : public MeshGeometryBase
{
    private:
        VkBuffer            _transferBuffer = VK_NULL_HANDLE;
        Allocation          _transferAllocation {};

        MeshBufferInfo      _meshBufferInfo {};

    public:
        explicit MeshGeometry () = default;

        MeshGeometry ( MeshGeometry const & ) = delete;
        MeshGeometry &operator = ( MeshGeometry const & ) = delete;

        MeshGeometry ( MeshGeometry && ) = delete;
        MeshGeometry &operator = ( MeshGeometry && ) = delete;

        ~MeshGeometry () override = default;

        void FreeResources ( Renderer &renderer ) noexcept;
        void FreeTransferResources ( Renderer &renderer ) noexcept;

        [[nodiscard]] MeshBufferInfo const &GetMeshBufferInfo () const noexcept;

        [[nodiscard]] bool LoadMesh ( Renderer &renderer,
            VkCommandBuffer commandBuffer,
            bool externalCommandBuffer,
            VkFence fence,
            std::string &&fileName
        ) noexcept;

        [[maybe_unused, nodiscard]] bool LoadMesh ( Renderer &renderer,
            VkCommandBuffer commandBuffer,
            bool externalCommandBuffer,
            VkFence fence,
            AbstractData data,
            uint32_t vertexCount
        ) noexcept;

        [[maybe_unused, nodiscard]] bool LoadMesh ( Renderer &renderer,
            VkCommandBuffer commandBuffer,
            bool externalCommandBuffer,
            VkFence fence,
            Indices16 indices,
            Positions positions,
            GXAABB const &bounds
        ) noexcept;

        [[maybe_unused, nodiscard]] bool LoadMesh ( Renderer &renderer,
            VkCommandBuffer commandBuffer,
            bool externalCommandBuffer,
            VkFence fence,
            Indices32 indices,
            Positions positions,
            GXAABB const &bounds
        ) noexcept;

        [[maybe_unused, nodiscard]] bool LoadMesh ( Renderer &renderer,
            VkCommandBuffer commandBuffer,
            bool externalCommandBuffer,
            VkFence fence,
            Indices16 indices,
            Positions positions,
            Vertices vertices,
            GXAABB const &bounds
        ) noexcept;

        [[maybe_unused, nodiscard]] bool LoadMesh ( Renderer &renderer,
            VkCommandBuffer commandBuffer,
            bool externalCommandBuffer,
            VkFence fence,
            Indices32 indices,
            Positions positions,
            Vertices vertices,
            GXAABB const &bounds
        ) noexcept;

    private:
        void FreeResourceInternal ( Renderer &renderer ) noexcept;

        void CommitMeshInfo ( VkDevice device,
            VkIndexType indexType,
            StreamInfo &&stream0,
            std::optional<StreamInfo> &&stream1
        ) noexcept;

        [[nodiscard]] bool GPUTransfer ( Renderer &renderer,
            VkCommandBuffer commandBuffer,
            bool externalCommandBuffer,
            VkFence fence,
            UploadJobs jobs
        ) noexcept;

        [[nodiscard]] bool LoadFromMesh2 ( Renderer &renderer,
            VkCommandBuffer commandBuffer,
            bool externalCommandBuffer,
            VkFence fence,
            std::string &&fileName
        ) noexcept;

        [[nodiscard]] bool Upload ( Renderer &renderer,
            VkCommandBuffer commandBuffer,
            bool externalCommandBuffer,
            VkFence fence,
            AbstractData indices,
            VkIndexType indexType,
            AbstractData vertexStream0,
            Vertices vertexStream1,
            uint32_t vertexCount
        ) noexcept;
};

} // namespace android_vulkan

#endif // ANDROID_VULKAN_MESH_GEOMETRY_HPP
