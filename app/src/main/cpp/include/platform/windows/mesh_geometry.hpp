#ifndef ANDROID_VULKAN_MESH_GEOMETRY_HPP
#define ANDROID_VULKAN_MESH_GEOMETRY_HPP


#include "mesh_buffer_info.hpp"
#include <mesh_geometry_base.hpp>


namespace android_vulkan {

class MeshGeometry final : public MeshGeometryBase
{
    private:
        MeshBufferInfo      _meshBufferInfo {};

    public:
        MeshGeometry () = default;

        MeshGeometry ( MeshGeometry const & ) = delete;
        MeshGeometry &operator = ( MeshGeometry const & ) = delete;

        MeshGeometry ( MeshGeometry && ) = delete;
        MeshGeometry &operator = ( MeshGeometry && ) = delete;

        ~MeshGeometry () override = default;

        [[nodiscard]] MeshBufferInfo const &GetMeshBufferInfo () const noexcept;

    private:
        [[nodiscard]] BufferSyncItem const &GetBufferSync ( BufferSyncItem::eType type ) const noexcept override;

        void CommitMeshInfo ( VkDevice device,
            VkIndexType indexType,
            StreamInfo &&stream0,
            std::optional<StreamInfo> &&stream1
        ) noexcept override;

        [[nodiscard]] VkBuffer &GetDeviceBuffer () noexcept override;
};

} // namespace android_vulkan

#endif // ANDROID_VULKAN_MESH_GEOMETRY_HPP
