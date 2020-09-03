#include <pbr/opaque_call.h>


namespace pbr {

OpaqueCall::OpaqueCall ( MeshRef &mesh, const GXMat4 &local )
{
    Append ( mesh, local );
}

void OpaqueCall::Append ( MeshRef &mesh, const GXMat4 &local )
{
    if ( mesh->IsUnique () )
    {
        AddUnique ( mesh, local );
        return;
    }

    AddBatch ( mesh, local );
}

void OpaqueCall::AddBatch ( MeshRef &mesh, const GXMat4 &local )
{
    const std::string& name = mesh->GetName ();
    auto findResult = _batch.find ( name );

    if ( findResult == _batch.cend () )
    {
        _batch.insert ( std::make_pair ( std::string_view ( name ), MeshGroup ( mesh, local ) ) );
        return;
    }

    findResult->second._locals.push_back ( local );
}

void OpaqueCall::AddUnique ( MeshRef &mesh, const GXMat4 &local )
{
    _unique.emplace_back ( std::make_pair ( mesh, local ) );
}

} // namespace pbr
