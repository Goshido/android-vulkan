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
            ComponentRef                                _component;
            android_vulkan::RigidBodyRef                _body;
        };

    private:
        Camera                                          _camera {};
        VkCommandPool                                   _commandPool = VK_NULL_HANDLE;
        std::vector<VkCommandBuffer>                    _commandBuffers {};

        ComponentRef                                    _cameraLight {};
        std::vector<CubeInfo>                           _cubes {};

        android_vulkan::ContactDetector                 _contactDetector {};
        android_vulkan::ContactManager                  _contactManager {};

        MeshRef                                         _contactMesh {};
        MaterialRef                                     _contactMaterial {};
        Manipulator                                     _manipulator {};
        RenderSession                                   _renderSession {};

        android_vulkan::Half4 const                     _aColor
        {
            115.0F * GX_MATH_UNORM_FACTOR,
            185.0F * GX_MATH_UNORM_FACTOR,
            0.0F * GX_MATH_UNORM_FACTOR,
            1.0F
        };

        android_vulkan::Half4 const                     _bColor
        {
            222.6F * GX_MATH_UNORM_FACTOR,
            79.2F * GX_MATH_UNORM_FACTOR,
            87.9F * GX_MATH_UNORM_FACTOR,
            1.0F
        };

        android_vulkan::Half4 const                     _defaultColor { 1.0F, 1.0F, 1.0F, 1.0F };

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

        [[nodiscard]] bool AppendCuboid ( android_vulkan::Renderer &renderer,
            VkCommandBuffer const* commandBuffers,
            size_t &commandBufferConsumed,
            std::string &&tag,
            ComponentRef &visual,
            char const* material,
            android_vulkan::Half4 const &color,
            android_vulkan::RigidBodyRef &physical,
            float x,
            float y,
            float z,
            float w,
            float h,
            float d
        ) noexcept;

        [[nodiscard]] bool CreateCommandPool ( android_vulkan::Renderer &renderer ) noexcept;
        void DestroyCommandPool ( VkDevice device ) noexcept;

        [[nodiscard]] bool CreateScene ( android_vulkan::Renderer &renderer ) noexcept;
        void DestroyScene ( VkDevice device ) noexcept;

        static void UpdateCuboid ( CubeInfo &cube ) noexcept;
};

} // namespace pbr::collision


#endif // PBR_COLLISION_H
