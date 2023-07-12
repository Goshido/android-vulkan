#ifndef PBR_BOX_STACK_H
#define PBR_BOX_STACK_H


#include <pbr/camera.h>
#include <pbr/point_light_component.h>
#include <pbr/render_session.h>
#include <game.h>
#include <physics.h>

GX_DISABLE_COMMON_WARNINGS

#include <list>

GX_RESTORE_WARNING_STATE


namespace pbr::box_stack {

class BoxStack final : public android_vulkan::Game
{
    private:
        Camera                                          _camera {};
        VkCommandPool                                   _commandPool = VK_NULL_HANDLE;
        std::vector<VkCommandBuffer>                    _commandBuffers {};

        ComponentRef                                    _cameraLight {};
        std::vector<ComponentRef>                       _cubes {};
        std::vector<android_vulkan::RigidBodyRef>       _cubeBodies {};

        std::array<GXColorRGB, 7U>                      _colors {};

        GXColorRGB                                      _defaultColor {};
        android_vulkan::Physics                         _physics {};
        MeshRef                                         _sphereMesh {};
        MaterialRef                                     _sphereMaterial {};

        RenderSession                                   _renderSession {};
        std::list<ComponentRef>                         _components {};

    public:
        BoxStack () = default;

        BoxStack ( BoxStack const & ) = delete;
        BoxStack &operator = ( BoxStack const & ) = delete;

        BoxStack ( BoxStack && ) = delete;
        BoxStack &operator = ( BoxStack && ) = delete;

        ~BoxStack () override = default;

    private:
        [[nodiscard]] bool IsReady () noexcept override;
        [[nodiscard]] bool OnFrame ( android_vulkan::Renderer &renderer, double deltaTime ) noexcept override;

        [[nodiscard]] bool OnInitDevice ( android_vulkan::Renderer &renderer ) noexcept override;
        void OnDestroyDevice ( android_vulkan::Renderer &renderer ) noexcept override;

        [[nodiscard]] bool OnSwapchainCreated ( android_vulkan::Renderer &renderer ) noexcept override;
        void OnSwapchainDestroyed ( android_vulkan::Renderer &renderer ) noexcept override;

        [[nodiscard]] bool AppendCuboid ( android_vulkan::Renderer &renderer,
            VkCommandBuffer const* commandBuffers,
            size_t &commandBufferConsumed,
            std::string &&tag,
            ComponentRef &visual,
            char const* material,
            GXColorRGB const &color,
            android_vulkan::RigidBodyRef &physical,
            float x,
            float y,
            float z,
            float w,
            float h,
            float d
        ) noexcept;

        void DestroyCommandPool ( VkDevice device ) noexcept;

        [[nodiscard]] bool CreateSceneManual ( android_vulkan::Renderer &renderer ) noexcept;

        void DestroyPhysics () noexcept;
        void UpdatePhysicsActors () noexcept;

        void InitColors () noexcept;

        static void OnLeftBumper ( void* context ) noexcept;
        static void OnRightBumper ( void* context ) noexcept;

        static void UpdateCuboid ( ComponentRef &cuboid, android_vulkan::RigidBodyRef &body ) noexcept;
};

} // namespace pbr::box_stack


#endif // PBR_BOX_STACK_H
