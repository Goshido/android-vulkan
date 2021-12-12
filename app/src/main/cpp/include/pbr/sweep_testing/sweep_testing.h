#ifndef PBR_SWEEP_TESTING_H
#define PBR_SWEEP_TESTING_H


#include "actor.h"
#include <pbr/render_session.h>
#include <pbr/camera.h>
#include <pbr/point_light_component.h>
#include <game.h>


namespace pbr::sweep_testing {

class SweepTesting final : public android_vulkan::Game
{
    private:
        constexpr static size_t                 GRID_X = 4U;
        constexpr static size_t                 GRID_Z = 4U;

        std::array<Actor, GRID_X * GRID_Z>      _actors {};
        Camera                                  _camera {};
        VkCommandPool                           _commandPool = VK_NULL_HANDLE;
        PointLightComponent                     _light {};
        Texture2DRef                            _overlay {};
        android_vulkan::Physics                 _physics {};
        RenderSession                           _renderSession {};

    public:
        SweepTesting () = default;

        SweepTesting ( SweepTesting const & ) = delete;
        SweepTesting& operator = ( SweepTesting const & ) = delete;

        SweepTesting ( SweepTesting && ) = delete;
        SweepTesting& operator = ( SweepTesting && ) = delete;

        ~SweepTesting () override = default;

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
};

} // namespace pbr::sweep_testing


#endif // PBR_SWEEP_TESTING_H
