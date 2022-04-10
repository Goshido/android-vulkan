#ifndef PBR_MARIO_PIPE_BASE_H
#define PBR_MARIO_PIPE_BASE_H


#include <pbr/scene.h>
#include <physics.h>


namespace pbr::mario {

class PipeBase
{
    public:
        PipeBase ( PipeBase const & ) = delete;
        PipeBase& operator = ( PipeBase const & ) = delete;

        PipeBase ( PipeBase && ) = delete;
        PipeBase& operator = ( PipeBase && ) = delete;

        virtual ~PipeBase () = default;

        // Note "x", "y" and "z" coordinates must be in renderer units.
        void Init ( android_vulkan::Renderer &renderer,
            size_t &commandBufferConsumed,
            VkCommandBuffer const* commandBuffers,
            Scene &scene,
            android_vulkan::Physics &physics,
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
        PipeBase () = default;

        // Note the offset must be in physics units.
        [[nodiscard]] virtual GXVec3 const& GetColliderOffset () const noexcept = 0;

        // Note the size must be in physics units.
        [[nodiscard]] virtual GXVec3 const& GetColliderSize () const noexcept = 0;

        [[nodiscard]] virtual char const* GetMesh () const noexcept = 0;
};

} // namespace pbr::mario


#endif // PBR_MARIO_PIPE_BASE_H
