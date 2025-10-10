#ifndef PBR_SWEEP_TESTING_HPP
#define PBR_SWEEP_TESTING_HPP


#include "actor_body.hpp"
#include "actor_sweep.hpp"
#include <game.hpp>
#include <platform/android/pbr/camera.hpp>
#include <platform/android/pbr/point_light_component.hpp>


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
        constexpr static size_t                         GRID_X = 4U;
        constexpr static size_t                         GRID_Z = 4U;

        std::array<ActorBody, GRID_X * GRID_Z>          _bodies {};
        Camera                                          _camera {};
        VkCommandPool                                   _commandPool = VK_NULL_HANDLE;
        eControlType                                    _controlType = eControlType::SweepObject;
        PointLightComponent                             _light {};
        Texture2DRef                                    _overlay {};
        android_vulkan::Physics                         _physics {};
        RenderSession                                   _renderSession {};
        ActorSweep                                      _sweep {};
        std::vector<android_vulkan::RigidBodyRef>       _sweepResult {};

    public:
        SweepTesting () = default;

        SweepTesting ( SweepTesting const & ) = delete;
        SweepTesting &operator = ( SweepTesting const & ) = delete;

        SweepTesting ( SweepTesting && ) = delete;
        SweepTesting &operator = ( SweepTesting && ) = delete;

        ~SweepTesting () override = default;

    private:
        [[nodiscard]] bool IsReady () noexcept override;
        [[nodiscard]] bool OnFrame ( android_vulkan::Renderer &renderer, double deltaTime ) noexcept override;

        [[nodiscard]] bool OnInitDevice ( android_vulkan::Renderer &renderer ) noexcept override;
        void OnDestroyDevice ( android_vulkan::Renderer &renderer ) noexcept override;

        [[nodiscard]] bool OnSwapchainCreated ( android_vulkan::Renderer &renderer ) noexcept override;
        void OnSwapchainDestroyed ( android_vulkan::Renderer &renderer ) noexcept override;

        void BindControls () noexcept;
        void UnbindControls () noexcept;

        [[nodiscard]] bool CreateCommandPool ( android_vulkan::Renderer &renderer ) noexcept;
        void DestroyCommandPool ( VkDevice device ) noexcept;

        [[nodiscard]] bool CreateScene ( android_vulkan::Renderer &renderer ) noexcept;
        void DoSweepTest () noexcept;

        static void OnSwitchControls ( void* context ) noexcept;
};

} // namespace pbr::sweep_testing


#endif // PBR_SWEEP_TESTING_HPP
