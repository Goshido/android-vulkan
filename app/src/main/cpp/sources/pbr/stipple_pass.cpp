#include <pbr/stipple_pass.h>


namespace pbr {

void StipplePass::Execute ( RenderSessionStats &/*renderSessionStats*/ ) noexcept
{
    // TODO
}

void StipplePass::Reset () noexcept
{
    _sceneData.clear ();
}

void StipplePass::Submit ( MeshRef &mesh,
    MaterialRef const &material,
    GXMat4 const &local,
    GXAABB const &worldBounds,
    GXColorRGB const &color0,
    GXColorRGB const &color1,
    GXColorRGB const &color2,
    GXColorRGB const &emission
) noexcept
{
    // NOLINTNEXTLINE - downcast
    auto& m = static_cast<GeometryPassMaterial&> ( *material );
    auto findResult = _sceneData.find ( m );

    if ( findResult != _sceneData.cend () )
    {
        findResult->second.Append ( mesh, local, worldBounds, color0, color1, color2, emission );
        return;
    }

    _sceneData.emplace (
        std::make_pair ( m, GeometryCall ( mesh, local, worldBounds, color0, color1, color2, emission ) )
    );
}

} // namespace pbr
