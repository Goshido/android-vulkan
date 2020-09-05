#ifndef OPAQUE_CALL_H
#define OPAQUE_CALL_H


#include "mesh_group.h"


namespace pbr {

class OpaqueCall final
{
    public:
        using UniqueList = std::vector<std::pair<MeshRef, GXMat4>>;
        using BatchList = std::map<std::string_view, MeshGroup>;

    private:
        BatchList       _batch;
        UniqueList      _unique;

    public:
        OpaqueCall () = delete;
        OpaqueCall ( const OpaqueCall &other ) = delete;
        OpaqueCall& operator = ( const OpaqueCall &other ) = delete;
        OpaqueCall ( OpaqueCall &&other ) = default;
        OpaqueCall& operator = ( OpaqueCall &&other ) = default;
        explicit OpaqueCall ( MeshRef &mesh, const GXMat4 &local );
        ~OpaqueCall () = default;

        void Append ( MeshRef &mesh, const GXMat4 &local );

        [[maybe_unused]] [[nodiscard]] const BatchList& GetBatchList () const;
        [[nodiscard]] const UniqueList& GetUniqueList () const;

    private:
        void AddBatch ( MeshRef &mesh, const GXMat4 &local );
        void AddUnique ( MeshRef &mesh, const GXMat4 &local );
};

} // namespace pbr


#endif // OPAQUE_CALL_H
