#ifndef OPAQUE_CALL_H
#define OPAQUE_CALL_H


#include "mesh_group.h"


namespace pbr {

class OpaqueCall final
{
    private:
        std::map<std::string_view, MeshGroup>       _batch;
        std::vector<std::pair<MeshRef, GXMat4>>     _unique;

    public:
        explicit OpaqueCall ( MeshRef &mesh, const GXMat4 &local );
        OpaqueCall ( OpaqueCall &&other ) = default;
        ~OpaqueCall () = default;

        OpaqueCall () = delete;
        OpaqueCall ( const OpaqueCall &other ) = delete;
        OpaqueCall& operator = ( const OpaqueCall &other ) = delete;

        OpaqueCall& operator = ( OpaqueCall &&other ) = default;

        void Append ( MeshRef &mesh, const GXMat4 &local );

    private:
        void AddBatch ( MeshRef &mesh, const GXMat4 &local );
        void AddUnique ( MeshRef &mesh, const GXMat4 &local );
};

} // namespace pbr


#endif // OPAQUE_CALL_H
