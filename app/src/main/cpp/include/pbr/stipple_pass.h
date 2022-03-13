#ifndef PBR_STIPPLE_PASS_H
#define PBR_STIPPLE_PASS_H


#include "render_session_stats.h"
#include "scene_data.h"


namespace pbr {

class StipplePass final
{
    private:
        SceneData       _sceneData {};

    public:
        StipplePass () = default;

        StipplePass ( StipplePass const & ) = delete;
        StipplePass& operator = ( StipplePass const & ) = delete;

        StipplePass ( StipplePass && ) = delete;
        StipplePass& operator = ( StipplePass && ) = delete;

        ~StipplePass () = default;

        void Execute ( RenderSessionStats &renderSessionStats ) noexcept;
        void Reset () noexcept;

        void Submit ( MeshRef &mesh,
            MaterialRef const &material,
            GXMat4 const &local,
            GXAABB const &worldBounds,
            GXColorRGB const &color0,
            GXColorRGB const &color1,
            GXColorRGB const &color2,
            GXColorRGB const &emission
        ) noexcept;
};

} // namespace pbr


#endif // PBR_STIPPLE_PASS_H
