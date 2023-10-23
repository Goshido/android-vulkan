#ifndef AVP_MESH_EXPORTER_HPP
#define AVP_MESH_EXPORTER_HPP


#include "exporter.hpp"

GX_DISABLE_COMMON_WARNINGS

#include <IGame/IGameObject.h>

GX_RESTORE_WARNING_STATE


namespace avp {

class MeshExporter final : public Exporter
{
    public:
        MeshExporter () = delete;

        MeshExporter ( MeshExporter const & ) = delete;
        MeshExporter &operator = ( MeshExporter const & ) = delete;

        MeshExporter ( MeshExporter && ) = delete;
        MeshExporter &operator = ( MeshExporter && ) = delete;

        ~MeshExporter () = delete;

        static void Run ( HWND parent, MSTR const &path, bool exportInCurrentPose ) noexcept;

    private:
        [[nodiscard]] static IGameMesh &GetMesh ( IGameObject &object, bool exportInCurrentPose ) noexcept;
};

} // namespace avp


#endif // AVP_MESH_EXPORTER_HPP
