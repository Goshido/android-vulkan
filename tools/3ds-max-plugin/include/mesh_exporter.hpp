#ifndef AVP_MESH_EXPORTER_HPP
#define AVP_MESH_EXPORTER_HPP


#include <GXCommon/GXWarning.hpp>

GX_DISABLE_COMMON_WARNINGS

#include <custcont.h>
#include <vector>
#include <IGame/IGameObject.h>

GX_RESTORE_WARNING_STATE


namespace avp {

class MeshExporter final
{
    private:
        std::vector<Point3>     _bitangents {};
        std::vector<Point3>     _normals {};
        std::vector<Point3>     _tangents {};

        std::vector<Point3>     _positions {};
        std::vector<Point2>     _uvs {};

        HWND                    _parent = nullptr;

    public:
        MeshExporter () = delete;

        MeshExporter ( MeshExporter const & ) = delete;
        MeshExporter &operator = ( MeshExporter const & ) = delete;

        MeshExporter ( MeshExporter && ) = delete;
        MeshExporter &operator = ( MeshExporter && ) = delete;

        explicit MeshExporter ( HWND parent, MSTR const &path, bool exportInCurrentPose ) noexcept;

        ~MeshExporter () = default;

    private:
        void PumpLowLevelData ( IGameMesh &mesh, int uvChannel ) noexcept;
        [[nodiscard]] static IGameMesh &GetMesh ( IGameObject &object, bool exportInCurrentPose ) noexcept;
};

} // namespace avp


#endif // AVP_MESH_EXPORTER_HPP
