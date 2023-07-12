#ifndef PBR_COLLISION_HPP
#define PBR_COLLISION_HPP


#include <pbr/collision/manipulator.hpp>
#include <pbr/camera.hpp>
#include <pbr/point_light_component.hpp>
#include <pbr/render_session.hpp>
#include <game.hpp>
#include <physics.hpp>


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

        GXColorRGB const                    _aColor
        {
            static_cast<GXUByte> ( 115U ),
            static_cast<GXUByte> ( 185U ),
            static_cast<GXUByte> ( 0U ),
            static_cast<GXUByte> ( 255U )
        };

        GXColorRGB const                    _bColor
        {
            static_cast<GXUByte> ( 223U ),
            static_cast<GXUByte> ( 79U ),
            static_cast<GXUByte> ( 88U ),
            static_cast<GXUByte> ( 255U )
        };

        GXColorRGB const                    _defaultColor
        {
            static_cast<GXUByte> ( 255U ),
            static_cast<GXUByte> ( 255U ),
            static_cast<GXUByte> ( 255U ),
            static_cast<GXUByte> ( 255U )
        };

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

        static void UpdateCuboid ( CubeInfo &cube ) noexcept;
};

} // namespace pbr::collision


#endif // PBR_COLLISION_HPP
