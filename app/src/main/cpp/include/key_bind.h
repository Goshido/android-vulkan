#ifndef ANDROID_VULKAN_KEY_BIND_H
#define ANDROID_VULKAN_KEY_BIND_H


namespace android_vulkan {

typedef void ( *KeyHandler ) ( void* context ) noexcept;

class KeyBind final
{
    public:
        void*           _context;
        KeyHandler      _handler;

    public:
        KeyBind () noexcept;

        KeyBind ( KeyBind const &other ) = default;
        KeyBind& operator = ( KeyBind const &other ) = default;

        KeyBind ( KeyBind &&other ) = default;
        KeyBind& operator = ( KeyBind &&other ) = default;

        ~KeyBind () = default;

        void Init ( void* context, KeyHandler handler ) noexcept;
        void Reset () noexcept;
};

} // namespace android_vulkan


#endif // ANDROID_VULKAN_KEY_BIND_H
