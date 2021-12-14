#ifndef PBR_COLLISION_H
#define PBR_COLLISION_H


#include <pbr/collision/manipulator.h>
#include <pbr/camera.h>
#include <pbr/point_light_component.h>
#include <pbr/render_session.h>
#include <game.h>
#include <physics.h>


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

        android_vulkan::Color32 const       _aColor { 115U, 185U, 0U, 255U };
        android_vulkan::Color32 const       _bColor { 223U, 79U, 88U, 255U };
        android_vulkan::Color32 const       _defaultColor { 255U, 255U, 255U, 255U };

    public:
        Collision () = default;

        Collision ( Collision const & ) = delete;
        Collision& operator = ( Collision const & ) = delete;

        Collision ( Collision && ) = delete;
        Collision& operator = ( Collision && ) = delete;

        ~Collision () override = default;

    private:
        [[nodiscard]] bool IsReady () noexcept override;
        [[nodiscard]] bool OnFrame ( android_vulkan::Renderer &renderer, double deltaTime ) noexcept override;

        [[nodiscard]] bool OnInitDevice ( android_vulkan::Renderer &renderer ) noexcept override;
        void OnDestroyDevice ( VkDevice device ) noexcept override;

        [[nodiscard]] bool OnSwapchainCreated ( android_vulkan::Renderer &renderer ) noexcept override;
        void OnSwapchainDestroyed ( VkDevice device ) noexcept override;

        [[nodiscard]] bool CreateCommandPool ( android_vulkan::Renderer &renderer ) noexcept;
        void DestroyCommandPool ( VkDevice device ) noexcept;

        [[nodiscard]] bool CreateScene ( android_vulkan::Renderer &renderer ) noexcept;
        void DestroyScene ( VkDevice device ) noexcept;

        [[nodiscard]] static bool AppendCuboid ( android_vulkan::Renderer &renderer,
            VkCommandBuffer const* commandBuffers,
            size_t &commandBufferConsumed,
            std::string &&tag,
            ComponentRef &visual,
            char const* material,
            android_vulkan::Color32 const &color,
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


#endif // PBR_COLLISION_H
