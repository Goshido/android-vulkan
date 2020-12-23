#ifndef ANDROID_VULKAN_KEY_ACTION_H
#define ANDROID_VULKAN_KEY_ACTION_H


#include "key_bind.h"


namespace android_vulkan {

class KeyAction final
{
    public:
        KeyBind         _bind {};
        KeyAction*      _next = nullptr;

    public:
        KeyAction () = default;

        KeyAction ( KeyAction const &other ) = delete;
        KeyAction& operator = ( KeyAction const &other ) = delete;

        KeyAction ( KeyAction &&other ) = delete;
        KeyAction& operator = ( KeyAction &&other ) = delete;

        ~KeyAction () = default;
};


} // namespace android_vulkan


#endif // ANDROID_VULKAN_KEY_ACTION_H
