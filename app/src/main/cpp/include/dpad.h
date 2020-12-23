#ifndef ANDROID_VULKAN_DPAD_H
#define ANDROID_VULKAN_DPAD_H


namespace android_vulkan {

class DPad final
{
    public:
        bool    _down = false;
        bool    _left = false;
        bool    _right = false;
        bool    _up = false;

    public:
        DPad () = default;

        DPad ( DPad const &other ) = delete;
        DPad& operator = ( DPad const &other ) = default;

        DPad ( DPad &&other ) = delete;
        DPad& operator = ( DPad &&other ) = delete;

        ~DPad () = default;
};

} // namespace android_vulkan


#endif // ANDROID_VULKAN_DPAD_H
