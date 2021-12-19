#ifndef PBR_SWEEP_TESTING_H
#define PBR_SWEEP_TESTING_H


#include "actor_body.h"
#include "actor_sweep.h"
#include <pbr/camera.h>
#include <pbr/point_light_component.h>
#include <game.h>


namespace pbr::sweep_testing {

class SweepTesting final : public android_vulkan::Game
{
    private:
        enum class eControlType : uint8_t
        {
            Camera,
            SweepObject
        };

    private:
        constexpr static size_t                     GRID_X = 4U;
        constexpr static size_t                     GRID_Z = 4U;

        std::array<ActorBody, GRID_X * GRID_Z>      _bodies {};
        ActorSweep                                  _sweep {};
        Camera                                      _camera {};
        VkCommandPool                               _commandPool = VK_NULL_HANDLE;
        eControlType                                _controlType = eControlType::SweepObject;
        PointLightComponent                         _light {};
        Texture2DRef                                _overlay {};
        android_vulkan::Physics                     _physics {};
        RenderSession                               _renderSession {};

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

        void BindControls () noexcept;
        void UnbindControls () noexcept;

        [[nodiscard]] bool CreateCommandPool ( android_vulkan::Renderer &renderer ) noexcept;
        void DestroyCommandPool ( VkDevice device ) noexcept;

        [[nodiscard]] bool CreateScene ( android_vulkan::Renderer &renderer ) noexcept;

        static void OnSwitchControls ( void* context ) noexcept;
};

} // namespace pbr::sweep_testing


#endif // PBR_SWEEP_TESTING_H
