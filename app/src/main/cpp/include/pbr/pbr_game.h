#ifndef PBR_GAME_H
#define PBR_GAME_H


#include <GXCommon/GXWarning.h>

GX_DISABLE_COMMON_WARNINGS

#include <list>

GX_RESTORE_WARNING_STATE

#include <game.h>
#include <physics.h>
#include "camera.h"
#include "render_session.h"
#include "point_light_component.h"


namespace pbr {

class PBRGame final : public android_vulkan::Game
{
    private:
        Camera                              _camera;
        VkCommandPool                       _commandPool;
        std::vector<VkCommandBuffer>        _commandBuffers;

        ComponentRef                        _cube;
        android_vulkan::RigidBodyRef        _cubeBody;
        ComponentRef                        _floor;
        android_vulkan::RigidBodyRef        _floorBody;
        float                               _floorPhase;
        GXVec3                              _floorRenderOffset;
        android_vulkan::Physics             _physics;

        RenderSession                       _renderSession;
        std::list<ComponentRef>             _components;

        // Just for fun - point light trajectory animation implementation.
        PointLight*                         _pointLight;
        float                               _lightPhase;
        GXVec3                              _lightOrigin;

    public:
        PBRGame () noexcept;

        PBRGame ( PBRGame const & ) = delete;
        PBRGame& operator = ( PBRGame const & ) = delete;

        PBRGame ( PBRGame && ) = delete;
        PBRGame& operator = ( PBRGame && ) = delete;

        ~PBRGame () override = default;

    private:
        [[nodiscard]] bool IsReady () override;
        [[nodiscard]] bool OnFrame ( android_vulkan::Renderer &renderer, double deltaTime ) override;

        [[nodiscard]] bool OnInitDevice ( android_vulkan::Renderer &renderer ) override;
        void OnDestroyDevice ( VkDevice device ) override;

        [[nodiscard]] bool OnSwapchainCreated ( android_vulkan::Renderer &renderer ) override;
        void OnSwapchainDestroyed ( VkDevice device ) override;

        void DestroyCommandPool ( VkDevice device ) noexcept;
        [[nodiscard]] bool UploadGPUContent ( android_vulkan::Renderer &renderer ) noexcept;

        void CreatePhysics () noexcept;
        void DestroyPhysics () noexcept;
        void UpdatePhysicsActors ( float deltaTime ) noexcept;
};

} // namespace pbr


#endif // PBR_GAME_H
