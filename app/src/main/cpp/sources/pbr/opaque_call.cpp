#include <pbr/opaque_call.h>


namespace pbr {

OpaqueCall::OpaqueCall ( size_t &maxBatch,
    size_t &maxUnique,
    MeshRef &mesh,
    GXMat4 const &local,
    android_vulkan::Half4 const &color0,
    android_vulkan::Half4 const &color1,
    android_vulkan::Half4 const &color2,
    android_vulkan::Half4 const &color3
)
{
    Append ( maxBatch, maxUnique, mesh, local, color0, color1, color2, color3 );
}

OpaqueCall::BatchList const& OpaqueCall::GetBatchList () const
{
    return _batch;
}

OpaqueCall::UniqueList const& OpaqueCall::GetUniqueList () const
{
    return _unique;
}

void OpaqueCall::Append ( size_t &maxBatch,
    size_t &maxUnique,
    MeshRef &mesh,
    GXMat4 const &local,
    android_vulkan::Half4 const &color0,
    android_vulkan::Half4 const &color1,
    android_vulkan::Half4 const &color2,
    android_vulkan::Half4 const &color3
)
{
    if ( mesh->IsUnique () )
    {
        AddUnique ( maxUnique, mesh, local, color0, color1, color2, color3 );
        return;
    }

    AddBatch ( maxBatch, mesh, local, color0, color1, color2, color3 );
}

void OpaqueCall::AddBatch ( size_t &maxBatch,
    MeshRef &mesh,
    GXMat4 const &local,
    android_vulkan::Half4 const &color0,
    android_vulkan::Half4 const &color1,
    android_vulkan::Half4 const &color2,
    android_vulkan::Half4 const &color3
)
{
    const std::string& name = mesh->GetName ();
    auto findResult = _batch.find ( name );

    if ( findResult == _batch.cend () )
    {
        _batch.insert (
            std::make_pair ( std::string_view ( name ), MeshGroup ( mesh, local, color0, color1, color2, color3 ) )
        );
    }
    else
    {
        findResult->second._opaqueData.emplace_back (
            OpaqueData {
                ._isVisible = true,
                ._local = local,
                ._color0 = color0,
                ._color1 = color1,
                ._color2 = color2,
                ._color3 = color3
            }
        );
    }

    size_t const count = _batch.size ();

    if ( maxBatch >= count )
        return;

    maxBatch = count;
}

void OpaqueCall::AddUnique ( size_t &maxUnique,
    MeshRef &mesh,
    GXMat4 const &local,
    android_vulkan::Half4 const &color0,
    android_vulkan::Half4 const &color1,
    android_vulkan::Half4 const &color2,
    android_vulkan::Half4 const &color3
)
{
    _unique.emplace_back (
        std::make_pair ( mesh,
            OpaqueData {
                ._isVisible = true,
                ._local = local,
                ._color0 = color0,
                ._color1 = color1,
                ._color2 = color2,
                ._color3 = color3
            }
        )
    );

    size_t const count = _unique.size ();

    if ( maxUnique >= count )
        return;

    maxUnique = count;
}

} // namespace pbr
