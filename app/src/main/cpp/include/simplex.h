#ifndef ANDROID_VULKAN_SIMPLEX_H
#define ANDROID_VULKAN_SIMPLEX_H


#include <shape.h>


namespace android_vulkan {

class Simplex final
{
    public:
        uint8_t     _pointCount;
        GXVec3      _supportPoints[ 4U ];

    public:
        Simplex () noexcept;

        Simplex ( Simplex const & ) = delete;
        Simplex& operator = ( Simplex const & ) = delete;

        Simplex ( Simplex && ) = delete;
        Simplex& operator = ( Simplex && ) = delete;

        ~Simplex () = default;

        void PushPoint ( GXVec3 const &supportPoint ) noexcept;

        [[nodiscard]] static GXVec3 FindSupportPoint ( GXVec3 const &direction,
            Shape const &shapeA,
            Shape const &shapeB
        ) noexcept;
};

} // namespace android_vulkan


#endif // ANDROID_VULKAN_SIMPLEX_H
