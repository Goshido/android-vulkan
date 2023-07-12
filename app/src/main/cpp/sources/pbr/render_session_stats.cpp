#include <pbr/render_session_stats.hpp>
#include <logger.hpp>

GX_DISABLE_COMMON_WARNINGS

#include <cstdio>
#include <memory>

GX_RESTORE_WARNING_STATE


namespace pbr {

constexpr static double TIMEOUT = 3.0;

RenderSessionStats::RenderSessionStats () noexcept:
    _timeout ( TIMEOUT )
{
    // NOTHING
}

void RenderSessionStats::PrintStats ( double deltaTime ) noexcept
{
    ++_frameCount;
    _timeout -= deltaTime;

    if ( _timeout > 0.0 )
        return;

    constexpr char const format[] =
R"__(pbr::RenderSessionStats::PrintStats:
                        Submitted     Rendered    Culled
              Vertices  %9zu    %9zu    %s
         Opaque meshes  %9zu    %9zu    %s
        Stipple meshes  %9zu    %9zu    %s
          Point lights  %9zu    %9zu    %s
     Local reflections  %9zu    %9zu    %s
    Global reflections  %9zu    %9zu    %s
           UI vertices  %9zu    %9zu        0%%)__";

    auto avgCounter = [ & ] ( size_t counter ) noexcept -> size_t {
        return counter / _frameCount;
    };

    constexpr size_t capacity = 8U;
    char opaqueCull[ capacity ];
    char stippleCull[ capacity ];
    char vertexCull[ capacity ];
    char pointLightCull[ capacity ];
    char reflectionLocalCull[ capacity ];
    char reflectionGlobalCull[ capacity ];

    auto avgPercent = [] ( char* dst, size_t renderCount, size_t submitCount ) noexcept -> char const* {
        if ( !submitCount )
        {
            constexpr char const notApplicable[] = "   N/A";
            std::memcpy ( dst, notApplicable, std::size ( notApplicable ) );
            return dst;
        }

        std::snprintf ( dst, capacity, "%5zu%%", 100U - ( 100U * renderCount / submitCount ) );
        return dst;
    };

    size_t const g = avgCounter ( _renderReflectionsGlobal );
    size_t const ui = avgCounter ( _submitUIVertices );

    android_vulkan::LogInfo ( format,
        avgCounter ( _submitVertices ),
        avgCounter ( _renderVertices ),
        avgPercent ( vertexCull, _renderVertices, _submitVertices ),
        avgCounter ( _submitOpaqueMeshes ),
        avgCounter ( _renderOpaqueMeshes ),
        avgPercent ( opaqueCull, _renderOpaqueMeshes, _submitOpaqueMeshes ),
        avgCounter ( _submitStippleMeshes ),
        avgCounter ( _renderStippleMeshes ),
        avgPercent ( stippleCull, _renderStippleMeshes, _submitStippleMeshes ),
        avgCounter ( _submitPointLights ),
        avgCounter ( _renderPointLights ),
        avgPercent ( pointLightCull, _renderPointLights, _submitPointLights ),
        avgCounter ( _submitReflectionsLocal ),
        avgCounter ( _renderReflectionsLocal ),
        avgPercent ( reflectionLocalCull, _renderReflectionsLocal, _submitReflectionsLocal ),
        g,
        g,
        avgPercent ( reflectionGlobalCull, g, g ),
        ui,
        ui
    );

    Reset ();
}

void RenderSessionStats::RenderOpaque ( uint32_t vertexCount, uint32_t instanceCount ) noexcept
{
    _renderOpaqueMeshes += static_cast<size_t> ( instanceCount );
    _renderVertices += static_cast<size_t> ( vertexCount ) * static_cast<size_t> ( instanceCount );
}

void RenderSessionStats::SubmitOpaque ( uint32_t vertexCount ) noexcept
{
    ++_submitOpaqueMeshes;
    _submitVertices += static_cast<size_t> ( vertexCount );
}

void RenderSessionStats::RenderStipple ( uint32_t vertexCount, uint32_t instanceCount ) noexcept
{
    ++_renderStippleMeshes;
    _renderVertices += static_cast<size_t> ( vertexCount ) * static_cast<size_t> ( instanceCount );
}

void RenderSessionStats::SubmitStipple ( uint32_t vertexCount ) noexcept
{
    ++_submitStippleMeshes;
    _submitVertices += static_cast<size_t> ( vertexCount );
}

void RenderSessionStats::RenderPointLights ( size_t count ) noexcept
{
    _renderPointLights += count;
}

void RenderSessionStats::SubmitPointLight () noexcept
{
    ++_submitPointLights;
}

void RenderSessionStats::RenderReflectionGlobal () noexcept
{
    ++_renderReflectionsGlobal;
}

void RenderSessionStats::RenderReflectionLocal ( size_t count ) noexcept
{
    _renderReflectionsLocal += count;
}

void RenderSessionStats::SubmitReflectionLocal () noexcept
{
    ++_submitReflectionsLocal;
}

void RenderSessionStats::SubmitUIVertices ( size_t count ) noexcept
{
    _submitUIVertices += count;
}

void RenderSessionStats::Reset () noexcept
{
    _frameCount = 0U;
    _renderOpaqueMeshes = 0U;
    _renderStippleMeshes = 0U;
    _renderPointLights = 0U;
    _renderReflectionsGlobal = 0U;
    _renderReflectionsLocal = 0U;
    _renderVertices = 0U;
    _submitOpaqueMeshes = 0U;
    _submitStippleMeshes = 0U;
    _submitPointLights = 0U;
    _submitReflectionsLocal = 0U;
    _submitUIVertices = 0U;
    _submitVertices = 0U;
    _timeout = TIMEOUT;
}

} // namespace pbr
