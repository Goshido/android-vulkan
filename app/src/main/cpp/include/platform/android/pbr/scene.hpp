#ifndef PBR_SCENE_HPP
#define PBR_SCENE_HPP


#include "actor.hpp"
#include "camera_component.hpp"
#include <platform/android/pbr/font_storage.hpp>
#include <pbr/scriptable_penetration.hpp>
#include <pbr/scriptable_raycast_result.hpp>
#include "render_session.hpp"
#include "renderable_component.hpp"
#include "scriptable_gamepad.hpp"
#include <sound_mixer.hpp>


namespace pbr {

class Scene final
{
    private:
        std::deque<ActorRef>                            _actors {};

        CameraComponent*                                _camera = nullptr;
        CameraComponent                                 _defaultCamera { "Default Camera" };
        MaterialRef                                     _defaultMaterial {};

        android_vulkan::EPA                             _epa {};
        ScriptableGamepad                               _gamepad {};
        std::vector<android_vulkan::Penetration>        _penetrations {};
        android_vulkan::Physics*                        _physics = nullptr;
        ScriptablePenetration                           _scriptablePenetration {};
        ScriptableRaycastResult                         _scriptableRaycastResult {};
        android_vulkan::ShapeRef                        _shapeBoxes[ 2U ] = {};
        android_vulkan::SoundMixer                      _soundMixer {};
        std::vector<android_vulkan::RigidBodyRef>       _sweepTestResult {};

        ComponentList                                   _renderableList {};
        RenderSession*                                  _renderSession = nullptr;
        UILayerList                                     _uiLayerList {};

        lua_Number                                      _aspectRatio = 1.0F;
        lua_Integer                                     _width = -1;
        lua_Integer                                     _height = -1;

        int                                             _appendActorFromNativeIndex = std::numeric_limits<int>::max ();
        int                                             _onAnimationUpdateIndex = std::numeric_limits<int>::max ();
        int                                             _onInputIndex = std::numeric_limits<int>::max ();
        int                                             _onPostPhysicsIndex = std::numeric_limits<int>::max ();
        int                                             _onPrePhysicsIndex = std::numeric_limits<int>::max ();
        int                                             _onRenderTargetChangedIndex = std::numeric_limits<int>::max ();
        int                                             _onUpdateIndex = std::numeric_limits<int>::max ();

        int                                             _sceneHandle = std::numeric_limits<int>::max ();

        lua_State*                                      _vm = nullptr;

    public:
        Scene () = default;

        Scene ( Scene const & ) = delete;
        Scene &operator = ( Scene const & ) = delete;

        Scene ( Scene && ) = delete;
        Scene &operator = ( Scene && ) = delete;

        ~Scene () = default;

        void DetachRenderable ( RenderableComponent const &component ) noexcept;
        [[nodiscard]] bool ExecuteInputEvents () noexcept;

        [[nodiscard]] GXMat4 const &GetActiveCameraLocalMatrix () const noexcept;
        [[nodiscard]] GXMat4 const &GetActiveCameraProjectionMatrix () const noexcept;
        [[nodiscard]] android_vulkan::Physics &GetPhysics () noexcept;

        [[nodiscard]] bool OnAnimationUpdated ( float deltaTime ) noexcept;

        [[nodiscard]] bool OnInitDevice ( android_vulkan::Renderer &renderer,
            RenderSession &renderSession,
            android_vulkan::Physics &physics
        ) noexcept;

        void OnDestroyDevice () noexcept;

        [[nodiscard]] bool OnInitSoundSystem () noexcept;
        void OnDestroySoundSystem () noexcept;

        void OnPause () noexcept;
        void OnResume () noexcept;

        [[nodiscard]] bool OnPrePhysics ( float deltaTime ) noexcept;
        [[nodiscard]] bool OnPostPhysics ( float deltaTime ) noexcept;
        [[nodiscard]] bool OnResolutionChanged ( VkExtent2D const &resolution, float aspectRatio ) noexcept;
        [[nodiscard]] bool OnUpdate ( float deltaTime ) noexcept;
        void OnUpdateAnimations ( float deltaTime, size_t commandBufferIndex ) noexcept;

        [[nodiscard]] bool LoadScene ( android_vulkan::Renderer &renderer,
            char const* scene,
            VkCommandPool commandPool
        ) noexcept;

        void RemoveActor ( Actor const &actor ) noexcept;
        void Submit ( android_vulkan::Renderer &renderer ) noexcept;

    private:
        void AppendActor ( ActorRef &actor ) noexcept;

        [[nodiscard]] int DoOverlapTestBoxBox ( lua_State &vm,
            GXMat4 const &localA,
            GXVec3 const &sizeA,
            GXMat4 const &localB,
            GXVec3 const &sizeB
        ) noexcept;

        [[nodiscard]] int DoPenetrationBox ( lua_State &vm,
            GXMat4 const &local,
            GXVec3 const &size,
            uint32_t groups
        ) noexcept;

        [[nodiscard]] int DoRaycast ( lua_State &vm, GXVec3 const &from, GXVec3 const &to, uint32_t groups ) noexcept;

        [[nodiscard]] int DoSweepTestBox ( lua_State &vm,
            GXMat4 const &local,
            GXVec3 const &size,
            uint32_t groups
        ) noexcept;

        void SubmitComponents ( android_vulkan::Renderer &renderer, RenderSession &renderSession ) noexcept;
        void SubmitUI ( android_vulkan::Renderer &renderer, RenderSession &renderSession ) noexcept;

        static void FreeTransferResources ( android_vulkan::Renderer &renderer ) noexcept;

        [[nodiscard]] static int OnAppendActor ( lua_State* state );
        [[nodiscard]] static int OnAppendUILayer ( lua_State* state );
        [[nodiscard]] static int OnDetachUILayer ( lua_State* state );
        [[nodiscard]] static int OnGetPenetrationBox ( lua_State* state );
        [[nodiscard]] static int OnGetPhysicsToRendererScaleFactor ( lua_State* state );
        [[nodiscard]] static int OnGetRendererToPhysicsScaleFactor ( lua_State* state );

        [[nodiscard]] static int OnGetRenderTargetAspectRatio ( lua_State* state );
        [[nodiscard]] static int OnGetRenderTargetWidth ( lua_State* state );
        [[nodiscard]] static int OnGetRenderTargetHeight ( lua_State* state );

        [[nodiscard]] static int OnOverlapTestBoxBox ( lua_State* state );

        [[nodiscard]] static int OnQuit ( lua_State* state );
        [[nodiscard]] static int OnRaycast ( lua_State* state );
        [[nodiscard]] static int OnSetActiveCamera ( lua_State* state );
        [[nodiscard]] static int OnSetBrightness ( lua_State* state );
        [[nodiscard]] static int OnSetExposureCompensation ( lua_State* state );
        [[nodiscard]] static int OnSetExposureMaximumBrightness ( lua_State* state );
        [[nodiscard]] static int OnSetExposureMinimumBrightness ( lua_State* state );
        [[nodiscard]] static int OnSetEyeAdaptationSpeed ( lua_State* state );
        [[nodiscard]] static int OnSetSoundChannelVolume ( lua_State* state );
        [[nodiscard]] static int OnSetSoundListenerTransform ( lua_State* state );
        [[nodiscard]] static int OnSetSoundMasterVolume ( lua_State* state );
        [[nodiscard]] static int OnSweepTestBox ( lua_State* state );
};

} // namespace pbr


#endif // PBR_SCENE_HPP
