#ifndef ANDROID_VULKAN_GJK_H
#define ANDROID_VULKAN_GJK_H


#include <shape.h>
#include <simplex.h>


namespace android_vulkan {

// The implementation is based on ideas from
// https://www.youtube.com/watch?v=Qupqu1xe7Io
class GJK final
{
    private:
        GXVec3      _direction;
        Simplex     _simplex;

        uint16_t    _steps;

        uint16_t    _testLine;
        uint16_t    _testTetrahedron;
        uint16_t    _testTriangle;

    public:
        GJK () noexcept;

        GJK ( GJK const & ) = delete;
        GJK& operator = ( GJK const & ) = delete;

        GJK ( GJK && ) = delete;
        GJK& operator = ( GJK && ) = delete;

        ~GJK () = default;

        [[nodiscard]] Simplex const& GetSimplex () const noexcept;
        [[nodiscard, maybe_unused]] uint16_t GetSteps () const noexcept;

        [[nodiscard, maybe_unused]] uint16_t GetTestLines () const noexcept;
        [[nodiscard, maybe_unused]] uint16_t GetTestTetrahedrons () const noexcept;
        [[nodiscard, maybe_unused]] uint16_t GetTestTriangles () const noexcept;

        void Reset () noexcept;

        // The method returns true if two shapes have intersection. Otherwise the method returns false.
        [[nodiscard]] bool Run ( Shape const &shapeA, Shape const &shapeB ) noexcept;

    private:
        void LineTest () noexcept;

        // The method returns true if shapes have intersection. The method returns false if needed more iterations.
        [[nodiscard]] bool TetrahedronTest () noexcept;

        void TriangleTest () noexcept;
};

} // namespace android_vulkan


#endif // ANDROID_VULKAN_GJK_H
