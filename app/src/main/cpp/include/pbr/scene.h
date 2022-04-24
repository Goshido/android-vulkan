#ifndef PBR_SCENE_H
#define PBR_SCENE_H


#include "actor.h"
#include "render_session.h"

GX_DISABLE_COMMON_WARNINGS

extern "C" {

#include <lua/lstate.h>

} // extern "C"

#include <list>

GX_RESTORE_WARNING_STATE


namespace pbr {

class Scene final
{
    public:
        using Actors = std::vector<ActorRef>;

    private:
        std::unordered_map<std::string, Actors>     _actorStorage {};

        ComponentList                               _freeTransferResourceList {};
        ComponentList                               _renderableList {};

        android_vulkan::Physics*                    _physics = nullptr;
        ScriptEngine*                               _scriptEngine = nullptr;

        int                                         _onPostPhysicsIndex = std::numeric_limits<int>::max ();
        int                                         _onPrePhysicsIndex = std::numeric_limits<int>::max ();
        int                                         _onUpdateIndex = std::numeric_limits<int>::max ();

        int                                         _sceneHandle = std::numeric_limits<int>::max ();

        lua_State*                                  _vm = nullptr;

    public:
        Scene () = default;

        Scene ( Scene const & ) = delete;
        Scene& operator = ( Scene const & ) = delete;

        Scene ( Scene && ) = delete;
        Scene& operator = ( Scene && ) = delete;

        ~Scene () = default;

        [[nodiscard]] bool OnInitDevice ( android_vulkan::Physics &physics ) noexcept;
        void OnDestroyDevice () noexcept;

        [[nodiscard]] bool OnPrePhysics ( double deltaTime ) noexcept;
        [[nodiscard]] bool OnPostPhysics ( double deltaTime ) noexcept;
        [[nodiscard]] bool OnUpdate ( double deltaTime ) noexcept;

        void AppendActor ( ActorRef &actor ) noexcept;
        void FreeTransferResources ( VkDevice device ) noexcept;
        void Submit ( RenderSession &renderSession ) noexcept;
};

} // namespace pbr


#endif // PBR_SCENE_H
