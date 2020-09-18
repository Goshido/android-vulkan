#ifndef ANDROID_VULKAN_TRIGGER_H
#define ANDROID_VULKAN_TRIGGER_H


#include <GXCommon/GXWarning.h>

GX_DISABLE_COMMON_WARNINGS

#include <android/input.h>

GX_RESTORE_WARNING_STATE


namespace android_vulkan {

typedef void ( *TriggerHandler ) ( void* context, float push );

class Trigger final
{
    private:
        void*                   _context;
        TriggerHandler          _handler;
        float                   _pushCurrent;
        float                   _pushOld;

    public:
        Trigger ();

        Trigger ( Trigger const &other ) = delete;
        Trigger& operator = ( Trigger const &other ) = delete;

        Trigger ( Trigger &&other ) = delete;
        Trigger& operator = ( Trigger &&other ) = delete;

        ~Trigger () = default;

        void Bind ( void* context, TriggerHandler handler );
        void Unbind ();

        void Execute ();
        void Update ( AInputEvent* event, int32_t axis );
};

} // namespace android_vulkan


#endif // ANDROID_VULKAN_TRIGGER_H
