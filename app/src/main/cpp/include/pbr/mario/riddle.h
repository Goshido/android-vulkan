#ifndef PBR_MARIO_RIDDLE_H
#define PBR_MARIO_RIDDLE_H


#include <pbr/scene.h>


namespace pbr::mario {

class Riddle final
{
    public:
        Riddle () = delete;

        Riddle ( Riddle const & ) = delete;
        Riddle& operator = ( Riddle const & ) = delete;

        Riddle ( Riddle && ) = delete;
        Riddle& operator = ( Riddle && ) = delete;

        ~Riddle () = delete;

        // Note "x", "y" and "z" coordinates must be in renderer units.
        static void Spawn ( android_vulkan::Renderer &renderer,
            VkCommandBuffer const*& commandBuffers,
            Scene &scene,
            float x,
            float y,
            float z
        ) noexcept;

        [[nodiscard]] constexpr static size_t CommandBufferCountRequirement () noexcept
        {
            // Mesh geometry and one texture 2D.
            return 2U;
        }
};

} // namespace pbr::mario


#endif // PBR_MARIO_RIDDLE_H
