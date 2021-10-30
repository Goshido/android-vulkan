#ifndef ANDROID_VULKAN_GJK_BASE_H
#define ANDROID_VULKAN_GJK_BASE_H


#include "simplex.h"



namespace android_vulkan {

// The implementation is based on ideas from
// https://www.youtube.com/watch?v=Qupqu1xe7Io
class GJKBase
{
    protected:
        GXVec3      _direction {};
        Simplex     _simplex {};

        uint16_t    _steps = 0U;

        uint16_t    _testLine = 0U;
        uint16_t    _testTetrahedron = 0U;
        uint16_t    _testTriangle = 0U;

    public:
        GJKBase ( GJKBase const & ) = delete;
        GJKBase& operator = ( GJKBase const & ) = delete;

        GJKBase ( GJKBase && ) = delete;
        GJKBase& operator = ( GJKBase && ) = delete;

        [[nodiscard]] Simplex const& GetSimplex () const noexcept;
        [[nodiscard]] uint16_t GetSteps () const noexcept;

        [[nodiscard, maybe_unused]] uint16_t GetTestLines () const noexcept;
        [[nodiscard, maybe_unused]] uint16_t GetTestTetrahedrons () const noexcept;
        [[nodiscard, maybe_unused]] uint16_t GetTestTriangles () const noexcept;

    protected:
        GJKBase () = default;
        virtual ~GJKBase () = default;

        void LineTest () noexcept;

        // The method returns true if simplex contains the origin.
        // The method returns false if more iterations are needed.
        bool TetrahedronTest () noexcept;

        void TriangleTest () noexcept;
        void ResetInternal ( GXVec3 const &initialDirection ) noexcept;
};

} // namespace android_vulkan


#endif // ANDROID_VULKAN_GJK_BASE_H
