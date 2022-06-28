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

class Actor final
{
    private:
        using RegisterHander = void ( Actor::* ) ( ComponentRef &component,
            ComponentList &freeTransferResource,
            ComponentList &renderable,
            android_vulkan::Physics &physics,
            lua_State &vm
        ) noexcept;

    private:
        std::deque<ComponentRef>                                _components {};
        std::string const                                       _name;
        TransformableList                                       _transformableComponents {};

        static int                                              _appendComponentIndex;
        static int                                              _makeActorIndex;

        static std::unordered_map<ClassID, RegisterHander>      _registerHandlers;

    public:
        Actor () noexcept;

        Actor ( Actor const & ) = delete;
        Actor& operator = ( Actor const & ) = delete;

        Actor ( Actor && ) = delete;
        Actor& operator = ( Actor && ) = delete;

        explicit Actor ( std::string &&name ) noexcept;

        ~Actor () = default;

        void AppendComponent ( ComponentRef &component ) noexcept;
        [[nodiscard]] std::string const& GetName () const noexcept;

        // Note transfrom must be in render units.
        void OnTransform ( GXMat4 const &transformWorld ) noexcept;

        void RegisterComponents ( ComponentList &freeTransferResource,
            ComponentList &renderable,
            android_vulkan::Physics &physics,
            lua_State &vm
        ) noexcept;

        static bool Init ( lua_State &vm ) noexcept;
        static void Destroy () noexcept;

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

        [[nodiscard]] static int OnGetName ( lua_State* state );
};

} // namespace pbr


#endif // PBR_ACTOR_H
