#ifndef EDITOR_MESH_GEOMETRY_REF_HPP
#define EDITOR_MESH_GEOMETRY_REF_HPP


#include <platform/windows/mesh_geometry.hpp>

GX_DISABLE_COMMON_WARNINGS

#include <memory>

GX_RESTORE_WARNING_STATE


namespace editor {

using MeshGeometryRef = std::shared_ptr<android_vulkan::MeshGeometry>;

} // namespace editor


#endif // EDITOR_MESH_GEOMETRY_REF_HPP
