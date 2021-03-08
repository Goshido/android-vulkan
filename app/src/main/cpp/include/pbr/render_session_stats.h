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
        size_t                  _frameCount;
        std::atomic_size_t      _renderMeshes;
        std::atomic_size_t      _renderPointLights;
        std::atomic_size_t      _renderReflectionsGlobal;
        std::atomic_size_t      _renderVertices;
        std::atomic_size_t      _submitMeshes;
        std::atomic_size_t      _submitPointLights;
        std::atomic_size_t      _submitVertices;
        double                  _timeout;

    public:
        RenderSessionStats () noexcept;

        RenderSessionStats ( RenderSessionStats const & ) = delete;
        RenderSessionStats& operator = ( RenderSessionStats const & ) = delete;

        RenderSessionStats ( RenderSessionStats && ) = delete;
        RenderSessionStats& operator = ( RenderSessionStats && ) = delete;

        ~RenderSessionStats () = default;

        void PrintStats ( double deltaTime );

        void RenderOpaque ( uint32_t vertexCount, uint32_t instanceCount );
        void SubmitOpaque ( uint32_t vertexCount );

        void RenderPointLights ( size_t count );
        void SubmitPointLight ();

        void RenderReflectionGlobal ();

    private:
        void Reset ();
};

} // namespace pbr


#endif // PBR_RENDER_SESSION_STATS_H
