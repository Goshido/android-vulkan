#ifndef RENDER_SESSION_H
#define RENDER_SESSION_H


#include <GXCommon/GXMath.h>
#include "types.h"


namespace pbr {

enum class ePresentTarget : uint8_t
{
    Albedo,
    Emission,
    Normal,
    Param
};

// Single threaded class
class RenderSession final
{
    public:
        size_t      _meshCount;
        GXMat4      _viewProjection;

    public:
        RenderSession ();
        ~RenderSession () = default;

        RenderSession ( const RenderSession &other ) = delete;
        RenderSession& operator = ( const RenderSession &other ) = delete;

        [[maybe_unused]] void AddMesh ( MeshRef &mesh, MaterialRef &material, const GXMat4 &local );

        [[maybe_unused]] void Begin ( const GXMat4 &view, const GXMat4 &projection );
        [[maybe_unused]] void End ( ePresentTarget target, android_vulkan::Renderer &renderer );
};

} // namespace pbr


#endif // RENDER_SESSION_H
