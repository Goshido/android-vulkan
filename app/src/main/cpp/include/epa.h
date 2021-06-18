#ifndef ANDROID_VULKAN_EPA_H
#define ANDROID_VULKAN_EPA_H


#include "simplex.h"
#include <vector>


namespace android_vulkan {

using Edge = std::pair<size_t, size_t>;
using Vertices = std::vector<GXVec3>;

struct Face final
{
    [[maybe_unused]] size_t     _a;
    [[maybe_unused]] size_t     _b;
    [[maybe_unused]] size_t     _c;
    [[maybe_unused]] GXVec3     _normal;

    Face () noexcept;

    Face ( Face const & ) = default;
    Face& operator = ( Face const & ) = default;

    Face ( Face && ) = default;
    Face& operator = ( Face && ) = default;

    [[maybe_unused]] explicit Face ( size_t a, size_t b, size_t c, Vertices const &vertices ) noexcept;

    ~Face () = default;

    [[maybe_unused]] void Flip () noexcept;
};

// The implementation is based on ideas from
// https://www.youtube.com/watch?v=6rgiPrzqt9w
// https://www.youtube.com/watch?v=0XQ2FSz3EK8
class EPA final
{
    private:
        float                   _depth;
        GXVec3                  _normal;

        std::vector<Face>       _faces;
        Vertices                _vertices;

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

    private:
        void CreatePolytope ( Simplex const &simplex ) noexcept;
};

} // namespace android_vulkan


#endif // ANDROID_VULKAN_EPA_H
