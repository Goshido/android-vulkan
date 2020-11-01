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
        using Counter = std::atomic<size_t>;

        size_t      _frameCount;
        Counter     _renderMeshes;
        Counter     _renderVertices;
        Counter     _submitMeshes;
        Counter     _submitVertices;
        double      _timeout;

    public:
        RenderSessionStats ();

        RenderSessionStats ( RenderSessionStats const & ) = delete;
        RenderSessionStats& operator = ( RenderSessionStats const & ) = delete;

        RenderSessionStats ( RenderSessionStats && ) = delete;
        RenderSessionStats& operator = ( RenderSessionStats && ) = delete;

        ~RenderSessionStats () = default;

        void PrintStats ( double deltaTime );

        void RenderOpaque ( uint32_t vertexCount, uint32_t instanceCount );
        void SubmitOpaque ( uint32_t vertexCount );

        void Reset ();
};

} // namespace pbr


#endif // PBR_RENDER_SESSION_STATS_H
