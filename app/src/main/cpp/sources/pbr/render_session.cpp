#include <pbr/render_session.h>

GX_DISABLE_COMMON_WARNINGS

#include <cassert>

GX_RESTORE_WARNING_STATE


namespace pbr {

RenderSession::RenderSession ():
    _meshCount ( 0U ),
    _viewProjection {}
{
    // NOTHING
}

void RenderSession::AddMesh ( MeshRef &/*mesh*/,
    MaterialRef &/*material*/,
    const GXMat4 &/*local*/
)
{
    ++_meshCount;

    // TODO
    assert ( !"RenderSession::AddMesh - Implement me!" );
}

void RenderSession::Begin ( const GXMat4 &view, const GXMat4 &projection )
{
    _meshCount = 0U;
    _viewProjection.Multiply ( view, projection );

    // TODO
    assert ( !"RenderSession::Begin - Implement me!" );
}

void RenderSession::End ( ePresentTarget /*target*/, android_vulkan::Renderer &/*renderer*/ )
{
    assert ( !"RenderSession::End - Implement me!" );
}

} // namespace pbr
