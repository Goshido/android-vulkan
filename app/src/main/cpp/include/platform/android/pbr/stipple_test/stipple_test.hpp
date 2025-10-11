#ifndef PBR_STIPPLE_TEST_HPP
#define PBR_STIPPLE_TEST_HPP


#include <game.hpp>
#include <platform/android/pbr/camera.hpp>
#include <platform/android/pbr/point_light_component.hpp>
#include <platform/android/pbr/render_session.hpp>
#include <platform/android/pbr/static_mesh_component.hpp>


namespace pbr::stipple_test {

class StippleTest final : public android_vulkan::Game
{
    private:
        Camera                                  _camera {};
        VkCommandPool                           _commandPool = VK_NULL_HANDLE;
        std::unique_ptr<StaticMeshComponent>    _floor {};
        PointLightComponent                     _light {};
        RenderSession                           _renderSession {};
        std::unique_ptr<StaticMeshComponent>    _stipple {};

        GXColorUNORM                            _stippleColor { 165U, 236U, 49U, 255U };
        float                                   _stippleAnimation = 0.0F;

    public:
        StippleTest () = default;

        StippleTest ( StippleTest const & ) = delete;
        StippleTest &operator = ( StippleTest const & ) = delete;

        StippleTest ( StippleTest && ) = delete;
        StippleTest &operator = ( StippleTest && ) = delete;

        ~StippleTest () override = default;

    private:
        [[nodiscard]] bool IsReady () noexcept override;
        [[nodiscard]] bool OnFrame ( android_vulkan::Renderer &renderer, double deltaTime ) noexcept override;

        [[nodiscard]] bool OnInitDevice ( android_vulkan::Renderer &renderer ) noexcept override;
        void OnDestroyDevice ( android_vulkan::Renderer &renderer ) noexcept override;

        [[nodiscard]] bool OnSwapchainCreated ( android_vulkan::Renderer &renderer ) noexcept override;
        void OnSwapchainDestroyed ( android_vulkan::Renderer &renderer ) noexcept override;

        void Animate ( float deltaTime ) noexcept;
        [[nodiscard]] bool CreateScene ( android_vulkan::Renderer &renderer ) noexcept;
};

} // namespace pbr::stipple_test


#endif // PBR_STIPPLE_TEST_HPP
