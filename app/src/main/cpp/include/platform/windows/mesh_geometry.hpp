#ifndef ANDROID_VULKAN_MESH_GEOMETRY_HPP
#define ANDROID_VULKAN_MESH_GEOMETRY_HPP


#include "mesh_buffer_info.hpp"
#include <mesh_geometry_base.hpp>


namespace android_vulkan {

class MeshGeometry final : public MeshGeometryBase
{
    public:
        struct Info final
        {
            VkIndexType                     _indexType = VK_INDEX_TYPE_UINT32;
            StreamInfo                      _stream0 {};
            std::optional<StreamInfo>       _stream1 = std::nullopt;

            // FUCK replace by array
            std::vector<UploadJob>          _jobs {};
        };

        using LoadResult = std::optional<Info>;

    private:
        VkBuffer                            _transferBuffer = VK_NULL_HANDLE;
        Allocation                          _transferAllocation {};

        MeshBufferInfo                      _meshBufferInfo {};

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

        [[nodiscard]] LoadResult LoadMesh ( Renderer &renderer, std::string &&fileName ) noexcept;
        [[nodiscard]] LoadResult LoadMesh ( Renderer &renderer, std::string_view fileName ) noexcept;

        [[maybe_unused, nodiscard]] LoadResult LoadMesh ( Renderer &renderer,
            AbstractData data,
            uint32_t vertexCount
        ) noexcept;

        [[maybe_unused, nodiscard]] LoadResult LoadMesh ( Renderer &renderer,
            Indices16 indices,
            Positions positions,
            GXAABB const &bounds
        ) noexcept;

        [[maybe_unused, nodiscard]] LoadResult LoadMesh ( Renderer &renderer,
            Indices32 indices,
            Positions positions,
            GXAABB const &bounds
        ) noexcept;

        [[maybe_unused, nodiscard]] LoadResult LoadMesh ( Renderer &renderer,
            Indices16 indices,
            Positions positions,
            Vertices vertices,
            GXAABB const &bounds
        ) noexcept;

        [[maybe_unused, nodiscard]] LoadResult LoadMesh ( Renderer &renderer,
            Indices32 indices,
            Positions positions,
            Vertices vertices,
            GXAABB const &bounds
        ) noexcept;

        [[nodiscard]] bool UploadToGPU ( Renderer &renderer,
            VkCommandBuffer commandBuffer,
            VkFence fence,
            Info &&info
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

        [[nodiscard]] LoadResult LoadFromMesh2 ( Renderer &renderer, std::string &&fileName ) noexcept;

        [[nodiscard]] LoadResult Upload ( Renderer &renderer,
            AbstractData indices,
            VkIndexType indexType,
            AbstractData vertexStream0,
            Vertices vertexStream1,
            uint32_t vertexCount
        ) noexcept;

        [[nodiscard]] bool CreateStagingBuffer ( Renderer &renderer, UploadJobs jobs ) noexcept;
};

} // namespace android_vulkan

#endif // ANDROID_VULKAN_MESH_GEOMETRY_HPP
