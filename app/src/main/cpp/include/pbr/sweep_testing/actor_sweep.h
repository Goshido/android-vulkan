#ifndef PBR_SWEEP_TESTING_ACTOR_SWEEP_H
#define PBR_SWEEP_TESTING_ACTOR_SWEEP_H


#include <pbr/render_session.h>
#include <shape.h>


namespace pbr::sweep_testing {

class ActorSweep final
{
    private:
        ComponentRef                _mesh {};
        android_vulkan::ShapeRef    _shape {};
        float                       _visibilityTimeout = 0.0F;
        bool                        _visible = true;

    public:
        ActorSweep () = default;

        ActorSweep ( ActorSweep const & ) = delete;
        ActorSweep& operator = ( ActorSweep const & ) = delete;

        ActorSweep ( ActorSweep && ) = delete;
        ActorSweep& operator = ( ActorSweep && ) = delete;

        ~ActorSweep () = default;

        void FreeTransferResources ( VkDevice device ) noexcept;

        void Destroy () noexcept;
        [[nodiscard, maybe_unused]] android_vulkan::ShapeRef const& GetShape () noexcept;

        [[nodiscard]] bool Init ( android_vulkan::Renderer &renderer,
            size_t &commandBufferConsumed,
            VkCommandBuffer const* commandBuffers,
            GXVec3 const &location,
            GXVec3 const &size
        ) noexcept;

        void SetOverlay ( Texture2DRef const &overlay ) noexcept;
        void Submit ( RenderSession &renderSession ) noexcept;
        void Update ( float deltaTime ) noexcept;
};

} // namespace pbr::sweep_testing


#endif // PBR_SWEEP_TESTING_ACTOR_SWEEP_H
