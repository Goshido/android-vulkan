#ifndef PBR_STIPPLE_TEST_H
#define PBR_STIPPLE_TEST_H


#include <game.h>
#include <pbr/camera.h>
#include <pbr/point_light_component.h>
#include <pbr/render_session.h>
#include <pbr/static_mesh_component.h>


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

        GXColorRGB                              _stippleColor
        {
            static_cast<GXUByte> ( 165U ),
            static_cast<GXUByte> ( 236U ),
            static_cast<GXUByte> ( 49U ),
            static_cast<GXUByte> ( 255U )
        };

        float                                   _stippleAnimation = 0.0F;

    public:
        StippleTest () = default;

        StippleTest ( StippleTest const & ) = delete;
        StippleTest& operator = ( StippleTest const & ) = delete;

        StippleTest ( StippleTest && ) = delete;
        StippleTest& operator = ( StippleTest && ) = delete;

        ~StippleTest () override = default;

    private:
        [[nodiscard]] bool IsReady () noexcept override;
        [[nodiscard]] bool OnFrame ( android_vulkan::Renderer &renderer, double deltaTime ) noexcept override;

        [[nodiscard]] bool OnInitDevice ( android_vulkan::Renderer &renderer ) noexcept override;
        void OnDestroyDevice ( VkDevice device ) noexcept override;

        [[nodiscard]] bool OnSwapchainCreated ( android_vulkan::Renderer &renderer ) noexcept override;
        void OnSwapchainDestroyed ( VkDevice device ) noexcept override;

        void Animate ( float deltaTime ) noexcept;
        [[nodiscard]] bool CreateScene ( android_vulkan::Renderer &renderer ) noexcept;
};

} // namespace pbr::stipple_test


#endif // PBR_STIPPLE_TEST_H
