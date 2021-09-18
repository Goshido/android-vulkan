#include <pbr/opaque_call.h>


namespace pbr {

OpaqueCall::OpaqueCall ( MeshRef &mesh,
    GXMat4 const &local,
    GXAABB const &worldBounds,
    android_vulkan::Half4 const &color0,
    android_vulkan::Half4 const &color1,
    android_vulkan::Half4 const &color2,
    android_vulkan::Half4 const &color3
) noexcept
{
    Append ( mesh, local, worldBounds, color0, color1, color2, color3 );
}

OpaqueCall::BatchList const& OpaqueCall::GetBatchList () const noexcept
{
    return _batch;
}

OpaqueCall::UniqueList const& OpaqueCall::GetUniqueList () const noexcept
{
    return _unique;
}

void OpaqueCall::Append ( MeshRef &mesh,
    GXMat4 const &local,
    GXAABB const &worldBounds,
    android_vulkan::Half4 const &color0,
    android_vulkan::Half4 const &color1,
    android_vulkan::Half4 const &color2,
    android_vulkan::Half4 const &color3
) noexcept
{
    if ( mesh->IsUnique () )
    {
        AddUnique ( mesh, local, worldBounds, color0, color1, color2, color3 );
        return;
    }

    AddBatch ( mesh, local, worldBounds, color0, color1, color2, color3 );
}

void OpaqueCall::AddBatch ( MeshRef &mesh,
    GXMat4 const &local,
    GXAABB const &worldBounds,
    android_vulkan::Half4 const &color0,
    android_vulkan::Half4 const &color1,
    android_vulkan::Half4 const &color2,
    android_vulkan::Half4 const &color3
) noexcept
{
    const std::string& name = mesh->GetName ();
    auto findResult = _batch.find ( name );

    if ( findResult == _batch.cend () )
    {
        _batch.insert (
            std::make_pair (
                std::string_view ( name ), MeshGroup ( mesh, local, worldBounds, color0, color1, color2, color3 )
            )
        );

        return;
    }

    findResult->second._opaqueData.emplace_back (
        OpaqueData {
            ._isVisible = true,
            ._local = local,
            ._worldBounds = worldBounds,
            ._color0 = color0,
            ._color1 = color1,
            ._color2 = color2,
            ._color3 = color3
        }
    );
}

void OpaqueCall::AddUnique ( MeshRef &mesh,
    GXMat4 const &local,
    GXAABB const &worldBounds,
    android_vulkan::Half4 const &color0,
    android_vulkan::Half4 const &color1,
    android_vulkan::Half4 const &color2,
    android_vulkan::Half4 const &color3
) noexcept
{
    _unique.emplace_back (
        std::make_pair ( mesh,
            OpaqueData {
                ._isVisible = true,
                ._local = local,
                ._worldBounds = worldBounds,
                ._color0 = color0,
                ._color1 = color1,
                ._color2 = color2,
                ._color3 = color3
            }
        )
    );
}

} // namespace pbr
