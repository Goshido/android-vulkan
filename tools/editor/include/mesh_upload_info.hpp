#ifndef EDITOR_MESH_UPLOAD_INFO_HPP
#define EDITOR_MESH_UPLOAD_INFO_HPP


#include "mesh_geometry_ref.hpp"

GX_DISABLE_COMMON_WARNINGS

#include <functional>
#include <optional>

GX_RESTORE_WARNING_STATE


namespace editor {

using MeshLoadResult = std::move_only_function<void ( std::optional<MeshGeometryRef> && )>;

class MeshUploadInfo final
{
    public:
        MeshGeometryRef                         _mesh {};
        android_vulkan::MeshGeometry::Info      _info {};
        MeshLoadResult                          _result = nullptr;

    public:
        MeshUploadInfo () = delete;

        MeshUploadInfo ( MeshUploadInfo const & ) = delete;
        MeshUploadInfo &operator = ( MeshUploadInfo const & ) = delete;

        MeshUploadInfo ( MeshUploadInfo && ) = default;
        MeshUploadInfo &operator = ( MeshUploadInfo && ) = default;

        explicit MeshUploadInfo ( MeshGeometryRef &&mesh,
            android_vulkan::MeshGeometry::Info &&info,
            MeshLoadResult &&result
        ) noexcept;

        ~MeshUploadInfo () = default;
};

} // namespace editor


#endif // EDITOR_MESH_UPLOAD_INFO_HPP
