#ifndef ANDROID_VULKAN_STICK_H
#define ANDROID_VULKAN_STICK_H

#include <GXCommon/GXWarning.h>

GX_DISABLE_COMMON_WARNINGS

#include <android/input.h>

GX_RESTORE_WARNING_STATE

#include <GXCommon/GXMath.h>


namespace android_vulkan {

typedef void ( *StickHandler ) ( void* context, float horizontal, float vertical );

class Stick final
{
    private:
        void*           _context;
        StickHandler    _handler;
        GXVec2          _stateCurrent;
        GXVec2          _stateOld;

    public:
        Stick ();

        Stick ( Stick const &other ) = delete;
        Stick& operator = ( Stick const &other ) = delete;

        Stick ( Stick &&other ) = delete;
        Stick& operator = ( Stick &&other ) = delete;

        ~Stick () = default;

        void Bind ( void* context, StickHandler handler );
        void Unbind ();

        void Execute ();
        void Update ( AInputEvent* event, int32_t horizontalAxis, int32_t verticalAxis );
    };

} // namespace android_vulkan


#endif // ANDROID_VULKAN_STICK_H
