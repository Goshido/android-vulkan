#ifndef ANDROID_VULKAN_MESH_GEOMETRY_HPP
#define ANDROID_VULKAN_MESH_GEOMETRY_HPP


#include "buffer_info.hpp"
#include "renderer.hpp"
#include "vertex_info.hpp"

GX_DISABLE_COMMON_WARNINGS

#include <span>
#include <string>

GX_RESTORE_WARNING_STATE


namespace android_vulkan {

class MeshGeometry final
{
    public:
        using AbstractData = std::span<uint8_t const>;
        using Indices = std::span<uint32_t const>;
        using Positions = std::span<GXVec3 const>;
        using Vertices = std::span<VertexInfo const>;

    private:
        struct Allocation final
        {
            VkDeviceMemory          _memory = VK_NULL_HANDLE;
            VkDeviceSize            _offset = std::numeric_limits<VkDeviceSize>::max ();
            VkDeviceSize            _range = 0U;
        };

        struct UploadJob final
        {
            VkBufferCopy            _copyInfo {};
            VkBuffer                _buffer = VK_NULL_HANDLE;
            VkBufferUsageFlags      _usage = 0U;
        };

        using UploadJobs = std::span<UploadJob const>;

    private:
        constexpr static size_t     VERTEX_BUFFER_COUNT = 2U;

        GXAABB                      _bounds {};

        VkBuffer                    _indexBuffer = VK_NULL_HANDLE;
        Allocation                  _indexAllocation {};

        VkBuffer                    _vertexBuffers[ VERTEX_BUFFER_COUNT ] { VK_NULL_HANDLE, VK_NULL_HANDLE };
        Allocation                  _vertexAllocations[ VERTEX_BUFFER_COUNT ] {};

        VkBuffer                    _transferBuffer = VK_NULL_HANDLE;
        Allocation                  _transferAllocation {};

        uint32_t                    _vertexCount = 0U;
        uint32_t                    _vertexBufferVertexCount = 0U;
        std::string                 _fileName {};

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

        // Method returns two buffers: position buffer and "rest data" buffer.
        [[nodiscard]] VkBuffer const *GetVertexBuffers () const noexcept;

        [[nodiscard]] BufferInfo GetBufferInfo () const noexcept;
        [[nodiscard]] MeshBufferInfo GetMeshBufferInfo () const noexcept;
        [[nodiscard]] uint32_t GetVertexBufferVertexCount () const noexcept;
        [[nodiscard]] VkBuffer const &GetIndexBuffer () const noexcept;
        [[nodiscard]] std::string const &GetName () const noexcept;
        [[nodiscard]] uint32_t GetVertexCount () const noexcept;

        // Mesh geometry is not unique if it was loaded from .mesh file.
        // Mesh geometry is unique if it was created from raw data.
        [[nodiscard]] bool IsUnique () const noexcept;
        void MakeUnique () noexcept;

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

        [[nodiscard]] bool LoadMesh ( Renderer &renderer,
            VkCommandBuffer commandBuffer,
            bool externalCommandBuffer,
            VkFence fence,
            Indices indices,
            Positions positions,
            GXAABB const &bounds
        ) noexcept;

        [[nodiscard]] bool LoadMesh ( Renderer &renderer,
            VkCommandBuffer commandBuffer,
            bool externalCommandBuffer,
            VkFence fence,
            Indices indices,
            Positions positions,
            Vertices vertices,
            GXAABB const &bounds
        ) noexcept;

        [[nodiscard]] constexpr static uint32_t GetVertexBufferCount () noexcept
        {
            return static_cast<uint32_t> ( VERTEX_BUFFER_COUNT );
        }

    private:
        void FreeResourceInternal ( Renderer &renderer ) noexcept;

        [[nodiscard]] bool LoadFromMesh ( Renderer &renderer,
            VkCommandBuffer commandBuffer,
            bool externalCommandBuffer,
            VkFence fence,
            std::string &&fileName
        ) noexcept;

        [[nodiscard]] bool LoadFromMesh2 ( Renderer &renderer,
            VkCommandBuffer commandBuffer,
            bool externalCommandBuffer,
            VkFence fence,
            std::string &&fileName
        ) noexcept;

        [[nodiscard]] bool UploadComplex ( Renderer &renderer,
            VkCommandBuffer commandBuffer,
            bool externalCommandBuffer,
            VkFence fence,
            AbstractData data,
            uint32_t indexCount,
            uint32_t vertexCount,
            bool hasRestData,
            bool needRepack
        ) noexcept;

        [[nodiscard]] bool UploadInternal ( Renderer &renderer,
            VkCommandBuffer commandBuffer,
            bool externalCommandBuffer,
            VkFence fence,
            UploadJobs jobs,
            AbstractData data
        ) noexcept;

        [[nodiscard]] bool UploadSimple ( Renderer &renderer,
            VkCommandBuffer commandBuffer,
            bool externalCommandBuffer,
            VkFence fence,
            AbstractData data,
            uint32_t vertexCount,
            VkBufferUsageFlags usage
        ) noexcept;
};

} // namespace android_vulkan


#endif // ANDROID_VULKAN_MESH_GEOMETRY_HPP
