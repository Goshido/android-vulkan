#ifndef PBR_ACTOR_H
#define PBR_ACTOR_H


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
        std::deque<ComponentRef>    _components {};
        std::string const           _name;
        TransformableList           _transformableComponents {};

        static int                  _appendComponentIndex;
        static int                  _makeActorIndex;

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

        static bool Register ( lua_State &vm ) noexcept;

    private:
        [[nodiscard]] static int OnGetName ( lua_State* state );
};

} // namespace pbr


#endif // PBR_ACTOR_H
