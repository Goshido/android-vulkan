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

        // Note maxBatch will be updated only if it's less than current max unique elements of this drawcall.
        // Note maxUnique will be updated only if it's less than current max unique elements of this drawcall.
        explicit OpaqueCall ( size_t &maxBatch, size_t &maxUnique, MeshRef &mesh, const GXMat4 &local );

        ~OpaqueCall () = default;

        // The method returns the maximum batch item count.
        // Note maxBatch will be updated only if it's less than current max unique elements of this drawcall.
        // Note maxUnique will be updated only if it's less than current max unique elements of this drawcall.
        void Append ( size_t &maxBatch, size_t &maxUnique, MeshRef &mesh, const GXMat4 &local );

        [[nodiscard]] const BatchList& GetBatchList () const;
        [[nodiscard]] const UniqueList& GetUniqueList () const;

    private:
        void AddBatch ( size_t &maxBatch, MeshRef &mesh, const GXMat4 &local );
        void AddUnique ( size_t &maxUnique, MeshRef &mesh, const GXMat4 &local );
};

} // namespace pbr


#endif // OPAQUE_CALL_H
