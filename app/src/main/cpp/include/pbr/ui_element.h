#ifndef PBR_UI_ELEMENT_H
#define PBR_UI_ELEMENT_H


#include <GXCommon/GXWarning.h>

GX_DISABLE_COMMON_WARNINGS

#include <memory>
#include <string>

extern "C" {

#include <lua/lstate.h>

} // extern "C"

// This macro was defined by Lua headers and breaks 'std'.
#undef ispow2

GX_RESTORE_WARNING_STATE


namespace pbr {

class UIElement
{
    protected:
        bool    _visible = false;

    public:
        UIElement () = delete;

        UIElement ( UIElement const & ) = delete;
        UIElement& operator = ( UIElement const & ) = delete;

        UIElement ( UIElement && ) = delete;
        UIElement& operator = ( UIElement && ) = delete;

        virtual ~UIElement () = default;

        virtual void ApplyLayout () noexcept = 0;
        virtual void Render () noexcept = 0;

    protected:
        explicit UIElement ( bool visible ) noexcept;

    private:
        [[nodiscard]] static int OnShow ( lua_State* state );
        [[nodiscard]] static int OnHide ( lua_State* state );
};

} // namespace pbr

#endif // PBR_UI_ELEMENT_H
