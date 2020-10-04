#include <pbr/opaque_call.h>


namespace pbr {

OpaqueCall::OpaqueCall ( size_t &maxBatch, size_t &maxUnique, MeshRef &mesh, const GXMat4 &local )
{
    Append ( maxBatch, maxUnique, mesh, local );
}

const OpaqueCall::BatchList& OpaqueCall::GetBatchList () const
{
    return _batch;
}

const OpaqueCall::UniqueList& OpaqueCall::GetUniqueList () const
{
    return _unique;
}

void OpaqueCall::Append ( size_t &maxBatch, size_t &maxUnique, MeshRef &mesh, const GXMat4 &local )
{
    if ( mesh->IsUnique () )
    {
        AddUnique ( maxUnique, mesh, local );
        return;
    }

    AddBatch ( maxBatch, mesh, local );
}

void OpaqueCall::AddBatch ( size_t &maxBatch, MeshRef &mesh, const GXMat4 &local )
{
    const std::string& name = mesh->GetName ();
    auto findResult = _batch.find ( name );

    if ( findResult == _batch.cend () )
        _batch.insert ( std::make_pair ( std::string_view ( name ), MeshGroup ( mesh, local ) ) );
    else
        findResult->second._locals.push_back ( local );

    size_t const count = _batch.size ();

    if ( maxBatch >= count )
        return;

    maxBatch = count;
}

void OpaqueCall::AddUnique ( size_t &maxUnique, MeshRef &mesh, const GXMat4 &local )
{
    _unique.emplace_back ( std::make_pair ( mesh, local ) );
    size_t const count = _unique.size ();

    if ( maxUnique >= count )
        return;

    maxUnique = count;
}

} // namespace pbr
