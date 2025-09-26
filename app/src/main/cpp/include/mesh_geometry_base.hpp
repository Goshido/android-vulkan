#ifndef ANDROID_VULKAN_MESH_GEOMETRY_BASE_HPP
#define ANDROID_VULKAN_MESH_GEOMETRY_BASE_HPP


#include "buffer_info.hpp"

#include "renderer.hpp"
#include "vertex_info.hpp"

GX_DISABLE_COMMON_WARNINGS

#include <optional>
#include <span>
#include <string>

GX_RESTORE_WARNING_STATE


namespace android_vulkan {

class MeshGeometryBase
{
    public:
        using AbstractData = std::span<uint8_t const>;
        using Indices16 = std::span<uint16_t const>;
        using Indices32 = std::span<uint32_t const>;
        using Positions = std::span<GXVec3 const>;
        using Vertices = std::span<VertexInfo const>;

        struct BufferSyncItem final
        {
            enum class eType : size_t
            {
                NoIndexInfo = 0U,
                WithIndexInfo = 1U
            };

            VkAccessFlags           _dstAccessMask = VK_IMAGE_ASPECT_NONE;
            VkBufferUsageFlags      _usage = VK_BUFFER_USAGE_FLAG_BITS_MAX_ENUM;
        };

    protected:
        struct StreamInfo final
        {
            size_t                  _offset = 0U;
            size_t                  _range = 0U;
        };

    private:
        struct Allocation final
        {
            VkDeviceMemory          _memory = VK_NULL_HANDLE;
            VkDeviceSize            _offset = std::numeric_limits<VkDeviceSize>::max ();
            VkDeviceSize            _range = 0U;
        };

        struct UploadJob final
        {
            void const*             _data = nullptr;
            VkDeviceSize            _dstOffset = 0U;
            VkDeviceSize            _size = 0U;
        };

        using UploadJobs = std::span<UploadJob const>;

    private:
        GXAABB                      _bounds {};
        Allocation                  _gpuAllocation {};

        VkBuffer                    _transferBuffer = VK_NULL_HANDLE;
        Allocation                  _transferAllocation {};

        uint32_t                    _vertexCount = 0U;
        uint32_t                    _vertexBufferVertexCount = 0U;
        std::string                 _fileName {};

    public:
        MeshGeometryBase ( MeshGeometryBase const & ) = delete;
        MeshGeometryBase &operator = ( MeshGeometryBase const & ) = delete;

        MeshGeometryBase ( MeshGeometryBase && ) = delete;
        MeshGeometryBase &operator = ( MeshGeometryBase && ) = delete;

        void FreeResources ( Renderer &renderer ) noexcept;
        void FreeTransferResources ( Renderer &renderer ) noexcept;

        [[nodiscard]] GXAABB const &GetBounds () const noexcept;
        [[nodiscard]] uint32_t GetVertexBufferVertexCount () const noexcept;
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
            Indices16 indices,
            Positions positions,
            GXAABB const &bounds
        ) noexcept;

        [[nodiscard]] bool LoadMesh ( Renderer &renderer,
            VkCommandBuffer commandBuffer,
            bool externalCommandBuffer,
            VkFence fence,
            Indices32 indices,
            Positions positions,
            GXAABB const &bounds
        ) noexcept;

        [[nodiscard]] bool LoadMesh ( Renderer &renderer,
            VkCommandBuffer commandBuffer,
            bool externalCommandBuffer,
            VkFence fence,
            Indices16 indices,
            Positions positions,
            Vertices vertices,
            GXAABB const &bounds
        ) noexcept;

        [[nodiscard]] bool LoadMesh ( Renderer &renderer,
            VkCommandBuffer commandBuffer,
            bool externalCommandBuffer,
            VkFence fence,
            Indices32 indices,
            Positions positions,
            Vertices vertices,
            GXAABB const &bounds
        ) noexcept;

    protected:
        MeshGeometryBase () = default;
        virtual ~MeshGeometryBase () = default;

        [[nodiscard]] virtual BufferSyncItem const &GetBufferSync ( BufferSyncItem::eType type ) const noexcept = 0;

        virtual void CommitMeshInfo ( VkDevice device,
            VkIndexType indexType,
            StreamInfo &&stream0,
            std::optional<StreamInfo> &&stream1
        ) noexcept = 0;

        [[nodiscard]] virtual VkBuffer &GetDeviceBuffer () noexcept = 0;

    private:
        [[nodiscard]] bool static CreateBuffer ( Renderer &renderer,
            VkBuffer &buffer,
            Allocation &allocation,
            VkBufferCreateInfo const &createInfo,
            VkMemoryPropertyFlags memoryProperty,
            char const *name
        ) noexcept;

        void FreeResourceInternal ( Renderer &renderer ) noexcept;

        [[nodiscard]] bool GPUTransfer ( Renderer &renderer,
            VkCommandBuffer commandBuffer,
            bool externalCommandBuffer,
            VkFence fence,
            UploadJobs jobs,
            BufferSyncItem::eType bufferSyncType
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
            uint32_t indexCount,
            VkIndexType indexType,
            AbstractData vertexStream0,
            Vertices vertexStream1,
            uint32_t vertexCount
        ) noexcept;
};

} // namespace android_vulkan


#endif // ANDROID_VULKAN_MESH_GEOMETRY_BASE_HPP
