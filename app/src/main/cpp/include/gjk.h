#ifndef ANDROID_VULKAN_GJK_HPP
#define ANDROID_VULKAN_GJK_HPP


#include "gjk_base.h"
#include "shape.h"


namespace android_vulkan {

// The implementation is based on ideas from
// https://www.youtube.com/watch?v=Qupqu1xe7Io
class GJK final : public GJKBase
{
    private:
        GXVec3      _direction {};

    public:
        GJK () = default;

        GJK ( GJK const & ) = delete;
        GJK &operator = ( GJK const & ) = delete;

        GJK ( GJK && ) = delete;
        GJK &operator = ( GJK && ) = delete;

        ~GJK () override = default;

        void Reset () noexcept;

        // The method returns true if two shapes have intersection. Otherwise the method returns false.
        [[nodiscard]] bool Run ( Shape const &shapeA, Shape const &shapeB ) noexcept;

    private:
        void TestLine () noexcept;

        // The method returns true if simplex contains the origin.
        // The method returns false if more iterations are needed.
        bool TestTetrahedron () noexcept;

        void TestTriangle () noexcept;

};

} // namespace android_vulkan


#endif // ANDROID_VULKAN_GJK_HPP
