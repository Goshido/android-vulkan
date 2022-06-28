#ifndef PBR_COMPONENT_H
#define PBR_COMPONENT_H


#include "component_desc.h"
#include "types.h"

GX_DISABLE_COMMON_WARNINGS

extern "C" {

#include <lua/lstate.h>

} // extern "C"

GX_RESTORE_WARNING_STATE


namespace pbr {

class Component
{
    protected:
        std::string     _name;

    private:
        ClassID         _classID;

    public:
        Component () = delete;

        Component ( Component const & ) = delete;
        Component& operator = ( Component const & ) = delete;

        Component ( Component && ) = delete;
        Component& operator = ( Component && ) = delete;

        virtual ~Component () = default;

        [[nodiscard]] ClassID GetClassID () const noexcept;
        [[nodiscard]] std::string const& GetName () const noexcept;

        [[nodiscard]] static ComponentRef Create ( android_vulkan::Renderer &renderer,
            size_t &commandBufferConsumed,
            size_t &dataRead,
            ComponentDesc const &desc,
            uint8_t const* data,
            VkCommandBuffer const* commandBuffers
        ) noexcept;

        static void Register ( lua_State &vm ) noexcept;

    protected:
        explicit Component ( ClassID classID ) noexcept;
        explicit Component ( ClassID classID, std::string &&name ) noexcept;

    private:
        [[nodiscard]] static int OnGetName ( lua_State* state );
};

} // namespace pbr


#endif // PBR_COMPONENT_H
