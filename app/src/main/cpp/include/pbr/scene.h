#ifndef PBR_SCENE_H
#define PBR_SCENE_H


#include "actor.h"
#include "camera_component.h"
#include "render_session.h"
#include "renderable_component.h"
#include "scriptable_gamepad.h"
#include "scriptable_penetration.h"
#include <sound_mixer.h>


namespace pbr {

class Scene final
{
    private:
        std::deque<ActorRef>                            _actors {};

        CameraComponent*                                _camera = nullptr;
        CameraComponent                                 _defaultCamera { "Default Camera" };

        android_vulkan::EPA                             _epa {};
        ScriptableGamepad                               _gamepad {};
        std::vector<android_vulkan::Penetration>        _penetrations {};
        android_vulkan::Physics*                        _physics = nullptr;
        ScriptablePenetration                           _scriptablePenetration {};
        android_vulkan::ShapeRef                        _shapeBoxes[ 2U ] = {};
        android_vulkan::SoundMixer                      _soundMixer {};
        android_vulkan::SoundStorage                    _soundStorage {};
        std::vector<android_vulkan::RigidBodyRef>       _sweepTestResult {};

        ComponentList                                   _renderableList {};

        lua_Number                                      _aspectRatio = 1.0;
        lua_Integer                                     _width = -1;
        lua_Integer                                     _height = -1;

        int                                             _appendActorFromNativeIndex = std::numeric_limits<int>::max ();
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
        Scene& operator = ( Scene const & ) = delete;

        Scene ( Scene && ) = delete;
        Scene& operator = ( Scene && ) = delete;

        ~Scene () = default;

        void DetachRenderable ( RenderableComponent const &component ) noexcept;
        [[nodiscard]] bool ExecuteInputEvents () noexcept;

        [[nodiscard]] GXMat4 const& GetActiveCameraLocalMatrix () const noexcept;
        [[nodiscard]] GXMat4 const& GetActiveCameraProjectionMatrix () const noexcept;
        [[nodiscard]] android_vulkan::Physics& GetPhysics () noexcept;

        [[nodiscard]] bool OnInitDevice ( android_vulkan::Renderer &renderer,
            android_vulkan::Physics &physics ) noexcept;

        void OnDestroyDevice () noexcept;

        void OnPause () noexcept;
        void OnResume () noexcept;

        [[nodiscard]] bool OnPrePhysics ( double deltaTime ) noexcept;
        [[nodiscard]] bool OnPostPhysics ( double deltaTime ) noexcept;
        [[nodiscard]] bool OnResolutionChanged ( VkExtent2D const &resolution, double aspectRatio ) noexcept;
        [[nodiscard]] bool OnUpdate ( double deltaTime ) noexcept;

        [[nodiscard]] bool LoadScene ( android_vulkan::Renderer &renderer,
            char const* scene,
            VkCommandPool commandPool
        ) noexcept;

        void RemoveActor ( Actor const &actor ) noexcept;
        void Submit ( android_vulkan::Renderer &renderer, RenderSession &renderSession ) noexcept;

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

        [[nodiscard]] int DoSweepTestBox ( lua_State &vm,
            GXMat4 const &local,
            GXVec3 const &size,
            uint32_t groups
        ) noexcept;

        static void FreeTransferResources ( android_vulkan::Renderer &renderer ) noexcept;

        [[nodiscard]] static int OnAppendActor ( lua_State* state );
        [[nodiscard]] static int OnGetPenetrationBox ( lua_State* state );
        [[nodiscard]] static int OnGetPhysicsToRendererScaleFactor ( lua_State* state );
        [[nodiscard]] static int OnGetRendererToPhysicsScaleFactor ( lua_State* state );

        [[nodiscard]] static int OnGetRenderTargetAspectRatio ( lua_State* state );
        [[nodiscard]] static int OnGetRenderTargetWidth ( lua_State* state );
        [[nodiscard]] static int OnGetRenderTargetHeight ( lua_State* state );

        [[nodiscard]] static int OnOverlapTestBoxBox ( lua_State* state );

        [[nodiscard]] static int OnQuit ( lua_State* state );
        [[nodiscard]] static int OnSetActiveCamera ( lua_State* state );
        [[nodiscard]] static int OnSweepTestBox ( lua_State* state );
};

} // namespace pbr


#endif // PBR_SCENE_H
