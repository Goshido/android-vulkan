#ifndef ANDROID_VULKAN_MESH_GEOMETRY_BASE_HPP
#define ANDROID_VULKAN_MESH_GEOMETRY_BASE_HPP


#include "buffer_info.hpp"
#include "renderer.hpp"
#include "vertex_info.hpp"

GX_DISABLE_COMMON_WARNINGS

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

    protected:
        struct StreamInfo final
        {
            size_t                  _offset = 0U;
            size_t                  _range = 0U;
        };

        struct Allocation final
        {
            VkDeviceMemory          _memory = VK_NULL_HANDLE;
            VkDeviceSize            _offset = std::numeric_limits<VkDeviceSize>::max ();
            VkDeviceSize            _range = 0U;
        };

        struct UploadJob final
        {
            void const*     _data = nullptr;
            VkDeviceSize    _dstOffset = 0U;
            VkDeviceSize    _size = 0U;
        };

        using UploadJobs = std::span<UploadJob const>;

    protected:
        constexpr static size_t INDEX16_LIMIT = 1U << 16U;
        constexpr static size_t const INDEX_SIZES[] = { sizeof ( uint16_t ), sizeof ( uint32_t ) };

    protected:
        GXAABB                      _bounds {};
        Allocation                  _gpuAllocation {};

        uint32_t                    _vertexCount = 0U;
        uint32_t                    _vertexBufferVertexCount = 0U;
        std::string                 _fileName {};

    public:
        MeshGeometryBase ( MeshGeometryBase const & ) = delete;
        MeshGeometryBase &operator = ( MeshGeometryBase const & ) = delete;

        MeshGeometryBase ( MeshGeometryBase && ) = delete;
        MeshGeometryBase &operator = ( MeshGeometryBase && ) = delete;

        [[nodiscard]] GXAABB const &GetBounds () const noexcept;
        [[nodiscard]] uint32_t GetVertexBufferVertexCount () const noexcept;
        [[nodiscard]] std::string const &GetName () const noexcept;
        [[nodiscard]] uint32_t GetVertexCount () const noexcept;

        // Mesh geometry is not unique if it was loaded from .mesh file.
        // Mesh geometry is unique if it was created from raw data.
        [[nodiscard]] bool IsUnique () const noexcept;
        void MakeUnique () noexcept;

    protected:
        explicit MeshGeometryBase () = default;
        virtual ~MeshGeometryBase () = default;

        [[nodiscard]] bool static CreateBuffer ( Renderer &renderer,
            VkBuffer &buffer,
            Allocation &allocation,
            VkBufferCreateInfo const &createInfo,
            VkMemoryPropertyFlags memoryProperty,
            char const *name
        ) noexcept;
};

} // namespace android_vulkan


#endif // ANDROID_VULKAN_MESH_GEOMETRY_BASE_HPP
