#ifndef PBR_ACTOR_H
#define PBR_ACTOR_H


#include "actor_desc.h"
#include "component_classes.h"
#include "types.h"
#include <physics.h>

GX_DISABLE_COMMON_WARNINGS

extern "C" {

#include <lua/lstate.h>

} // extern "C"

#include <deque>
#include <unordered_map>

GX_RESTORE_WARNING_STATE


namespace pbr {

class Scene;

class Actor final
{
    public:
        // Forward declaration. This will init '_registerHandlers' static field.
        class StaticInitializer;

    private:
        using DestroyHander = void ( Actor::* ) ( Component &component ) noexcept;

        using RegisterFromNativeHander = void ( Actor::* ) ( ComponentRef &component,
            ComponentList &renderable,
            android_vulkan::Physics &physics,
            lua_State &vm
        ) noexcept;

        using RegisterFromScriptHander = void ( Actor::* ) ( ComponentRef &component,
            ComponentList &renderable,
            android_vulkan::Physics &physics
        ) noexcept;

        using SpawnActors = std::unordered_map<Actor const*, ActorRef>;

    private:
        std::deque<ComponentRef>            _components {};
        std::string                         _name;
        Scene*                              _scene = nullptr;
        TransformableList                   _transformableComponents {};

        static int                          _appendComponentFromNativeIndex;
        static int                          _makeActorIndex;

        static DestroyHander                _destroyHandlers[ static_cast<size_t> ( ClassID::COUNT ) ];
        static RegisterFromNativeHander     _registerFromNativeHandlers[ static_cast<size_t> ( ClassID::COUNT ) ];
        static RegisterFromScriptHander     _registerFromScriptHandlers[ static_cast<size_t> ( ClassID::COUNT ) ];

        static SpawnActors                  _spawnActors;

    public:
        Actor () = delete;

        Actor ( Actor const & ) = delete;
        Actor& operator = ( Actor const & ) = delete;

        Actor ( Actor && ) = delete;
        Actor& operator = ( Actor && ) = delete;

        explicit Actor ( std::string &&name ) noexcept;
        explicit Actor ( ActorDesc const &desc, uint8_t const* data ) noexcept;

        ~Actor () = default;

        void AppendComponent ( ComponentRef &component ) noexcept;
        void DestroyComponent ( Component &component ) noexcept;
        [[nodiscard]] std::string const& GetName () const noexcept;

        // Note transform must be in render units.
        void OnTransform ( GXMat4 const &transformWorld ) noexcept;

        void RegisterComponentsFromNative ( Scene &scene,
            ComponentList &renderable,
            android_vulkan::Physics &physics,
            lua_State &vm
        ) noexcept;

        void RegisterComponentsFromScript ( Scene &scene,
            ComponentList &renderable,
            android_vulkan::Physics &physics
        ) noexcept;

        static bool Init ( lua_State &vm ) noexcept;
        static void Destroy () noexcept;
        [[nodiscard]] static ActorRef& GetReference ( Actor const &handle ) noexcept;

    private:
        void AppendCameraComponentFromNative ( ComponentRef &component,
            ComponentList &renderable,
            android_vulkan::Physics &physics,
            lua_State &vm
        ) noexcept;

        void AppendCameraComponentFromScript ( ComponentRef &component,
            ComponentList &renderable,
            android_vulkan::Physics &physics
        ) noexcept;

        void AppendPointLightComponentFromNative ( ComponentRef &component,
            ComponentList &renderable,
            android_vulkan::Physics &physics,
            lua_State &vm
        ) noexcept;

        void AppendPointLightComponentFromScript ( ComponentRef &component,
            ComponentList &renderable,
            android_vulkan::Physics &physics
        ) noexcept;

        void AppendReflectionComponentFromNative ( ComponentRef &component,
            ComponentList &renderable,
            android_vulkan::Physics &physics,
            lua_State &vm
        ) noexcept;

        void AppendReflectionComponentFromScript ( ComponentRef &component,
            ComponentList &renderable,
            android_vulkan::Physics &physics
        ) noexcept;

        void AppendRigidBodyComponentFromNative ( ComponentRef &component,
            ComponentList &renderable,
            android_vulkan::Physics &physics,
            lua_State &vm
        ) noexcept;

        void AppendRigidBodyComponentFromScript ( ComponentRef &component,
            ComponentList &renderable,
            android_vulkan::Physics &physics
        ) noexcept;

        void AppendScriptComponentFromNative ( ComponentRef &component,
            ComponentList &renderable,
            android_vulkan::Physics &physics,
            lua_State &vm
        ) noexcept;

        void AppendScriptComponentFromScript ( ComponentRef &component,
            ComponentList &renderable,
            android_vulkan::Physics &physics
        ) noexcept;

        void AppendSoundEmitterGlobalComponentFromNative ( ComponentRef &component,
            ComponentList &renderable,
            android_vulkan::Physics &physics,
            lua_State &vm
        ) noexcept;

        void AppendSoundEmitterGlobalComponentFromScript ( ComponentRef &component,
            ComponentList &renderable,
            android_vulkan::Physics &physics
        ) noexcept;

        void AppendStaticMeshComponentFromNative ( ComponentRef &component,
            ComponentList &renderable,
            android_vulkan::Physics &physics,
            lua_State &vm
        ) noexcept;

        void AppendStaticMeshComponentFromScript ( ComponentRef &component,
            ComponentList &renderable,
            android_vulkan::Physics &physics
        ) noexcept;

        void AppendTransformComponentFromNative ( ComponentRef &component,
            ComponentList &renderable,
            android_vulkan::Physics &physics,
            lua_State &vm
        ) noexcept;

        void AppendTransformComponentFromScript ( ComponentRef &component,
            ComponentList &renderable,
            android_vulkan::Physics &physics
        ) noexcept;

        void AppendUnknownComponentFromNative ( ComponentRef &component,
            ComponentList &renderable,
            android_vulkan::Physics &physics,
            lua_State &vm
        ) noexcept;

        void AppendUnknownComponentFromScript ( ComponentRef &component,
            ComponentList &renderable,
            android_vulkan::Physics &physics
        ) noexcept;

        void DestroyRigidBodyComponent ( Component &component ) noexcept;
        void DestroyStaticMeshComponent ( Component &component ) noexcept;
        void DestroyComponentStub ( Component &component ) noexcept;

        void RemoveComponent ( Component const &component ) noexcept;

        [[nodiscard]] static int OnAppendComponent ( lua_State* state );
        [[nodiscard]] static int OnCreate ( lua_State* state );
        [[nodiscard]] static int OnDestroy ( lua_State* state );
        [[nodiscard]] static int OnGarbageCollected ( lua_State* state );
        [[nodiscard]] static int OnGetName ( lua_State* state );
};

} // namespace pbr


#endif // PBR_ACTOR_H
