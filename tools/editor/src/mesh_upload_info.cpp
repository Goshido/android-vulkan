#include <precompiled_headers.hpp>
#include <mesh_upload_info.hpp>


namespace editor {

MeshUploadInfo::MeshUploadInfo ( MeshGeometryRef &&mesh,
    android_vulkan::MeshGeometry::Info &&info,
    MeshLoadResult &&result
) noexcept:
    _mesh ( std::move ( mesh ) ),
    _info ( std::move ( info ) ),
    _result ( std::move ( result ) )
{
    // NOTHING
}

} // namespace editor
