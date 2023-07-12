#ifndef ANDROID_VULKAN_SIMPLEX_H
#define ANDROID_VULKAN_SIMPLEX_H


#include <shape.h>


namespace android_vulkan {

class Simplex final
{
    public:
        uint8_t     _pointCount = 0U;
        GXVec3      _supportPoints[ 4U ] {};

    public:
        Simplex () = default;

        Simplex ( Simplex const & ) = delete;
        Simplex &operator = ( Simplex const & ) = delete;

        Simplex ( Simplex && ) = delete;
        Simplex &operator = ( Simplex && ) = delete;

        ~Simplex () = default;

        void PushPoint ( GXVec3 const &supportPoint ) noexcept;
};

} // namespace android_vulkan


#endif // ANDROID_VULKAN_SIMPLEX_H
