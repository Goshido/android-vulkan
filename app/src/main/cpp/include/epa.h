#ifndef ANDROID_VULKAN_EPA_H
#define ANDROID_VULKAN_EPA_H


#include "simplex.h"


namespace android_vulkan {

class EPA final
{
    private:
        float       _depth;
        GXVec3      _normal;

    public:
        EPA () noexcept;

        EPA ( EPA const & ) = delete;
        EPA& operator = ( EPA const & ) = delete;

        EPA ( EPA && ) = delete;
        EPA& operator = ( EPA && ) = delete;

        ~EPA () = default;

        [[maybe_unused, nodiscard]] float GetDepth () const noexcept;
        [[maybe_unused, nodiscard]] GXVec3 const& GetNormal () const noexcept;

        void Reset () noexcept;
        [[nodiscard]] bool Run ( Simplex const &simplex, Shape const &shapeA, Shape const &shapeB ) noexcept;
};

} // namespace android_vulkan


#endif // ANDROID_VULKAN_EPA_H
