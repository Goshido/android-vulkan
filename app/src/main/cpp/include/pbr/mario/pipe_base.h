#ifndef PBR_MARIO_PIPE_BASE_H
#define PBR_MARIO_PIPE_BASE_H


#include <pbr/types.h>
#include <rigid_body.h>


namespace pbr::mario {

class PipeBase
{
    private:
        android_vulkan::RigidBodyRef    _collider;
        ComponentRef                    _staticMesh;

    public:
        PipeBase ( PipeBase const & ) = delete;
        PipeBase& operator = ( PipeBase const & ) = delete;

        PipeBase ( PipeBase && ) = delete;
        PipeBase& operator = ( PipeBase && ) = delete;

        virtual ~PipeBase () = default;

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

    protected:
        PipeBase () noexcept;

        // Note the offset must be in physics units.
        [[nodiscard]] virtual GXVec3 const& GetColliderOffset () const noexcept = 0;

        // Note the size must be in physics units.
        [[nodiscard]] virtual GXVec3 const& GetColliderSize () const noexcept = 0;

        [[nodiscard]] virtual char const* GetMesh () const noexcept = 0;
};

} // namespace pbr::mario


#endif // PBR_MARIO_PIPE_BASE_H
