#ifndef PBR_RENDER_SESSION_STATS_H
#define PBR_RENDER_SESSION_STATS_H


#include <GXCommon/GXWarning.h>

GX_DISABLE_COMMON_WARNINGS

#include <atomic>

GX_RESTORE_WARNING_STATE


namespace pbr {

// Thread safe class
class RenderSessionStats final
{
    private:
        size_t                  _frameCount = 0U;
        std::atomic_size_t      _renderOpaqueMeshes = 0U;
        std::atomic_size_t      _renderStippleMeshes = 0U;
        std::atomic_size_t      _renderPointLights = 0U;
        std::atomic_size_t      _renderReflectionsGlobal = 0U;
        std::atomic_size_t      _renderReflectionsLocal = 0U;
        std::atomic_size_t      _renderVertices = 0U;
        std::atomic_size_t      _submitOpaqueMeshes = 0U;
        std::atomic_size_t      _submitStippleMeshes = 0U;
        std::atomic_size_t      _submitPointLights = 0U;
        std::atomic_size_t      _submitReflectionsLocal = 0U;
        std::atomic_size_t      _submitUIVertices = 0U;
        std::atomic_size_t      _submitVertices = 0U;
        double                  _timeout;

    public:
        RenderSessionStats () noexcept;

        RenderSessionStats ( RenderSessionStats const & ) = delete;
        RenderSessionStats& operator = ( RenderSessionStats const & ) = delete;

        RenderSessionStats ( RenderSessionStats && ) = delete;
        RenderSessionStats& operator = ( RenderSessionStats && ) = delete;

        ~RenderSessionStats () = default;

        void PrintStats ( double deltaTime ) noexcept;

        void RenderOpaque ( uint32_t vertexCount, uint32_t instanceCount ) noexcept;
        void SubmitOpaque ( uint32_t vertexCount ) noexcept;

        void RenderStipple ( uint32_t vertexCount, uint32_t instanceCount ) noexcept;
        void SubmitStipple ( uint32_t vertexCount ) noexcept;

        void RenderPointLights ( size_t count ) noexcept;
        void SubmitPointLight () noexcept;

        void RenderReflectionGlobal () noexcept;

        void RenderReflectionLocal ( size_t count ) noexcept;
        void SubmitReflectionLocal () noexcept;

        void SubmitUIVertices ( size_t count ) noexcept;

    private:
        void Reset () noexcept;
};

} // namespace pbr


#endif // PBR_RENDER_SESSION_STATS_H
