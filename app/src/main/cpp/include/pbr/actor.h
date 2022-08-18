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

        using RegisterHander = void ( Actor::* ) ( ComponentRef &component,
            ComponentList &freeTransferResource,
            ComponentList &renderable,
            android_vulkan::Physics &physics,
            lua_State &vm
        ) noexcept;

    private:
        std::deque<ComponentRef>    _components {};
        std::string                 _name;
        Scene*                      _scene = nullptr;
        TransformableList           _transformableComponents {};

        static int                  _appendComponentIndex;
        static int                  _makeActorIndex;

        static DestroyHander        _destroyHandlers[ static_cast<size_t> ( ClassID::COUNT ) ];
        static RegisterHander       _registerHandlers[ static_cast<size_t> ( ClassID::COUNT ) ];

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

        // Note transfrom must be in render units.
        void OnTransform ( GXMat4 const &transformWorld ) noexcept;

        void RegisterComponents ( Scene &scene,
            ComponentList &freeTransferResource,
            ComponentList &renderable,
            android_vulkan::Physics &physics,
            lua_State &vm
        ) noexcept;

        static bool Init ( lua_State &vm ) noexcept;

    private:
        void AppendCameraComponent ( ComponentRef &component,
            ComponentList &freeTransferResource,
            ComponentList &renderable,
            android_vulkan::Physics &physics,
            lua_State &vm
        ) noexcept;

        void AppendPointLightComponent ( ComponentRef &component,
            ComponentList &freeTransferResource,
            ComponentList &renderable,
            android_vulkan::Physics &physics,
            lua_State &vm
        ) noexcept;

        void AppendReflectionComponent ( ComponentRef &component,
            ComponentList &freeTransferResource,
            ComponentList &renderable,
            android_vulkan::Physics &physics,
            lua_State &vm
        ) noexcept;

        void AppendRigidBodyComponent ( ComponentRef &component,
            ComponentList &freeTransferResource,
            ComponentList &renderable,
            android_vulkan::Physics &physics,
            lua_State &vm
        ) noexcept;

        void AppendScriptComponent ( ComponentRef &component,
            ComponentList &freeTransferResource,
            ComponentList &renderable,
            android_vulkan::Physics &physics,
            lua_State &vm
        ) noexcept;

        void AppendStaticMeshComponent ( ComponentRef &component,
            ComponentList &freeTransferResource,
            ComponentList &renderable,
            android_vulkan::Physics &physics,
            lua_State &vm
        ) noexcept;

        void AppendTransformComponent ( ComponentRef &component,
            ComponentList &freeTransferResource,
            ComponentList &renderable,
            android_vulkan::Physics &physics,
            lua_State &vm
        ) noexcept;

        void AppendUnknownComponent ( ComponentRef &component,
            ComponentList &freeTransferResource,
            ComponentList &renderable,
            android_vulkan::Physics &physics,
            lua_State &vm
        ) noexcept;

        void DestroyRigidBodyComponent ( Component &component ) noexcept;
        void DestroyStaticMeshComponent ( Component &component ) noexcept;
        void DestroyComponentStub ( Component &component ) noexcept;

        void RemoveComponent ( Component const &component ) noexcept;

        [[nodiscard]] static int OnDestroy ( lua_State* state );
        [[nodiscard]] static int OnGetName ( lua_State* state );
};

} // namespace pbr


#endif // PBR_ACTOR_H
