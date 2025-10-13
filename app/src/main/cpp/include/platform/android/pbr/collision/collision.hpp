#ifndef PBR_COLLISION_HPP
#define PBR_COLLISION_HPP


#include <game.hpp>
#include "manipulator.hpp"
#include <physics.hpp>
#include <platform/android/pbr/camera.hpp>
#include <platform/android/pbr/point_light_component.hpp>
#include <platform/android/pbr/render_session.hpp>


namespace pbr::collision {

class Collision final : public android_vulkan::Game
{
    private:
        struct CubeInfo final
        {
            ComponentRef                    _component;
            android_vulkan::RigidBodyRef    _body;
        };

    private:
        Camera                              _camera {};
        VkCommandPool                       _commandPool = VK_NULL_HANDLE;
        std::vector<VkCommandBuffer>        _commandBuffers {};

        ComponentRef                        _cameraLight {};
        std::vector<CubeInfo>               _cubes {};

        android_vulkan::ContactDetector     _contactDetector {};
        android_vulkan::ContactManager      _contactManager {};

        MeshRef                             _contactMesh {};
        MaterialRef                         _contactMaterial {};
        Manipulator                         _manipulator {};
        RenderSession                       _renderSession {};

        GXColorUNORM const                  _aColor { 115U, 185U, 0U, 255U };
        GXColorUNORM const                  _bColor { 223U, 79U, 88U, 255U };
        GXColorUNORM const                  _defaultColor { 255U, 255U, 255U, 255U };

    public:
        Collision () = default;

        Collision ( Collision const & ) = delete;
        Collision &operator = ( Collision const & ) = delete;

        Collision ( Collision && ) = delete;
        Collision &operator = ( Collision && ) = delete;

        ~Collision () override = default;

    private:
        [[nodiscard]] bool IsReady () noexcept override;
        [[nodiscard]] bool OnFrame ( android_vulkan::Renderer &renderer, double deltaTime ) noexcept override;

        [[nodiscard]] bool OnInitDevice ( android_vulkan::Renderer &renderer ) noexcept override;
        void OnDestroyDevice ( android_vulkan::Renderer &renderer ) noexcept override;

        [[nodiscard]] bool OnSwapchainCreated ( android_vulkan::Renderer &renderer ) noexcept override;
        void OnSwapchainDestroyed ( android_vulkan::Renderer &renderer ) noexcept override;

        [[nodiscard]] bool CreateCommandPool ( android_vulkan::Renderer &renderer ) noexcept;
        void DestroyCommandPool ( VkDevice device ) noexcept;

        [[nodiscard]] bool CreateScene ( android_vulkan::Renderer &renderer ) noexcept;
        void DestroyScene ( android_vulkan::Renderer &renderer ) noexcept;

        [[nodiscard]] static bool AppendCuboid ( android_vulkan::Renderer &renderer,
            VkCommandBuffer const* commandBuffers,
            VkFence const* fences,
            size_t &commandBufferConsumed,
            std::string &&tag,
            ComponentRef &visual,
            char const* material,
            GXColorUNORM color,
            android_vulkan::RigidBodyRef &physical,
            float x,
            float y,
            float z,
            float w,
            float h,
            float d
        ) noexcept;

        static void UpdateCuboid ( CubeInfo &cube ) noexcept;
};

} // namespace pbr::collision


#endif // PBR_COLLISION_HPP
