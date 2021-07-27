#ifndef PBR_MARIO_RIDDLE_H
#define PBR_MARIO_RIDDLE_H


#include <pbr/types.h>
#include <rigid_body.h>


namespace pbr::mario {

class Riddle final
{
    private:
        android_vulkan::RigidBodyRef    _collider;
        ComponentRef                    _staticMesh;

    public:
        Riddle () noexcept;

        Riddle ( Riddle const & ) = delete;
        Riddle& operator = ( Riddle const & ) = delete;

        Riddle ( Riddle && ) = delete;
        Riddle& operator = ( Riddle && ) = delete;

        ~Riddle () = default;

        [[nodiscard]] android_vulkan::RigidBodyRef& GetCollider () noexcept;
        [[nodiscard]] ComponentRef& GetComponent () noexcept;

        // Note "x", "y" and "z" coordinates must be in renderer units.
        void Init ( android_vulkan::Renderer &renderer,
            size_t &commandBufferConsumed,
            VkCommandBuffer const* commandBuffers,
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
