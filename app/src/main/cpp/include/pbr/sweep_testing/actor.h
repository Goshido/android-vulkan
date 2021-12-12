#ifndef PBR_SWEEP_TESTING_ACTOR_H
#define PBR_SWEEP_TESTING_ACTOR_H


#include <pbr/render_session.h>
#include <physics.h>


namespace pbr::sweep_testing {

class Actor final
{
    private:
        [[maybe_unused]] android_vulkan::RigidBodyRef       _body {};
        ComponentRef                                        _mesh {};

    public:
        Actor () = default;

        Actor ( Actor const & ) = delete;
        Actor& operator = ( Actor const & ) = delete;

        Actor ( Actor && ) = delete;
        Actor& operator = ( Actor && ) = delete;

        ~Actor () = default;

        [[maybe_unused]] void EnableOverlay ( Texture2DRef const &overlay ) noexcept;
        [[maybe_unused]] void DisableOverlay () noexcept;

        void FreeTransferResources ( VkDevice device ) noexcept;
        void Destroy () noexcept;

        [[nodiscard]] bool Init ( android_vulkan::Renderer &renderer,
            android_vulkan::Physics &physics,
            size_t &commandBufferConsumed,
            VkCommandBuffer const* commandBuffers,
            GXVec3 const &location,
            GXVec3 const &size
        ) noexcept;

        void Submit ( RenderSession &renderSession ) noexcept;
        void Update ( float deltaTime ) noexcept;
};

} // namespace pbr::sweep_testing


#endif // PBR_SWEEP_TESTING_ACTOR_H
