#ifndef PBR_SCENE_H
#define PBR_SCENE_H


#include "actor.h"
#include "camera_component.h"
#include "render_session.h"
#include "scriptable_gamepad.h"

GX_DISABLE_COMMON_WARNINGS

extern "C" {

#include <lua/lstate.h>

} // extern "C"

GX_RESTORE_WARNING_STATE


namespace pbr {

class Scene final
{
    private:
        std::deque<ActorRef>        _actors {};

        CameraComponent*            _camera = nullptr;
        CameraComponent             _defaultCamera { "Default Camera" };
        ScriptableGamepad           _gamepad {};
        android_vulkan::Physics*    _physics = nullptr;

        ComponentList               _freeTransferResourceList {};
        ComponentList               _renderableList {};

        lua_Number                  _aspectRatio = 1.0;
        lua_Integer                 _width = -1;
        lua_Integer                 _height = -1;

        int                         _appendActorIndex = std::numeric_limits<int>::max ();
        int                         _onInputIndex = std::numeric_limits<int>::max ();
        int                         _onPostPhysicsIndex = std::numeric_limits<int>::max ();
        int                         _onPrePhysicsIndex = std::numeric_limits<int>::max ();
        int                         _onRenderTargetChangedIndex = std::numeric_limits<int>::max ();
        int                         _onUpdateIndex = std::numeric_limits<int>::max ();

        int                         _sceneHandle = std::numeric_limits<int>::max ();

        lua_State*                  _vm = nullptr;

    public:
        Scene () = default;

        Scene ( Scene const & ) = delete;
        Scene& operator = ( Scene const & ) = delete;

        Scene ( Scene && ) = delete;
        Scene& operator = ( Scene && ) = delete;

        ~Scene () = default;

        [[nodiscard]] bool ExecuteInputEvents () noexcept;

        [[nodiscard]] GXMat4 const& GetActiveCameraLocalMatrix () const noexcept;
        [[nodiscard]] GXMat4 const& GetActiveCameraProjectionMatrix () const noexcept;

        void OnCaptureInput () noexcept;
        void OnReleaseInput () const noexcept;

        [[nodiscard]] bool OnInitDevice ( android_vulkan::Physics &physics ) noexcept;
        void OnDestroyDevice () noexcept;

        [[nodiscard]] bool OnPrePhysics ( double deltaTime ) noexcept;
        [[nodiscard]] bool OnPostPhysics ( double deltaTime ) noexcept;
        [[nodiscard]] bool OnResolutionChanged ( VkExtent2D const &resolution, double aspectRatio ) noexcept;
        [[nodiscard]] bool OnUpdate ( double deltaTime ) noexcept;

        void AppendActor ( ActorRef &actor ) noexcept;
        void FreeTransferResources ( VkDevice device ) noexcept;
        void Submit ( RenderSession &renderSession ) noexcept;

    private:
        [[nodiscard]] static int OnGetPhysicsToRendererScaleFactor ( lua_State* state );
        [[nodiscard]] static int OnGetRendererToPhysicsScaleFactor ( lua_State* state );

        [[nodiscard]] static int OnGetRenderTargetAspectRatio ( lua_State* state );
        [[nodiscard]] static int OnGetRenderTargetWidth ( lua_State* state );
        [[nodiscard]] static int OnGetRenderTargetHeight ( lua_State* state );

        [[nodiscard]] static int OnQuit ( lua_State* state );
        [[nodiscard]] static int OnSetActiveCamera ( lua_State* state );
};

} // namespace pbr


#endif // PBR_SCENE_H
