#include <pbr/render_session_stats.h>
#include <logger.h>


namespace pbr {

constexpr static double const TIMEOUT = 3.0;

RenderSessionStats::RenderSessionStats () noexcept:
    _frameCount ( 0U ),
    _renderMeshes ( 0U ),
    _renderPointLights ( 0U ),
    _renderReflectionsGlobal ( 0U ),
    _renderVertices ( 0U ),
    _submitMeshes ( 0U ),
    _submitPointLights ( 0U ),
    _submitVertices ( 0U ),
    _timeout ( TIMEOUT )
{
    // NOTHING
}

void RenderSessionStats::PrintStats ( double deltaTime )
{
    ++_frameCount;
    _timeout -= deltaTime;

    if ( _timeout > 0.0 )
        return;

    constexpr char const format[] =
R"__(>>> RenderSessionStats::PrintStats:
    Submitted meshes: %zu
    Submitted vertices: %zu
    Submitted point lights: %zu
    Rendered global reflections: %zu
    Rendered meshes: %zu
    Rendered vertices: %zu
    Rendered point lights: %zu
    Culling mesh percent: %zu%%
    Culling vertex percent: %zu%%
    Culling point light percent: %zu%%
)__";

    auto avgCounter = [ & ] ( size_t counter ) -> size_t {
        return counter / _frameCount;
    };

    auto avgPercent = [] ( size_t renderCount, size_t submitCount ) -> size_t {
        return 100U - ( 100U * renderCount / submitCount );
    };

    android_vulkan::LogInfo ( format,
        avgCounter ( _submitMeshes ),
        avgCounter ( _submitMeshes ),
        avgCounter ( _submitVertices ),
        avgCounter ( _submitPointLights ),
        avgCounter ( _renderReflectionsGlobal ),
        avgCounter ( _renderMeshes ),
        avgCounter ( _renderVertices ),
        avgCounter ( _renderPointLights ),
        avgPercent ( _renderMeshes, _submitMeshes ),
        avgPercent ( _renderVertices, _submitVertices ),
        avgPercent ( _renderPointLights, _submitPointLights )
    );

    Reset ();
}

void RenderSessionStats::RenderOpaque ( uint32_t vertexCount, uint32_t instanceCount )
{
    _renderMeshes += static_cast<size_t> ( instanceCount );
    _renderVertices += static_cast<size_t> ( vertexCount ) * static_cast<size_t> ( instanceCount );
}

void RenderSessionStats::SubmitOpaque ( uint32_t vertexCount )
{
    ++_submitMeshes;
    _submitVertices += static_cast<size_t> ( vertexCount );
}

void RenderSessionStats::RenderPointLights ( size_t count )
{
    _renderPointLights += count;
}

void RenderSessionStats::SubmitPointLight ()
{
    ++_submitPointLights;
}

void RenderSessionStats::RenderReflectionGlobal ()
{
    ++_renderReflectionsGlobal;
}

void RenderSessionStats::Reset ()
{
    _frameCount = 0U;
    _renderReflectionsGlobal = 0U;
    _renderMeshes = 0U;
    _renderPointLights = 0U;
    _renderVertices = 0U;
    _submitMeshes = 0U;
    _submitPointLights = 0U;
    _submitVertices = 0U;
    _timeout = TIMEOUT;
}

} // namespace pbr
