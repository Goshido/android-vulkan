#ifndef PBR_MARIO_PIPE_X2_H
#define PBR_MARIO_PIPE_X2_H


#include "pipe_base.h"


namespace pbr::mario {

class PipeX2 final : public PipeBase
{
    public:
        PipeX2 () = delete;

        PipeX2 ( PipeX2 const & ) = delete;
        PipeX2& operator = ( PipeX2 const & ) = delete;

        PipeX2 ( PipeX2 && ) = delete;
        PipeX2& operator = ( PipeX2 && ) = delete;

        ~PipeX2 () = delete;

        // Note "x", "y" and "z" coordinates must be in renderer units.
        static void Spawn ( android_vulkan::Renderer &renderer,
            VkCommandBuffer const*& commandBuffers,
            Scene &scene,
            float x,
            float y,
            float z
        ) noexcept;
};

} // namespace pbr::mario


#endif // PBR_MARIO_PIPE_X2_H
