#ifndef ANDROID_VULKAN_EPA_H
#define ANDROID_VULKAN_EPA_H


#include "simplex.h"
#include <vector>


namespace android_vulkan {

using Vertices = std::vector<GXVec3>;

struct Face final
{
    size_t      _a;
    size_t      _b;
    size_t      _c;
    GXVec3      _normal;

    Face () noexcept;

    Face ( Face const & ) = default;
    Face& operator = ( Face const & ) = default;

    Face ( Face && ) = default;
    Face& operator = ( Face && ) = default;

    explicit Face ( size_t a, size_t b, size_t c, Vertices const &vertices ) noexcept;

    ~Face () = default;

    [[maybe_unused]] void Flip () noexcept;
};

// The implementation is based on ideas from
// https://www.youtube.com/watch?v=6rgiPrzqt9w
// https://www.youtube.com/watch?v=0XQ2FSz3EK8
class EPA final
{
    private:
        using Edge = std::pair<size_t, size_t>;
        using FindResult = std::pair<size_t, float>;

    private:
        float                   _depth;
        GXVec3                  _normal;

        std::vector<Edge>       _edges;
        std::vector<Face>       _faces;
        Vertices                _vertices;

        uint16_t                _steps;

    public:
        EPA () noexcept;

        EPA ( EPA const & ) = delete;
        EPA& operator = ( EPA const & ) = delete;

        EPA ( EPA && ) = delete;
        EPA& operator = ( EPA && ) = delete;

        ~EPA () = default;

        [[maybe_unused, nodiscard]] float GetDepth () const noexcept;
        [[maybe_unused, nodiscard]] GXVec3 const& GetNormal () const noexcept;

        [[maybe_unused, nodiscard]] uint16_t GetSteps () const noexcept;
        [[maybe_unused, nodiscard]] uint16_t GetEdgeCount () const noexcept;
        [[maybe_unused, nodiscard]] uint16_t GetFaceCount () const noexcept;
        [[maybe_unused, nodiscard]] uint16_t GetVertexCount () const noexcept;

        void Reset () noexcept;
        [[nodiscard]] bool Run ( Simplex const &simplex, Shape const &shapeA, Shape const &shapeB ) noexcept;

    private:
        void CreatePolytope ( Simplex const &simplex ) noexcept;
        [[nodiscard]] FindResult FindClosestFace () noexcept;
        void SolveEdge ( size_t a, size_t b ) noexcept;
};

} // namespace android_vulkan


#endif // ANDROID_VULKAN_EPA_H
